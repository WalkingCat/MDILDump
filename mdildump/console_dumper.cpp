#include "stdafx.h"
#include "console_dumper.h"

using namespace std;

struct ulong_as_chars {
	char chars[5];
	ulong_as_chars(unsigned long ulong) {
		chars[0] = *((char*)&ulong + 3);
		chars[1] = *((char*)&ulong + 2);
		chars[2] = *((char*)&ulong + 1);
		chars[3] = *((char*)&ulong + 0);
		chars[4] = 0;
	}
};

void dump_flags( uint32_t flags )
{
	bool first = true;
	for (int i = 0; i < 32; i++) {
		if (flags & (1 << i)) {
			if (first) first = false; else { printf_s("|"); }
			printf_s("0x%x", 1 << i);
		}
	}
}

void console_dumper::dump_mdil_header( const char* title, const char* description )
{
	if (!m_data.header) {
		printf_s("%s: Not found\n", title);
		return;
	}

	const mdil_header& header = *m_data.header;

	if (title != nullptr) printf_s("%s: Size = %d bytes\n", title, sizeof(mdil_header));
	if (description != nullptr) printf_s("%s\n", description);

	printf_s("\t.hdrSize = %d\n", header.hdrSize);
	printf_s("\t.magic = %s\n", ulong_as_chars(header.magic).chars);
	printf_s("\t.version = 0x%08x\n", header.version);
	printf_s("\t.typeMapCount = %d\n", header.typeMapCount);
	printf_s("\t.methodMapCount = %d\n", header.methodMapCount);
	printf_s("\t.GenericInstancesSize = %d\n", header.genericInstSize);
	printf_s("\t.extModRefsCount = %d\n", header.extModRefsCount);
	printf_s("\t.extTypeRefsCount = %d\n", header.extTypeRefsCount);
	printf_s("\t.extMemberRefsCount = %d\n", header.extMemberRefsCount);
	printf_s("\t.typeSpecCount = %d\n", header.typeSpecCount);
	printf_s("\t.methodSpecCount = %d\n", header.methodSpecCount);
	printf_s("\t.section10Size = %d\n", header.section10Size);
	printf_s("\t.namePoolSize = %d\n", header.namePoolSize);
	printf_s("\t.typeSize = %d\n", header.typeSize);
	printf_s("\t.userStringPoolSize = %d\n", header.userStringPoolSize);
	printf_s("\t.codeSize = %d\n", header.codeSize);
	printf_s("\t.section16Size = %d\n", header.section16Size);
	printf_s("\t.unknown17 = %d\n", header.section17Size);
	printf_s("\t.debugMapCount = %d\n", header.debugMapCount);
	printf_s("\t.debugInfoSize = %d\n", header.debugInfoSize);
	printf_s("\t.timeDateStamp = 0x%x\n", header.timeDateStamp);
	printf_s("\t.subsystem = %d\n", header.subsystem);
	printf_s("\t.baseAddress = 0x%x\n", header.baseAddress);
	printf_s("\t.entryPointToken = 0x%x\n", header.entryPointToken);

	unsigned int flags = header.flags;
	printf_s("\t.flags = 0x%x ", flags);
	if (flags != 0) { printf_s("("); dump_flags(flags); printf_s(")"); }
	printf_s("\n");

	printf_s("\t.unknown = 0x%x\n", header.Unknown);
	printf_s("\t.platformID = %d (%s)\n", header.platformID, (header.platformID == 1) ? "Triton" : "Unknown");
	printf_s("\t.platformDataSize = %d\n", header.platformDataSize);
	printf_s("\t.code1Size = %d\n", header.code1Size);
	printf_s("\t.debugInfo1Size = %d\n", header.debugInfo1Size);
	printf_s("\t.is_4 = %d\n", header.is_4);
	printf_s("\t.is_C68D0000 = %08X\n", header.is_C68D0000);
	printf_s("\t.is_0 = %d\n", header.is_0);
	printf_s("\t.is_0_too = %d\n", header.is_0_too);
	printf_s("\n");
}

void console_dumper::dump_mdil_header_2( const char* title, const char* description )
{
	if (!m_data.header_2) {
		printf_s("%s: Not found\n", title);
		return;
	}

	const mdil_header_2& header2 = *m_data.header_2;

	if (title != nullptr) printf_s("%s: Size = %d bytes\n", title, sizeof(mdil_header_2));
	if (description != nullptr) printf_s("%s\n", description);

	printf_s("\t.Size = %d\n", header2.size);
	printf_s("\t.field_04 = %08x\n", header2.field_04);
	printf_s("\t.field_08 = %08x\n", header2.field_08);
	printf_s("\t.field_0C = %08x\n", header2.field_0c);
	printf_s("\t.field_10 = %08x\n", header2.field_10);
	printf_s("\t.field_14 = %08x\n", header2.field_14);
	printf_s("\t.field_18 = %08x\n", header2.field_18);
	printf_s("\t.field_1C = %08x\n", header2.field_1c);
	printf_s("\t.field_20 = %08x\n", header2.field_20);
	printf_s("\t.field_24 = %08x\n", header2.field_24);
	printf_s("\t.field_28 = %08x\n", header2.field_28);
	printf_s("\t.field_2C = %08x\n", header2.field_2c);
	printf_s("\t.field_30 = %08x\n", header2.field_30);
	printf_s("\t.field_34 = %08x\n", header2.field_34);
	printf_s("\t.field_38 = %08x\n", header2.field_38);
	printf_s("\t.field_3C = %08x\n", header2.field_3c);
	printf_s("\t.field_40 = %08x\n", header2.field_40);
	printf_s("\t.field_44 = %08x\n", header2.field_44);
	printf_s("\t.field_48 = %08x\n", header2.field_48);
	printf_s("\t.field_4C = %08x\n", header2.field_4c);
	printf_s("\t.field_50 = %08x\n", header2.field_50);
	printf_s("\t.field_54 = %08x\n", header2.field_54);
	printf_s("\t.field_58 = %08x\n", header2.field_58);
	printf_s("\t.field_5C = %08x\n", header2.field_5c);
	printf_s("\t.field_60 = %08x\n", header2.field_60);
	printf_s("\t.section21_count = %08x\n", header2.section_21_count);
	printf_s("\t.section22_count = %08x\n", header2.section_22_count);
	printf_s("\t.field_6C = %08x\n", header2.field_6c);
	printf_s("\t.field_70 = %08x\n", header2.field_70);
	printf_s("\t.field_74 = %08x\n", header2.field_74);
	printf_s("\n");
}

template<typename T>
void print_vector_size(const shared_vector<T>& data, const char* title, const char* description)
{
	if (!data) {
		printf_s("%s: Not found\n", title);
		return;
	}

	if (title != nullptr) {
		if (sizeof(T) == 1) printf_s("%s: Size = %d bytes\n", title, data.size());
		else printf_s("%s: Count = %d, Size = %d bytes\n", title, data.size(), data.size() * sizeof(T));
	}
	if (description != nullptr) printf_s("(%s)\n", description);
}

void console_dumper::dump_bytes( const shared_vector<unsigned char>& data, const char* title, const char* description )
{
	print_vector_size(data, title, description);

	dump_bytes_int(data, 0, data.size());

	printf_s("\n");
}

void console_dumper::dump_bytes_int( const shared_vector<unsigned char>& data, size_t offset, size_t count )
{
	unsigned long i;
	for(i = 0; i < count; i++) {
		if ((i % 16) == 0) printf_s("%.06X:", i);
		printf_s(" %02X", data->at(i + offset));
		if ((i % 16) == 15) printf_s("\n");
	}
	if ((i % 16) != 0) printf_s("\n");
}

void console_dumper::dump_chars(const shared_vector<char>& data, const char* title, const char* description)
{
	print_vector_size(data, title, description);

	unsigned long i;
	for(i = 0; i < data.size(); i++) {
		if ((i % 64) == 0) printf_s("%.06X: ", i);
		printf_s("%c", data->at(i));
		if ((i % 64) == 63) printf_s("\n");
	}
	if ((i % 64) != 0) printf_s("\n");
	printf_s("\n");
}

void console_dumper::dump_ulongs(const shared_vector<unsigned long>& data, const char* title, const char* description)
{
	print_vector_size(data, title, description);

	unsigned long i;
	for(i = 0; i < data.size(); i++) {
		if ((i % 8) == 0) printf_s("%06X:", i);
		printf_s(" %08x", data->at(i));
		if ((i % 8) == 7) printf_s("\n");
	}
	if ((i % 8) != 0) printf_s("\n");
	printf_s("\n");
}

void console_dumper::dump_method_map(const char* title, const char* description)
{
	print_vector_size(m_data.method_map, title, description);

	if (m_data.method_map.size() > 0) {
		unsigned long i = 0;
		for (i = 0; i < m_data.method_map.size(); i++) {
			if (((i % 4) == 0)) printf_s("%06X: ", i);

			unsigned long method = m_data.method_map->at(i);
			if (method == 0xCAFEDEAD) { // WTF ?
				printf_s("????: %08X", method);
			} else if (method & (1 << 31)) {
				printf_s("GENI: %08X", method  & ~(1 << 31));
			} else {
				printf_s("CODE: %08X", method);
			}

			printf_s( ((i % 4) == 3) ? "\n" : " ");
		}
		if ((i % 4) != 0) printf_s("\n");
	}
	printf_s("\n");
}

void console_dumper::dump_generic_instances( const char* title, const char* description )
{
	print_vector_size(m_data.generic_instances, title, description);

	if (m_data.generic_instances.size() > 4) {
		auto sig = *(DWORD*) m_data.generic_instances->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		if (sig == 'MDGI') {
			//TODO: move this part into parser !
			size_t pos = 4;
			while (pos < m_data.generic_instances.size()) {
				auto genInst = (GenericInst*) &m_data.generic_instances->at(pos);
				printf_s("%08X: InstCount = %d, Arity = %d ", pos, genInst->InstCount, genInst->Arity);
				if ((genInst->InstCount == 0) || (genInst->Arity == 0)) break;
				pos += sizeof(GenericInst);
				pos += (genInst->InstCount * genInst->Arity) * 4; // here is a matrix, skip it
				printf_s("\t");
				for (WORD i = 0; i < genInst->InstCount; i++) {
					printf_s("[CODE: %08X DBUG: %08X] ", *(DWORD*) &m_data.generic_instances->at(pos), *(DWORD*) &m_data.generic_instances->at(pos + 4));
					pos += 4 * 2;
				}
				printf_s("\n");
			}

		} else { printf("Signature Incorrect, should be 'MDGI'"); }
	}
	printf_s("\n");
}

std::string console_dumper::format_ext_module_ref( unsigned long id )
{
	auto saved = ext_modules.find(id);
	if (saved != ext_modules.end()) return saved->second;
	else {
		string module(m_data.name_pool->data() + m_data.ext_module_refs->at(id).ModName);
		auto off = module.find(',');
		if (off > 0) {
			auto off2 = module.find(',', off + 1);
			if (off2 > 0) module.swap(module.substr(0, off2));
			else module.swap(module.substr(0, off));
		}
		ext_modules[id] = module;
		return ext_modules[id];
	}
}

void console_dumper::dump_ext_module_refs( const char* title, const char* description )
{
	print_vector_size(m_data.ext_module_refs, title, description);

	if ((!m_data.ext_module_refs) || m_data.ext_module_refs->empty()) return;

	printf_s("Signature?: 0x%X\n", m_data.ext_module_refs->at(0).ModName);
	for (unsigned long i = 1; i < m_data.ext_module_refs.size(); i++) {
		printf_s("%04d = %s [%s]\n", i, format_ext_module_ref(i).c_str(), &m_data.name_pool->at(m_data.ext_module_refs->at(i).RefName));
	}
	printf_s("\n");
}

void console_dumper::dump_ext_type_refs( const char* title, const char* description )
{
	print_vector_size(m_data.ext_type_refs, title, description);

	for (unsigned long i = 1; i < m_data.ext_type_refs.size(); i++) {
		auto ref = m_data.ext_type_refs->at(i);
 		printf_s("%04d: [MODR(%04d), %04d]", i, ref.module, ref.ordinal);
		printf_s(" ; MODR = %s\n", format_ext_module_ref(ref.module).c_str());
	}
	printf_s("\n");
}

std::string console_dumper::format_ext_type_ref( unsigned long id )
{
	stringstream ss;
	auto ref = m_data.ext_type_refs->at(id);

	ss << format_ext_module_ref(ref.module) << "(" << setw(4) << setfill('0') << ref.ordinal << ")";

	return ss.str();
}

void console_dumper::dump_ext_member_refs( const char* title, const char* description )
{
	print_vector_size(m_data.ext_member_refs, title, description);

	for (unsigned long i = 1; i < m_data.ext_member_refs.size(); i++) {
		auto ref = m_data.ext_member_refs->at(i);
		printf_s("%04d: %s ", i, ref.isField ? "FIELD " : "MEMBER");
		if (ref.isTypeSpec) {
			printf_s("[TYPS(%04d), %04d]\n", ref.extTypeRid, ref.ordinal);
		} else {
			printf_s("[TYPR(%04d), %04d]", ref.extTypeRid, ref.ordinal);
			printf_s(" ; TYPR = %s\n", format_ext_type_ref(ref.extTypeRid).c_str());
		}
	}
	printf_s("\n");
}

void console_dumper::dump_types(const char* title, const char* description)
{
	print_vector_size(m_data.types, title, description);

	size_t pos = 0;
	if (m_data.types.size() >= 4) {
		unsigned long sig = * (unsigned long*) m_data.types->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		pos += 4;
	}

	map<unsigned long, unsigned long> offsets;

	for (auto i = begin(*m_data.type_map); i != end(*m_data.type_map); ++i) {
		offsets[*i] = 1;
	}

	for (auto i = begin(*m_data.type_specs); i != end(*m_data.type_specs); ++i) {
		offsets[*i] = 2;
	}

	for (auto i = begin(*m_data.method_specs); i != end(*m_data.method_specs); ++i) {
		offsets[*i] = 3;
	}

	pair<unsigned long, unsigned long> prev(0xffffffff, 0);
	for (auto i = begin(offsets); i != end(offsets); ++i) {
		if (prev.first < m_data.types.size()) {
			printf_s("%s_%06X\n", (prev.second == 1) ? "TYPM" : (prev.second == 2) ? "TYPS" : "METS", prev.first);
			dump_bytes_int(m_data.types, prev.first, i->first - prev.first);
		}
		prev = *i;
	}
	printf_s("%s_%06X\n", (prev.second == 1) ? "TYPM" : (prev.second == 2) ? "TYPS" : "METS", prev.first);
	dump_bytes_int(m_data.types, prev.first, m_data.types.size() - prev.first);
	
// 	if (pos < m_data.types.size()) {
// 		dump_bytes_int(m_data.types, pos, m_data.types.size() - pos);
// 	}
	
	printf_s("\n");
}

void console_dumper::dump_code( const mdil_code& code, const char* title, const char* description )
{
	print_vector_size(code.data, title, description);

	size_t pos = 0;
	if (code.data.size() >= 4) {
		unsigned long sig = * (unsigned long*) code.data->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		pos += 4;
	}
	
	printf_s("\n");

	for (auto m = begin(code.methods); m != end(code.methods); ++m) {
		printf_s("METHOD_%06X:\nSize = %4d (0x%04X) bytes, Routine = %4d (0x%04X) bytes, Exceptions = %d\n",
			m->global_offset, m->size, m->size, m->routine_size, m->routine_size, m->exception_count);
		dump_bytes_int(code.data, m->offset, m->size);
		if (!m->routine.empty()) dump_instructions(m->routine, code.data->data() + m->offset + m->routine_offset, m->routine_size);
		printf_s("\n");
	}
}

void console_dumper::dump_instructions( const std::vector<std::shared_ptr<mdil_instruction>>& code, unsigned char* data, unsigned long count )
{
	for(auto i : code) {
		printf_s("MDIL_%04X:", i->offset);
		for (unsigned long pos = i->offset; pos < (i->offset + i->length); ++pos) {
			printf_s(" %02X", data[pos]);
		}

		if (i->length < 2) printf_s("\t"); // too short, add some spaces
		if (i->length > 4) printf_s("\n\t\t"); // too long, wrap to next line

		if (!i->opcode.empty()) {
			printf_s("\t%s", i->opcode.c_str());
			if (i->opcode.length() < 4) printf_s("\t");
			if (!i->operands.empty()) {
				printf_s("\t%s", i->operands.c_str());
			}
			if (!i->annotation.empty()) {
				if (i->operands.empty()) printf_s("\t\t");
				printf_s("\t; %s", i->annotation.c_str());
			}
			printf_s("\n");
		}
	}
}

void console_dumper::dump_debug_info( const shared_vector<unsigned char>& data, bool hasSig, const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	print_vector_size(data, title, description);

	size_t pos = 0;

	if (hasSig && (data.size() >= 4)) {
		unsigned long sig = * (unsigned long*) data->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		pos += 4;
	}

	if (pos < data.size()) dump_bytes_int(data, pos, data.size() - pos);

	printf_s("\n");
}
