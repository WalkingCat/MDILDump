#include "stdafx.h"
#include "mdil_ctl_parser.h"

enum CompactLayoutToken
{
	INVALID,
	START_TYPE,
	SMALL_START_TYPE,
	SIMPLE_START_TYPE,
	MODEST_START_TYPE,
	END_TYPE,
	IMPLEMENT_INTERFACE,
	ADVANCE_ENCLOSING_TYPEDEF,
	ADVANCE_METHODDEF,
	ADVANCE_METHODDEF_SHORT_MINUS_8,
	ADVANCE_METHODDEF_SHORT_0 = 0x11,
	ADVANCE_METHODDEF_SHORT_PLUS_8 = 0x19,
	ADVANCE_FIELDDEF,
	ADVANCE_FIELDDEF_SHORT_MINUS_8,
	ADVANCE_FIELDDEF_SHORT_0 = 0x23,
	ADVANCE_FIELDDEF_SHORT_PLUS_8 = 0x2B,
	FIELD_OFFSET,
	IMPLEMENT_INTERFACE_METHOD,
	METHOD,
	NORMAL_METHOD,
	SIMPLE_METHOD,
	PINVOKE_METHOD = 0x50,
	METHOD_IMPL,
	FIELD_INSTANCE,
	FIELD_STATIC,
	FIELD_THREADLOCAL,
	FIELD_CONTEXTLOCAL,
	FIELD_RVA,
	FIELD_SIMPLE,
	FIELD_MAX = 0x67,
	NATIVECALLABLE_METHOD = 0x67,
	RUNTIME_IMPORT_METHOD,
	RUNTIME_EXPORT_METHOD
};

uint32_t mdil_ctl_parser::read_uint32_le()
{
	unsigned long ret = m_buffer[m_pos] + (m_buffer[m_pos+1] << 8) + (m_buffer[m_pos+2] << 16) + (m_buffer[m_pos+3] << 24);
	m_pos += 4;
	return ret;
}

int32_t mdil_ctl_parser::read_compressed_int32()
{
	return (int32_t) read_compressed_uint32();
}

uint32_t mdil_ctl_parser::read_compressed_uint32()
{
	uint32_t ret = 0;

	uint8_t b = m_buffer[m_pos++];

	if ((b & 1) == 0) {
		ret = b >> 1;
	} else if ((b & 2) == 0) {
		ret = (b >> 2) + (m_buffer[m_pos++] << 6);
	} else if ((b & 4) == 0) {
		ret = (b >> 3) + (m_buffer[m_pos] << 5) + (m_buffer[m_pos+1] << 13);
		m_pos += 2;
	} else if ((b & 8) == 0) {
		ret = (b >> 4) + (m_buffer[m_pos] << 4) + (m_buffer[m_pos+1] << 12) + (m_buffer[m_pos+2] << 20);
		m_pos += 3;
	} else { // 0x0F
		ret = read_uint32_le();
	}

	return ret;
}

uint32_t mdil_ctl_parser::read_compressed_token()
{
	uint32_t ret = read_compressed_uint32();
	uint8_t token_type_flag = ret & 3;
	CorTokenType token_type = (token_type_flag == 0) ? mdtModule : (token_type_flag == 1) ? mdtTypeDef : (token_type_flag == 2) ? mdtTypeRef : mdtTypeSpec;
	return (ret >> 2) | token_type;
}

void mdil_ctl_parser::dump_type_map( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
//	print_vector_size(m_data.type_map, title, description);

	for (unsigned long i = 1; i < m_data.type_map.size(); i++) {
		auto type_offset = m_data.type_map->at(i);
		if (type_offset == 0) continue;
		printf_s("TYPM(%04X)=TYPE(%04X)\n", i, type_offset);
		m_pos = type_offset;
		bool res = dump_ctl_type();
		if (!res) printf_s("*ERROR*\n");
		printf_s("\n");
	}

	printf_s("\n");
}

void mdil_ctl_parser::dump_type_specs( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	for (unsigned long i = 1; i < m_data.type_specs.size(); i++) {
		auto type_spec = m_data.type_specs->at(i);
		unsigned long pos = 0;
		printf_s("TYPS(%04X)=TYPE(%04X) ", i, type_spec);
		m_pos = type_spec;
		bool res = dump_cor_type();
		if (!res) printf_s(" ERR");
		printf_s(" (size=%04X, next=%04X)\n", m_pos - type_spec, m_pos);
	}

	printf_s("\n");
}

void ctl_dump_flags( uint32_t flags )
{
	bool first = true;
	for (int i = 0; i < 32; i++) {
		if (flags & (1 << i)) {
			if (first) first = false; else { printf_s("|"); }
			printf_s("0x%x", 1 << i);
		}
	}
}

bool mdil_ctl_parser::dump_ctl_type_members( uint32_t fieldCount, uint32_t methodCount, uint32_t interfaceCount )
{
	static const uint32_t field_encodings[] = { 0x0112, 0x1112, 0x0608, 0x0108, 0x0102, 0x0312, 0x0612, 0x1108,
												0x0308, 0x1612, 0x0111, 0x1312, 0x0618, 0x0309, 0x0609, 0x0311, };
	static const char* field_storage[] = { "Instance", "Static", "ThreadLocal", "ContextLocal", "RVA" };
	static const char* field_protection[] = { "Private Scope", "Private", "Family and Assembly", "Assembly", "Family", "Family or Assembly", "Public" };

	bool fine = true;

	if ((m_buffer[m_pos] == 0x73) && (m_buffer[m_pos+1] == 0x0A)) {
		printf_s("73 0A ; what's this ?\n");
		m_pos += 2;
	}

	for (uint32_t i = 0; i < fieldCount; i++) {
		printf_s("\tField %d\n", i);
		bool go_on = false;
		do {
			go_on = false;
			uint8_t byte = m_buffer[m_pos++];
			if (byte == ADVANCE_FIELDDEF) {
				printf_s("\t\tAdvance Diff = %04X\n", read_compressed_int32());
				go_on = true;
			} else if (byte == FIELD_OFFSET) {
				printf_s("\t\tExplicit Offset = %04X\n", read_compressed_uint32());
			} else if ((byte >= ADVANCE_FIELDDEF_SHORT_MINUS_8) && (byte <= ADVANCE_FIELDDEF_SHORT_PLUS_8)) {
				printf_s("\t\tAdvance Diff = %04X\n", byte - ADVANCE_FIELDDEF_SHORT_0);
				go_on = true;
			} else if ((byte >= FIELD_SIMPLE) && (byte  < FIELD_MAX)) {
				printf_s("\t\tEncoding Index = %04X\n", byte - FIELD_SIMPLE);
				uint32_t encoding = field_encodings[byte - FIELD_SIMPLE];
				printf_s("\t\tStorage = %s\n", field_storage[encoding >> 12]);
				printf_s("\t\tProtection = %s\n", field_protection[(encoding >> 8) & 0xf]);
				printf_s("\t\tType = %04X\n", encoding & 0xff);
			} else if ((byte >= FIELD_INSTANCE) && (byte <= FIELD_RVA)) {
				printf_s("\t\tStorage = %s\n", field_storage[byte - FIELD_INSTANCE]);
				uint8_t b = m_buffer[m_pos++];
				printf_s("\t\tProtection = %s\n", field_protection[b >> 5]);
				printf_s("\t\tType = %04X\n", b & 0x1F);
			} else {
				printf_s("\t\tUnknown %02X\n", byte);
				fine = false;
			}
		} while (go_on);

		if (!fine) break;
	}

	if (fine) for (uint32_t i = 0; i < methodCount; i++) {
		printf_s("\tMethod %d\n", i);
		bool go_on = false;
		do {
			go_on = false;
			uint8_t byte = m_buffer[m_pos++];
			if (byte == ADVANCE_METHODDEF) {
				printf_s("\t\tAdvance Diff = %04X\n", read_compressed_int32());
				go_on = true;
			} else if ((byte >= ADVANCE_METHODDEF_SHORT_MINUS_8) && (byte <= ADVANCE_METHODDEF_SHORT_PLUS_8)) {
				printf_s("\t\tAdvance Diff = %04X\n", byte - ADVANCE_METHODDEF_SHORT_0);
				go_on = true;
			} else if (byte == METHOD) {
				printf_s("\t\tMethod\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("\t\tImpl Attributes = %08X\n", read_compressed_uint32());
				printf_s("\t\tImpl Hints = %08X\n", read_compressed_uint32());
			} else if (byte == NORMAL_METHOD) {
				printf_s("\t\tNormal Method\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
			} else if ((byte >= SIMPLE_METHOD) && (byte < (SIMPLE_METHOD + 32))) {
				printf_s("\t\tSimple Method\n");
				printf_s("\t\tEncoding Index = %04X\n", byte - SIMPLE_METHOD);
			} else if (byte == PINVOKE_METHOD) {
				printf_s("\t\tPInvoke Method\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("\t\tImpl Attributes = %08X\n", read_compressed_uint32());
				printf_s("\t\tImpl Hints = %08X\n", read_compressed_uint32());
				printf_s("\t\tModule Name Offset = %04X\n", read_compressed_uint32());
				printf_s("\t\tEntry Point Name Offset = %08X\n", read_compressed_uint32());
			} else if (byte == NATIVECALLABLE_METHOD) {
				printf_s("\t\tNative Callable Method\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("\t\tImpl Attributes = %08X\n", read_compressed_uint32());
				printf_s("\t\tImpl Hints = %08X\n", read_compressed_uint32());
				printf_s("\t\tEntry Point Name Offset = %08X\n", read_compressed_uint32());
				printf_s("\t\tCalling Convention = %04X\n", read_compressed_uint32());
			} else if (byte == RUNTIME_IMPORT_METHOD) {
				printf_s("\t\tRuntime Import Method\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("\t\tImpl Attributes = %08X\n", read_compressed_uint32());
				printf_s("\t\tImpl Hints = %08X\n", read_compressed_uint32());
				printf_s("\t\tEntry Point Name Offset = %08X\n", read_compressed_uint32());
				printf_s("\t\tCalling Convention = %04X\n", read_compressed_uint32());
			} else if (byte == RUNTIME_EXPORT_METHOD) {
				printf_s("\t\tRuntime Export Method\n");
				printf_s("\t\tAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("\t\tImpl Attributes = %08X\n", read_compressed_uint32());
				printf_s("\t\tImpl Hints = %08X\n", read_compressed_uint32());
				printf_s("\t\tEntry Point Name Offset = %08X\n", read_compressed_uint32());
			} else {
				printf_s("\t\tUnknown %02X\n", byte);
				fine = false;
			}
		} while (go_on);

		if (!fine) break;
	}

	return fine;
}

bool mdil_ctl_parser::dump_ctl_type()
{
	bool ret = true;

	bool go_on = false;
	do {
		go_on = false;

		uint8_t byte = m_buffer[m_pos++];
		switch (byte)
		{
		case ADVANCE_ENCLOSING_TYPEDEF:
			printf_s("Advance Diff = %04X\n", read_compressed_uint32());
			go_on = true;
			break;
		case START_TYPE:
			{
				printf_s("Start\n");
				printf_s("Type Attributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("Base = %08X\n", read_compressed_token());
				uint32_t interfaceCount = read_compressed_uint32();
				printf_s("Interfaces Count = %d\n", interfaceCount);
				uint32_t fieldCount = read_compressed_uint32();
				printf_s("Field Count = %d\n", fieldCount);
				uint32_t methodCount = read_compressed_uint32();
				printf_s("Method Count = %d\n", methodCount);
				printf_s("New Virtual Method Count = %d\n", read_compressed_uint32());
				printf_s("Override Virtual Method Count = %d\n", read_compressed_uint32());
				go_on = dump_ctl_type_members(fieldCount, methodCount, interfaceCount);
				break;
			}
		case SMALL_START_TYPE:
			{
				printf_s("Small Start\n");
				printf_s("TypeAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("Base = %08X\n", read_compressed_token());
				uint32_t counts = read_compressed_uint32();
				printf_s("Field Count = %d\n", counts & 7);
				printf_s("Method Count = %d\n", counts >> 3);
				go_on = dump_ctl_type_members(counts & 7, counts >> 3, 0);
				break;
			}
		case SIMPLE_START_TYPE:
			{
				printf_s("Simple Start\n");
				printf_s("TypeAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("Base = %08X\n", read_compressed_token());
				uint32_t fieldCount = read_compressed_uint32();
				printf_s("Field Count = %d\n", fieldCount);
				uint32_t methodCount = read_compressed_uint32();
				printf_s("Method Count = %d\n", methodCount);
				go_on = dump_ctl_type_members(fieldCount, methodCount, 0);
				break;
			}
		case MODEST_START_TYPE:
			{
				printf_s("Modest Start\n");
				printf_s("TypeAttributes = "); ctl_dump_flags(read_compressed_uint32()); printf_s("\n");
				printf_s("Base = %08X\n", read_compressed_token());
				uint32_t fieldCount = read_compressed_uint32();
				printf_s("Field Count = %d\n", fieldCount);
				uint32_t methodCount = read_compressed_uint32();
				printf_s("Method Count = %d\n", methodCount);
				uint32_t counts = read_compressed_uint32();
				printf_s("Interfaces Count = %d\n", counts & 3);
				printf_s("New Virtual Method Count = %d\n", (counts >> 2) & 3);
				printf_s("Override Virtual Method Count = %d\n", counts >> 4);
				go_on = dump_ctl_type_members(fieldCount, methodCount, counts & 3);
				break;
			}
		case END_TYPE:  printf_s("End\n", byte); break;
		default: {
			if ((byte == 0x6a) && (m_buffer[m_pos] == 0x6f)) {
				printf_s("6A 6F %02X %02X; what's this ?\n", m_buffer[m_pos+1], m_buffer[m_pos+2]);
				m_pos += 3;
				go_on = true;
				break;
			}
			printf_s("Unknown %02X\n", byte); ret = false; break;
		}
		}
	} while (go_on);

	return ret;
}

bool mdil_ctl_parser::dump_cor_type()
{
	bool ret = true;

	uint8_t type = m_buffer[m_pos++];
	switch (type)
	{
	case ELEMENT_TYPE_VOID: printf_s("void"); break;
	case ELEMENT_TYPE_BOOLEAN: printf_s("bool"); break;
	case ELEMENT_TYPE_CHAR: printf_s("char"); break;
	case ELEMENT_TYPE_I1: printf_s("sbyte"); break;
	case ELEMENT_TYPE_U1: printf_s("byte"); break;
	case ELEMENT_TYPE_I2: printf_s("short"); break;
	case ELEMENT_TYPE_U2: printf_s("ushort"); break;
	case ELEMENT_TYPE_I4: printf_s("int"); break;
	case ELEMENT_TYPE_U4: printf_s("uint"); break;
	case ELEMENT_TYPE_I8: printf_s("long"); break;
	case ELEMENT_TYPE_U8: printf_s("ulong"); break;
	case ELEMENT_TYPE_R4: printf_s("float"); break;
	case ELEMENT_TYPE_R8: printf_s("double"); break;
	case ELEMENT_TYPE_STRING: printf_s("string"); break;
	case ELEMENT_TYPE_TYPEDBYREF: printf_s("byref"); break;
	case ELEMENT_TYPE_U: printf_s("uintptr"); break;
	case ELEMENT_TYPE_I: printf_s("intptr"); break;
	case ELEMENT_TYPE_OBJECT:  printf_s("object"); break;
	case ELEMENT_TYPE_VALUETYPE: printf_s("struct"); printf_s("(%08X)", read_compressed_token()); break;
	case ELEMENT_TYPE_CLASS: printf_s("class"); printf_s("(%08X)", read_compressed_token()); break;
	case ELEMENT_TYPE_VAR: printf_s("val"); printf_s("(%08X)", read_compressed_uint32()); break;
	case ELEMENT_TYPE_GENERICINST:
		printf_s("g*");
		if (dump_cor_type()) {
			uint32_t count = read_compressed_uint32();
			printf_s("[%d]", count);
			printf_s("<");
			for (uint32_t i = 0; i < count; i++) {
				if (!dump_cor_type()) { ret = false; break; }
				else { if (i < (count - 1)) printf_s(", "); }
			}
			printf_s(">");
		} else ret = false;
		break;
	case ELEMENT_TYPE_MVAR: printf_s("mval"); printf_s("(%08X)", read_compressed_uint32()); break;
	default: printf_s("unknown(%X)", type); ret = false; break;
	}

	return ret;
}
