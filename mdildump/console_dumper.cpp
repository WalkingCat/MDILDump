#include "stdafx.h"
#include "console_dumper.h"

using namespace std;

struct ulong_as_chars {
	char chars[5];
	ulong_as_chars(uint32_t ulong) {
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

const wchar_t* format_element_type(CorElementType type) {
	switch (type)
	{
	case ELEMENT_TYPE_VOID: return L"void";
	case ELEMENT_TYPE_BOOLEAN: return L"bool";
	case ELEMENT_TYPE_CHAR: return L"char";
	case ELEMENT_TYPE_I1: return L"sbyte";
	case ELEMENT_TYPE_U1: return L"byte";
	case ELEMENT_TYPE_I2: return L"short";
	case ELEMENT_TYPE_U2: return L"ushort";
	case ELEMENT_TYPE_I4: return L"int";
	case ELEMENT_TYPE_U4: return L"uint";
	case ELEMENT_TYPE_I8: return L"long";
	case ELEMENT_TYPE_U8: return L"ulong";
	case ELEMENT_TYPE_R4: return L"float";
	case ELEMENT_TYPE_R8: return L"double";
	case ELEMENT_TYPE_STRING: return L"string";
	case ELEMENT_TYPE_PTR: return L"*";
	case ELEMENT_TYPE_BYREF: return L"@";
	case ELEMENT_TYPE_VALUETYPE: return L"VALUETYPE";
	case ELEMENT_TYPE_CLASS: return L"CLASS";
	case ELEMENT_TYPE_VAR: return L"VAR";
	case ELEMENT_TYPE_ARRAY: return L"ARRAY";
	case ELEMENT_TYPE_GENERICINST: return L"GENERICINST";
	case ELEMENT_TYPE_TYPEDBYREF:  return L"TYPEDBYREF";
	case ELEMENT_TYPE_I: return L"IntPtr";
	case ELEMENT_TYPE_U: return L"UIntPtr";
	case ELEMENT_TYPE_FNPTR: return L"FNPTR";
	case ELEMENT_TYPE_OBJECT: return L"object";
	case ELEMENT_TYPE_SZARRAY: return L"SZARRAY";
	case ELEMENT_TYPE_MVAR:	return L"MVAR";
	default: return L"####";
	}
}

const wstring format_type_argument(uint32_t types) { // generic instance
	wstringstream ret;
	bool first = true;
	for (uint32_t b = 0; b < 32; ++b) {
		if (types & (1 << b)) {
			if (!first) ret << L"|"; else first = false;
			if (b <= 0x12) ret << format_element_type((CorElementType)b);
			else if (b == 0x17) ret << L"NULLABLE";
			else if (b == 0x1e) ret << L"SHARED_VALUETYPE";
			else if (b == 0x1f) ret << L"SHARED_NULLABLE";
			else ret << L"???";
		}
	}
	return ret.str();
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
	printf_s("\t.section10Count = %d\n", header.section_10_count);
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

wstring console_dumper::format_generic_params( const shared_vector<std::shared_ptr<const mdil_generic_param>>& generic_params )
{
	wstring ret;
	if (generic_params) {
		ret = L"<";
		for(uint32_t i = 0; i < generic_params.size(); ++i) {
			auto param = generic_params->at(i);
			if (param->attributes & gpCovariant) ret += L"out ";
			else if (param->attributes & gpContravariant) ret += L"in ";
			ret += m_metadata->format_token(param->token);
			if (i < (generic_params.size()-1)) ret += L",";
		}
		ret += L">";
	}
	return ret;
}

std::wstring console_dumper::format_type_name( mdToken token, bool qualified, bool omit_generic_params )
{
	wstring ret;

	auto tt = TypeFromToken(token);
	if (tt == mdtTypeDef) {
		bool go_on;
		do {
			wstring type_name;
			go_on = false;
			if (TypeFromToken(token) == mdtTypeDef) {
				if (token != mdTypeDefNil) {
					auto rid = RidFromToken(token);
					if (rid < m_data.type_map.type_defs.size()) {
						auto type_def = m_data.type_map.type_defs->at(rid);
						type_name = m_metadata->format_token(type_def->token);
						if (!(ret.empty() && omit_generic_params)) // only omit generic params for it self, not its enclosing types
							type_name += format_generic_params(type_def->generic_params);
						if (qualified) {
							token = type_def->enclosing_type_token;
							go_on = true;
						}
					}
				}
			} else type_name = L"(invalid)";

			if (!type_name.empty()) {
				if (ret.empty()) ret = type_name;
				else ret = type_name + L"." + ret;
			}
		} while (go_on);
	} else if (TypeFromToken(token) == mdtTypeSpec) {
		uint32_t rid = RidFromToken(token);
		if (rid < m_data.type_specs.type_specs.size()) {
			ret = format_type_spec(m_data.type_specs.type_specs->at(rid).get());
		}
	} else ret = m_metadata->format_token(token);

	return ret;
}

std::wstring console_dumper::format_method_name( mdToken token, bool qualified, bool omit_generic_params )
{
	wstring ret;

	auto tt = TypeFromToken(token);
	if (tt == mdtMethodDef) {
		uint32_t rid = RidFromToken(token);
		if (rid < m_data.method_map.method_def_mappings.size()) {
			auto method_def = m_data.method_map.method_def_mappings->at(rid)->method_def;
			if (method_def) {
				if (qualified) ret += format_type_name(method_def->type_token, true) + L".";
				ret += m_metadata->format_token(method_def->token);
				if (!omit_generic_params) ret += format_generic_params(method_def->generic_params);
			} else ret = L"(invalid)";
		}
	} else ret = m_metadata->format_token(token); //TODO: support method spec

	return ret;
}

void console_dumper::dump_method_def( const mdil_method_def* method_def, bool is_interface )
{
	if (method_def) {
		printf_s("\t");
		switch (method_def->attributes & mdMemberAccessMask)
		{
		case mdPrivateScope: printf_s("/*PrivateScope*/ "); break;
		case mdPrivate: printf_s("private "); break;
		case mdFamANDAssem: printf_s("/*FamANDAssem*/ "); break;
		case mdAssem: printf_s("internal "); break;
		case mdFamily: printf_s("protected "); break;
		case mdFamORAssem: printf_s("protected internal "); break;
		case mdPublic: printf_s("public "); break;
		}
		
		if (method_def->attributes & mdStatic) printf_s("static ");
		if (method_def->attributes & mdFinal) printf_s("sealed ");
		
		if (!is_interface) {
			if (method_def->attributes & mdAbstract) printf_s("abstract ");
			else {
				if (method_def->attributes & mdVirtual) printf_s("virtual ");
				if (method_def->attributes & mdNewSlot) printf_s("new "); // override ????
			}
		}

		printf_s("%S;", format_method_name(method_def->token).c_str());

		if (method_def->impl_hints & mdil_method_def::mihDefault_Ctor) printf_s(" //default ctor");

		if (method_def->attributes & mdPinvokeImpl) printf_s(" //PInvoke [%s]%s", m_data.name_pool->data() + method_def->module_name, m_data.name_pool->data() + method_def->entry_point_name);
		if (method_def->attributes & mdUnmanagedExport) printf_s(" //UnmanagedExport %s", m_data.name_pool->data() + method_def->entry_point_name);
		if (method_def->kind == mdil_method_def::mkRuntimeImport) printf_s(" //RuntimeImport %s", m_data.name_pool->data() + method_def->entry_point_name);
	}
}

void console_dumper::dump_type_def( mdil_type_def* type_def )
{
	static const char* field_storage[] = { "", "static ", "/*ThreadLocal*/ ", "/*ContextLocal*/ ", "/*RVA*/ static " };
	static const char* field_protection[] = { "/*private scope*/ ", "private ", "/*Fam_and_Assem*/ ", "internal ", "protected ", "protected internal ", "public " };

	if (type_def != nullptr) {
		bool notes = false;
		if (type_def->attributes & tdWindowsRuntime) {
			printf_s("// WindowsRuntime");
			notes = true;
		}
		if (type_def->winrt_redirected) {
			if (notes) printf_s(" ");
			printf_s("// WinRT Redirected %02X", *type_def->winrt_redirected);
			notes = true;
		}
		if (notes) printf_s("\n");

		if (type_def->guid_information) {
			printf_s("[Guid(\"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\"),", *(uint32_t*) &type_def->guid_information->guid[0],
				*(uint16_t*) &type_def->guid_information->guid[4], *(uint16_t*) &type_def->guid_information->guid[6],
				type_def->guid_information->guid[8], type_def->guid_information->guid[9],
				type_def->guid_information->guid[10], type_def->guid_information->guid[11],
				type_def->guid_information->guid[12], type_def->guid_information->guid[13],
				type_def->guid_information->guid[14], type_def->guid_information->guid[15]);
			printf_s(" Unknown(%08X)]\n", type_def->guid_information->unknown);
		}

		if ((type_def->attributes & tdLayoutMask) || type_def->layout_pack || type_def->layout_size) {
			printf_s("[StructLayout(");
			switch (type_def->attributes & tdLayoutMask) {
			case tdAutoLayout: printf_s("LayoutKind.Auto"); break;
			case tdSequentialLayout: printf_s("LayoutKind.Sequential"); break;
			case tdExplicitLayout: printf_s("LayoutKind.Explicit"); break;
			default: break;
			}
			if (type_def->layout_pack) printf_s(",Pack=%d", *type_def->layout_pack);
			if (type_def->layout_size) printf_s(",Size=%d", *type_def->layout_size);
			printf_s("]\n");
		}

		switch (type_def->attributes & tdVisibilityMask) {
		case tdPublic: printf_s("public "); break;
		default: break;
		}

		bool is_interface = (type_def->attributes & tdClassSemanticsMask & tdInterface) == tdInterface;

		if ((type_def->attributes & tdAbstract) && (type_def->attributes & tdSealed)) printf_s("static ");
		else {
			if (!is_interface && (type_def->attributes & tdAbstract)) printf_s("abstract ");
			if (type_def->attributes & tdSealed) printf_s("sealed ");
		}

		wstring base_type_name;
		if (!IsNilToken(type_def->base_type_token)) base_type_name = format_type_name(type_def->base_type_token);

		if (is_interface) {
			printf_s("interface ");
		} else {
			if (base_type_name == L"System.ValueType") {
				printf_s("struct "); base_type_name.clear();
			} else if (base_type_name == L"System.Object") {
				printf_s("class "); base_type_name.clear();
			} else if (base_type_name == L"System.Enum") {
				printf_s("enum "); base_type_name.clear();
			} else printf_s("class ");
		}

		printf_s("%S", format_type_name(type_def->token, true).c_str());

		if (!base_type_name.empty() || (type_def->impl_interfaces.size() > 0))
			printf_s(" : ");

		bool has_base = false;

		if (!base_type_name.empty()) {
			has_base = true;
			printf_s("%S", base_type_name.c_str());
		};

		for(auto it = begin(type_def->impl_interfaces); it != end(type_def->impl_interfaces); ++it) {
			if (has_base) printf_s(", "); else has_base = true;
			printf_s("%S", m_metadata->format_token(**it).c_str());
		}
		if (type_def->attributes & tdSpecialName) printf_s(" // SpecialName");
		if (type_def->attributes & tdRTSpecialName) printf_s(" // RTSpecialName");
		printf_s("\n");

// 		m_metadata_reader->dump_type(type_def->token);

		printf_s("{\n");
		if(type_def->fields.size() > 0) printf_s("// fields\n");
		for(auto it = begin(type_def->fields); it != end(type_def->fields); ++it) {
			if (*it) {
				printf_s("\t%s%s", field_storage[it->get()->storage], field_protection[it->get()->protection]);
				if (it->get()->boxing_type_token) {
					printf_s("%S ", format_type_name(*it->get()->boxing_type_token).c_str());
				} else {
					printf_s("%S ", format_element_type(it->get()->element_type));
				}
				printf_s("%S;", m_metadata->format_token(it->get()->token).c_str());

				if (it->get()->explicit_offset) printf_s(" // offset = %04X", *it->get()->explicit_offset);
				printf_s("\n");
			} else {
				printf_s("\t/*invalid*/ field;\n");
				break;
			}
		}

		if(type_def->methods.size() > 0) printf_s("// methods\n");
		for(auto it = begin(type_def->methods); it != end(type_def->methods); ++it) {
			dump_method_def(it->get(), is_interface);
			printf_s("\n");
		}

		for(auto it = begin(type_def->impl_interface_methods); it != end(type_def->impl_interface_methods); ++it) {
			auto method = *it;
			if (method) {
				printf_s("\t%S : %S", format_method_name(method->token).c_str(),
					format_method_name(method->overridden_method_token, true).c_str());
				printf_s("\n");
			}
		}
		printf_s("}\n");
	}
}

void console_dumper::dump_type_map( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	if (m_data.type_map.type_defs && m_data.type_map.raw)  {
		print_vector_size(m_data.type_map.type_defs, title, description);

		for (unsigned long i = 1; i < m_data.type_map.type_defs.size(); i++) {
			if (m_data.type_map.raw->at(i) == 0) continue;
			printf_s("TYPD(%04X)=TYPE(%04X):", i, m_data.type_map.raw->at(i));
			if (m_data.type_map.type_defs->at(i)) {
				printf_s("\n");
				dump_type_def(m_data.type_map.type_defs->at(i).get());
			} else printf_s(" (invalid)\n", i, m_data.type_map.raw->at(i));

		}
		printf_s("\n");
	} else dump_ulongs(m_data.type_map.raw, title, description);
}

void console_dumper::dump_method_map(const char* title, const char* description)
{
	if (m_data.method_map.method_def_mappings && m_data.method_map.raw) {
		print_vector_size(m_data.method_map.method_def_mappings, title, description);

		for (uint32_t i = 1; i < m_data.method_map.method_def_mappings.size(); ++i) {
			auto mapping = m_data.method_map.method_def_mappings->at(i);
			printf_s("METD(%04X)=%s(%04X): ", i, mapping->is_generic_inst ? "GENI" : "CODE", mapping->offset);
			if (mapping->method_def) printf_s("%S", format_method_name(mapping->method_def->token, true).c_str());
			printf_s("\n");
		}
		printf_s("\n");
	} else dump_ulongs(m_data.method_map.raw, title, description);
}

void console_dumper::dump_generic_instances( const char* title, const char* description )
{
	if (!m_data.generic_instances.generic_methods.empty() && m_data.generic_instances.raw) {
		print_vector_size(m_data.generic_instances.raw, title, description);

		if (m_data.generic_instances.raw.size() >= 4) {
			unsigned long sig = * (unsigned long*) m_data.generic_instances.raw->data();
			printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		}

		unordered_map<uint32_t, mdToken> generic_methods_map;
		if (m_data.method_map.method_def_mappings) {
			for (uint32_t i = 1; i < m_data.method_map.method_def_mappings.size(); ++i) {
				auto mapping = m_data.method_map.method_def_mappings->at(i);
				if (mapping && mapping->is_generic_inst && mapping->method_def) {
					generic_methods_map[mapping->offset] = mapping->method_def->token;
				}
			}
		}

		map<uint32_t, shared_ptr<mdil_generic_method>> ordered_map(begin(m_data.generic_instances.generic_methods), end(m_data.generic_instances.generic_methods));
		for (auto it = begin(ordered_map); it != end(ordered_map); ++it) {
			auto generic_method = it->second;
			
			printf_s("GENI(%04X)", it->first);
			auto method = generic_methods_map.find(it->first);
			if (method != generic_methods_map.end()) printf_s("=METD(%04X): %S\n", RidFromToken(method->second), format_method_name(method->second, true).c_str());
			else printf_s(":\n");

			if (generic_method) {
				printf_s("Instances = %d, Arguments = %d\n", generic_method->instance_count, generic_method->argument_count);
				for (uint16_t i = 0; i < generic_method->instance_count; ++i) {
					printf_s("\tInstance %d: CODE(%04X), DBUG(%04X)\n", i, generic_method->instances[i]->code_offset, generic_method->instances[i]->debug_offset);
					for (uint8_t a = 0; a < generic_method->argument_count; ++a) {
						uint32_t types = generic_method->instances[i]->argument_types[a];
						printf_s("\t\tArgument %d: %S\n", a, format_type_argument(types).c_str());
					}
				}
			} else printf_s("(invalid)\n");
		}
	} else dump_bytes(m_data.generic_instances.raw, title, description);
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
		printf_s("MODR(%04X)=NAME(%04X)", i, m_data.ext_module_refs->at(i).ModName);
		if (m_data.ext_module_refs->at(i).ModName != 0)
			printf_s(": %s (Ref: %s)\n", format_ext_module_ref(i).c_str(), &m_data.name_pool->at(m_data.ext_module_refs->at(i).RefName));
		else printf_s("\n");
	}
	printf_s("\n");
}

void console_dumper::dump_ext_type_refs( const char* title, const char* description )
{
	print_vector_size(m_data.ext_type_refs, title, description);

	for (unsigned long i = 1; i < m_data.ext_type_refs.size(); i++) {
		auto ref = m_data.ext_type_refs->at(i);
 		printf_s("TYPR(%04X)=MODR(%04X)[%04X]", i, ref.module, ref.ordinal, i);
		auto name = m_metadata->format_token(mdtTypeRef | i, true);
		if (!name.empty()) printf_s(": %S\n", name.c_str());
		else printf_s("\n");
	}
	printf_s("\n");
}

std::wstring console_dumper::format_member_ref_name( mdMemberRef token, bool no_fallback /*= false*/ )
{
	wstringstream ret;

	auto rid = RidFromToken(token);

	if (rid < m_data.ext_member_refs.size()) {
		auto ref = m_data.ext_member_refs->at(rid);

		wstring type_name = ref.isTypeSpec ? format_type_name(mdtTypeSpec | ref.extTypeRid) : m_metadata->format_token(mdtTypeRef | ref.extTypeRid, no_fallback);
		wstring name = m_metadata->format_token(mdtMemberRef | rid, true);

		if (!type_name.empty()) ret << type_name << L".";

		if (!name.empty()) {
			ret << name;
		} else {
			if (!(type_name.empty() && no_fallback)) {
				ret << (ref.isField ? L"field" : L"method") << L"_" << hex << setw(6) << setfill(L'0') << ref.ordinal;
			}
		}
	}

	return ret.str();
}

void console_dumper::dump_ext_member_refs( const char* title, const char* description )
{
	print_vector_size(m_data.ext_member_refs, title, description);

	for (unsigned long i = 1; i < m_data.ext_member_refs.size(); i++) {
		auto ref = m_data.ext_member_refs->at(i);
		printf_s("MEMR(%04X)=%S(%04X)[(%S)%04X]", i, ref.isTypeSpec ? L"TYPS" : L"TYPR", ref.extTypeRid, ref.isField ? L" FIELD" : L"METHOD", ref.ordinal);
		auto name = format_member_ref_name(mdtMemberRef | i, true);
		if (!name.empty()) printf_s(": %S\n", name.c_str());
		else printf_s("\n");
	}
	printf_s("\n");
}

wstring console_dumper::format_type_spec( mdil_type_spec* type_spec, bool prefix )
{
	wstringstream ret;

	if (type_spec != nullptr) {
		switch (type_spec->element_type)
		{
		case ELEMENT_TYPE_VOID: ret << L"void"; break;
		case ELEMENT_TYPE_BOOLEAN: ret << L"bool"; break;
		case ELEMENT_TYPE_CHAR: ret << L"char"; break;
		case ELEMENT_TYPE_I1: ret << L"sbyte"; break;
		case ELEMENT_TYPE_U1: ret << L"byte"; break;
		case ELEMENT_TYPE_I2: ret << L"short"; break;
		case ELEMENT_TYPE_U2: ret << L"ushort"; break;
		case ELEMENT_TYPE_I4: ret << L"int"; break;
		case ELEMENT_TYPE_U4: ret << L"uint"; break;
		case ELEMENT_TYPE_I8: ret << L"long"; break;
		case ELEMENT_TYPE_U8: ret << L"ulong"; break;
		case ELEMENT_TYPE_R4: ret << L"float"; break;
		case ELEMENT_TYPE_R8: ret << L"double"; break;
		case ELEMENT_TYPE_STRING: ret << L"string"; break;
		case ELEMENT_TYPE_PTR: ret << format_type_spec(((mdil_type_spec_with_child*) type_spec)->child.get()) << L"*"; break;
		case ELEMENT_TYPE_BYREF: ret << format_type_spec(((mdil_type_spec_with_child*) type_spec)->child.get()) << L"@"; break;
		case ELEMENT_TYPE_VALUETYPE: ret << (prefix ? L"struct " : L"") << format_type_name(((mdil_type_spec_with_type*)type_spec)->type_token, true, false); break;
		case ELEMENT_TYPE_CLASS: ret << (prefix ? L"class " : L"") << format_type_name(((mdil_type_spec_with_type*)type_spec)->type_token, true, false); break;
		case ELEMENT_TYPE_VAR: ret << L"VAR_" << hex << setfill(L'0') << setw(4)  << ((mdil_type_spec_with_number*)type_spec)->number; break;
		case ELEMENT_TYPE_ARRAY: {
			auto array = (mdil_type_spec_array*) type_spec;
			ret << format_type_spec(array->child.get());
			ret << L"[";
			for (uint32_t i = 0; i < array->rank; ++i) {
				if ((i < array->lbounds.size()) && (array->lbounds[i] > 0)) ret << array->lbounds[i];
				if (((i < array->lbounds.size()) && (array->lbounds[i] > 0)) || (i < array->bounds.size())) ret << L":";
				if (i < array->bounds.size()) ret << array->bounds[i];
				if (i < (array->rank - 1)) ret << L",";
			}
			ret << L"]";
			break;
								 }
		case ELEMENT_TYPE_GENERICINST: {
			auto inst = (mdil_type_spec_generic*) type_spec;
			ret << format_type_spec(inst->child.get(), prefix);
			ret << L"<";
			uint32_t count = inst->type_arguments.size();
			for (uint32_t i=0; i < count; ++i) {
				ret << format_type_spec(inst->type_arguments[i].get());
				if (i < (count-1)) ret << L",";
			}
			ret << ">";
			break;
									   }
		case ELEMENT_TYPE_TYPEDBYREF: ret << format_type_spec(((mdil_type_spec_with_child*)type_spec)->child.get(), prefix); break;
		case ELEMENT_TYPE_I: ret << L"IntPtr"; break;
		case ELEMENT_TYPE_U: ret << L"UIntPtr"; break;
		case ELEMENT_TYPE_FNPTR: ret << L"FnPtr"; break;
		case ELEMENT_TYPE_OBJECT: ret << L"object"; break;
		case ELEMENT_TYPE_SZARRAY: ret << format_type_spec(((mdil_type_spec_with_child*)type_spec)->child.get(), prefix) << L"[]"; break;
		case ELEMENT_TYPE_MVAR:	ret << L"MVAR" << hex << setw(4) << setfill(L'0') << ((mdil_type_spec_with_number*)type_spec)->number; break;
		default: ret << L"#" << hex << setw(2) << setfill(L'0') << type_spec->element_type << L"#"; break;
		}
	} else ret << L"(invalid)";

	return ret.str();
}

void console_dumper::dump_type_specs( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	if (m_data.type_specs.type_specs && m_data.type_specs.raw)  {
		print_vector_size(m_data.type_specs.type_specs, title, description);

		for (unsigned long i = 1; i < m_data.type_specs.type_specs.size(); i++) {
			printf_s("TYPS(%04X)=TYPE(%04X) : ", i, m_data.type_specs.raw->at(i));
			printf_s("%S", format_type_spec(m_data.type_specs.type_specs->at(i).get(), true).c_str());
			printf_s("\n");
		}
		printf_s("\n");
	} else dump_ulongs(m_data.type_specs.raw, title, description);
}

void console_dumper::dump_method_specs( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	if (m_data.method_specs.method_specs && m_data.method_specs.raw)  {
		print_vector_size(m_data.method_specs.method_specs, title, description);

		for (unsigned long i = 1; i < m_data.method_specs.method_specs.size(); i++) {
			auto method_spec = m_data.method_specs.method_specs->at(i);
			printf_s("METS(%04X)=TYPE(%04X): ", i, m_data.method_specs.raw->at(i));
			printf_s("%S", format_method_name(method_spec->method_token, true, true).c_str());
			printf_s("<");
			for (uint32_t i = 0; i < method_spec->parameters.size(); ++i) {
				printf_s("%S", format_type_spec(method_spec->parameters->at(i).get()).c_str());
				if (i < (method_spec->parameters.size() - 1)) printf_s(",");
			}
			printf_s(">\n");
		}
		printf_s("\n");
	} else dump_ulongs(m_data.method_specs.raw, title, description);
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

	for (auto i = begin(*m_data.type_map.raw); i != end(*m_data.type_map.raw); ++i) {
		offsets[*i] = 1;
	}

	for (auto i = begin(*m_data.type_specs.raw); i != end(*m_data.type_specs.raw); ++i) {
		offsets[*i] = 2;
	}

	for (auto i = begin(*m_data.method_specs.raw); i != end(*m_data.method_specs.raw); ++i) {
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

void console_dumper::build_code_map()
{
	if (m_code_map.empty()) { // if not empty, then this has been run
		if (m_data.method_map.method_def_mappings) {
			for (uint32_t i = 1; i < m_data.method_map.method_def_mappings.size(); ++i) {
				auto mapping = m_data.method_map.method_def_mappings->at(i);
				if (mapping && mapping->method_def) {
					if (!mapping->is_generic_inst) {
						code_mapping cm;
						cm.generic_inst = 0;
						cm.is_generic_inst = false;
						cm.method_token = mapping->method_def->token;
						m_code_map[mapping->offset] = cm;
					} else {
						auto inst = m_data.generic_instances.generic_methods.find(mapping->offset);
						if ((inst != m_data.generic_instances.generic_methods.end()) && inst->second) {
							for (uint32_t in = 0; in < inst->second->instances.size(); ++in) {
								if (inst->second->instances[in]) {
									code_mapping cm;
									cm.generic_inst = in;
									cm.argument_types = inst->second->instances[in]->argument_types;
									cm.is_generic_inst = true;
									cm.method_token = mapping->method_def->token;
									m_code_map[inst->second->instances[in]->code_offset] = cm;
								}
							}
						}
					}
				}
			}
		}
	}
}


void console_dumper::dump_code( const mdil_code& code, const char* title, const char* description )
{
	print_vector_size(code.raw, title, description);

	if (code.raw.size() >= 4) {
		unsigned long sig = * (unsigned long*) code.raw->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
	}
	
	printf_s("\n");

	build_code_map();

	for (auto m = begin(code.methods); m != end(code.methods); ++m) {
		printf_s("CODE(%04X)", m->global_offset);
		auto mapping = m_code_map.find(m->global_offset);
		if (mapping != m_code_map.end()) {
			printf_s("=METD(%04X): %S\n", RidFromToken(mapping->second.method_token), format_method_name(mapping->second.method_token, true).c_str());
			for (uint32_t a = 0; a < mapping->second.argument_types.size(); ++a) {
				printf_s("\tType Argument %d: %S\n", a, format_type_argument(mapping->second.argument_types[a]).c_str());
			}
		} else printf_s(":\n");
		printf_s("Size = %4d (0x%04X) bytes, Routine = %4d (0x%04X) bytes, Exceptions = %d\n",
			m->size, m->size, m->routine_size, m->routine_size, m->exception_count);
		dump_bytes_int(code.raw, m->offset + m->routine_offset, m->routine_size);
		if (!m->routine.empty()) dump_instructions(m->routine, code.raw->data() + m->offset + m->routine_offset, m->routine_size);
		printf_s("\n");
	}
}

int set_to_column(int col, int cur) {
	if (cur > col) { putchar('\n'); cur = 0; }
	for (int i = cur; i < col; ++i) putchar(' ');
	return col;
}

void console_dumper::dump_instructions( const std::vector<std::shared_ptr<mdil_instruction>>& code, unsigned char* data, unsigned long count )
{
	for(auto i : code) {
		int len = 0;
		len += printf_s("MDIL_%04X:", i->offset);
		for (unsigned long pos = i->offset; pos < (i->offset + i->length); ++pos) {
			len += printf_s(" %02X", data[pos]);
		}

		len = set_to_column(25, len);

		if (!i->opcode.empty()) {
			len += printf_s(" %s", i->opcode.c_str());

			if (!i->operands.empty()) {
				len = set_to_column(45, len);
				len += printf_s(" %s", i->operands.c_str());
			}
			if (i->ref_type != i->rtNone) {
				wstringstream ref_ss;
				if (i->ref_type == i->rtMetadataToken) {
					switch (TypeFromToken(i->ref_value))
					{
					case mdtTypeDef:
					case mdtTypeRef:
					case mdtTypeSpec:
						ref_ss << format_type_name(i->ref_value, true);
						break;
					case mdtMethodDef:
					case mdtMethodSpec:
						ref_ss << format_method_name(i->ref_value, true);
						break;
					case mdtMemberRef:
						ref_ss << format_member_ref_name(i->ref_value);
						break;
					default:
						ref_ss << m_metadata->format_token(i->ref_value);
						break;
					}
				} else if (i->ref_type == i->rtJumpDistance) {
					ref_ss << "MDIL_" << uppercase << hex << setfill(L'0') << setw(4) << (i->offset + i->length + (int) i->ref_value); 
				}

				auto ref_str = ref_ss.str();
				if (!ref_str.empty()) {
					len = set_to_column(65, len);
					len += printf_s(" ; %S", ref_str.c_str());
				}
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
