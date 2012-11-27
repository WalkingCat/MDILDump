#include "stdafx.h"
#include "mdil_parser.h"
#include "mdil_decoder.h"

DWORD parse_compressed_dword(const unsigned char* buffer, unsigned long &pos) {
	DWORD ret = 0;

	BYTE b;
	do {
		b = *(buffer + pos++);
		ret = (ret << 8) + (b & 0x7f);
	} while (b & 0x80);

	return ret;
}

DWORD parse_method(const unsigned char* buffer, DWORD* routineOffset, DWORD* routineSize, DWORD* exceptCount) {
	DWORD pos = 0;

	DWORD routine_size = 0, except = 0;

	routine_size = buffer[0];
	pos += 1;

	if (routine_size > 0xdf) {
		except = (routine_size >> 3) & 3;
		routine_size = routine_size & 7;

		if (routine_size <= 5) {
			routine_size = buffer[1] + routine_size * 0x100;
			pos += 1;
		} else if (routine_size == 6) {
			routine_size = *(WORD*) (buffer + 1);
			pos += 2;
		} else if (routine_size == 7) {
			routine_size = *(DWORD*) (buffer + 1);
			pos += 4;
		}

		if (except == 3) {
			except = buffer[pos];
			pos += 1;

			if (except == 0xff) {
				except = * (DWORD*) (buffer + pos);
				pos += 4;
			}
		}
	}

	if (routineOffset != nullptr) *routineOffset = pos;

	pos += routine_size;

	for (DWORD i = 0; i < except; ++i) {
		DWORD flags = parse_compressed_dword(buffer, pos);
		DWORD tryOffset = parse_compressed_dword(buffer, pos);
		DWORD tryLength = parse_compressed_dword(buffer, pos);
		DWORD handlerOffset = parse_compressed_dword(buffer, pos);
		DWORD handlerLength = parse_compressed_dword(buffer, pos);
		DWORD filterOffset = parse_compressed_dword(buffer, pos); // type token or filter offset
// 			printf_s("[FL:%X TO:%X TL:%X HO:%X HL:%X FO:%X] ", flags, tryOffset, tryLength, handlerOffset, handlerLength, filterOffset);
	}

	if (routineSize) *routineSize = routine_size;
	if (exceptCount) *exceptCount = except;

	return pos;
}

class easy_file {
	FILE* file;
public:
	easy_file(const char* path, const char* mode) : file(nullptr) { fopen_s(&file, path, mode); }
	~easy_file() { if (file != nullptr) fclose(file); }

	bool valid() { return file != nullptr; }
	long offset() { return ftell(file); }
	bool reset(long offset) { return fseek(file, offset, SEEK_SET) == 0; }
	unsigned char read1() { unsigned char ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	unsigned short read2() { unsigned short ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	unsigned int read4() { unsigned int ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	void skip(long bytes) { fseek(file, bytes, SEEK_CUR); }
	bool read(void* buffer, size_t size, size_t bytes) { return fread_s(buffer, size, 1, bytes, file) == bytes; }
	bool read(void* buffer, size_t size) { return read(buffer, size, size); }

	template<typename T>
	inline bool read(shared_vector<T>& data, size_t count) {
		data.resize(count);
		return read(data->data(), data.size() * sizeof(T));
	}
};

const char* g_file_reading_error = "file reading error";

std::string mdil_parser::parse(const char* filename, mdil_data& data) {
	easy_file file(filename, "rb");
	if (file.valid()) {
		file.reset(0x3c);
		file.reset(file.read4());

		if (file.read4() != 'EP') {
			return "PE signature incorrect";
		}

		file.skip(2);
		WORD numofsects = file.read2();
		file.skip(12);
		WORD sizeofopthdr = file.read2();
		file.skip(2);

		const WORD pe32 = 0x10b;
		const WORD pe32plus = 0x20b;

		WORD peversion = file.read2();
		if ((peversion != pe32) && (peversion != pe32plus)) {
			return "PE version unknown";
		}

		if (peversion == pe32) {
			file.skip(26);
		} else if (peversion == pe32plus) {
			file.skip(22);
		}

		file.skip(42);
		file.skip(sizeofopthdr - 70);

		unsigned int mdilOffset = 0, mdilSize = 0;

		for (int sect = 0; sect < numofsects; sect++) {
			char name[9] = {};

			file.read(name, sizeof(name), 8);

			if (strcmp(name, ".mdil") == 0) {
// 				printf_s("MDIL Section:\n");
// 				printf_s("\tVirtualSize = %d, VirtualAddress = 0x%x\n", file.read4(), file.read4());
				file.read4();file.read4();
				mdilSize = file.read4();
				mdilOffset = file.read4();
// 				printf_s("\tRawDataSize = %d, RawDataAddress = 0x%x\n", mdilSize, mdilOffset);
				file.skip(16);
			} else {
				file.skip(32);
			}
		}

		if ((mdilOffset > 0) && (mdilSize > 0)) {
			file.reset(mdilOffset);

			//TODO, check header size first, and alloc memory accordingly
			data.header = std::make_shared<mdil_header>();
			if (!file.read(data.header.get(), sizeof(mdil_header))) return g_file_reading_error;
			if (data.header->hdrSize != sizeof(mdil_header)) return "MDIL Header size mismatch";
			if (data.header->magic != 'MDIL') return "MDIL Header magic mismatch";

			data.header_2 = std::make_shared<mdil_header_2>();
			if (!file.read(data.header_2.get(), sizeof(mdil_header_2))) return g_file_reading_error;
			if (data.header_2->size != sizeof(mdil_header_2)) return "MDIL Header 2 size mismatch";

			if (!file.read(data.platform_data, data.header->platformDataSize)) return g_file_reading_error;

			if (data.header->flags & mdil_header::WellKnownTypesPresent) {
				if(!file.read(data.well_known_types, 30)) return g_file_reading_error; // magic number ?
			}
			
			if (!file.read(data.type_map, data.header->typeMapCount)) return g_file_reading_error;
			if (!file.read(data.method_map, data.header->methodMapCount)) return g_file_reading_error;
			if (!file.read(data.generic_instances, data.header->genericInstSize)) return g_file_reading_error;
			if (!file.read(data.ext_module_refs, data.header->extModRefsCount)) return g_file_reading_error;
			if (!file.read(data.ext_type_refs, data.header->extTypeRefsCount)) return g_file_reading_error;
			if (!file.read(data.ext_member_refs, data.header->extMemberRefsCount)) return g_file_reading_error;
			if (!file.read(data.type_specs.raw, data.header->typeSpecCount)) return g_file_reading_error;
			if (!file.read(data.method_specs, data.header->methodSpecCount)) return g_file_reading_error;
			if (!file.read(data.section_10, data.header->section10Size)) return g_file_reading_error;
			if (!file.read(data.name_pool, data.header->namePoolSize)) return g_file_reading_error;
			if (!file.read(data.types, data.header->typeSize)) return g_file_reading_error;
			if (!file.read(data.user_string_pool, data.header->userStringPoolSize)) return g_file_reading_error;

			if (!file.read(data.code_1.raw, data.header->code1Size)) return g_file_reading_error;

			DWORD code1_pos = 0;
			if (data.code_1.raw.size() >= 4) code1_pos += 4;
			while (code1_pos < data.code_1.raw.size()) {
				DWORD routine_offset, routine_size, except;
				DWORD length = parse_method(data.code_1.raw->data() + code1_pos, &routine_offset, &routine_size, &except);
				if (length > 0) data.code_1.methods.push_back(mdil_method(code1_pos, code1_pos, length, routine_offset, routine_size, except));
				code1_pos += length;
			}

			if (!file.read(data.code_2.raw, data.header->codeSize - data.header->code1Size)) return g_file_reading_error;

			DWORD code2_pos = 0;
			if (data.code_2.raw.size() >= 4) code2_pos += 4;
			while (code2_pos < data.code_2.raw.size()) {
				DWORD routine_offset, routine_size, except;
				DWORD length = parse_method(data.code_2.raw->data() + code2_pos, &routine_offset, &routine_size, &except);
				if (length > 0) data.code_2.methods.push_back(mdil_method(code2_pos, data.code_1.raw.size() + code2_pos, length, routine_offset, routine_size, except));
				code2_pos += length;
			}

			if (!file.read(data.section_16, data.header->section16Size)) return g_file_reading_error;
			if (!file.read(data.section_17, data.header->section17Size)) return g_file_reading_error;
			if (!file.read(data.debug_map, data.header->debugMapCount)) return g_file_reading_error;
			if (!file.read(data.debug_info_1, data.header->debugInfo1Size)) return g_file_reading_error;
			if (!file.read(data.debug_info_2, data.header->debugInfoSize - data.header->debugInfo1Size)) return g_file_reading_error;
			if (!file.read(data.section_21, data.header_2->section_21_count * 12)) return g_file_reading_error;
			if (!file.read(data.section_22, data.header_2->section_22_count * 8)) return g_file_reading_error;

// 			auto paddings = mdilOffset + mdilSize - file.offset();
		} else {
			return ".mdil section not found";
		}
	} else {
		return "open file failed";
	}

	return std::string();
}