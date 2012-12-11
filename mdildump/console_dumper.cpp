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

const char* format_element_type(CorElementType type) {
	switch (type)
	{
	case ELEMENT_TYPE_VOID: return "void";
	case ELEMENT_TYPE_BOOLEAN: return "bool";
	case ELEMENT_TYPE_CHAR: return "char";
	case ELEMENT_TYPE_I1: return "sbyte";
	case ELEMENT_TYPE_U1: return "byte";
	case ELEMENT_TYPE_I2: return "short";
	case ELEMENT_TYPE_U2: return "ushort";
	case ELEMENT_TYPE_I4: return "int";
	case ELEMENT_TYPE_U4: return "uint";
	case ELEMENT_TYPE_I8: return "long";
	case ELEMENT_TYPE_U8: return "ulong";
	case ELEMENT_TYPE_R4: return "float";
	case ELEMENT_TYPE_R8: return "double";
	case ELEMENT_TYPE_STRING: return "string";
	case ELEMENT_TYPE_PTR: return "*";
	case ELEMENT_TYPE_BYREF: return "@";
	case ELEMENT_TYPE_VALUETYPE: return "VALUETYPE";
	case ELEMENT_TYPE_CLASS: return "CLASS";
	case ELEMENT_TYPE_VAR: return "VAR";
	case ELEMENT_TYPE_ARRAY: return "ARRAY";
	case ELEMENT_TYPE_GENERICINST: return "GENERICINST";
	case ELEMENT_TYPE_TYPEDBYREF:  return "TYPEDBYREF";
	case ELEMENT_TYPE_I: return "IntPtr";
	case ELEMENT_TYPE_U: return "UIntPtr";
	case ELEMENT_TYPE_FNPTR: return "FNPTR";
	case ELEMENT_TYPE_OBJECT: return "object";
	case ELEMENT_TYPE_SZARRAY: return "SZARRAY";
	case ELEMENT_TYPE_MVAR:	return "MVAR";
	default: return "####";
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

string console_dumper::format_generic_params( const shared_vector<std::shared_ptr<const mdil_generic_param>>& generic_params )
{
	string ret;
	if (generic_params) {
		ret = "<";
		for(uint32_t i = 0; i < generic_params.size(); ++i) {
			auto param = generic_params->at(i);
			if (param->attributes & gpCovariant) ret += "out ";
			else if (param->attributes & gpContravariant) ret += "in ";
			ret += m_metadata->format_token(param->token);
			if (i < (generic_params.size()-1)) ret += ",";
		}
		ret += ">";
	}
	return ret;
}

std::string console_dumper::format_type_name_by_token( mdToken token, bool qualified, bool omit_generic_params )
{
	string ret;

	auto tt = TypeFromToken(token);
	if (tt == mdtTypeDef) {
		bool go_on;
		do {
			string type_name;
			go_on = false;
			if (TypeFromToken(token) == mdtTypeDef) {
				if (token != mdTypeDefNil) {
					auto rid = RidFromToken(token);
					if (rid < m_data.type_map.type_defs.size()) {
						auto type_def = m_data.type_map.type_defs->at(rid);
						type_name = m_metadata->format_token(type_def->token);
						if (!ret.empty() || omit_generic_params) // only omit generic params for it self, not its enclosing types
							type_name += format_generic_params(type_def->generic_params);
						if (qualified) {
							token = type_def->enclosing_type_token;
							go_on = true;
						}
					}
				}
			} else type_name = "(invalid)";

			if (!type_name.empty()) {
				if (ret.empty()) ret = type_name;
				else ret = type_name + "." + ret;
			}
		} while (go_on);
	} else ret = m_metadata->format_token(token); // to do: support type spec

	return ret;
}

std::string console_dumper::format_method_name_by_token( mdToken token, bool qualified, bool omit_generic_params )
{
	string ret;

	auto tt = TypeFromToken(token);
	if (tt == mdtMethodDef) {
		uint32_t rid = RidFromToken(token);
		if (rid < m_data.method_map.method_def_mappings.size()) {
			auto method_def = m_data.method_map.method_def_mappings->at(rid)->method_def;
			if (method_def) {
				if (qualified) ret += format_type_name_by_token(method_def->type_token, true) + ".";
				ret += m_metadata->format_token(method_def->token);
				if (!omit_generic_params) ret += format_generic_params(method_def->generic_params);
			} else ret = "(invalid)";
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
			if (method_def->attributes & mdVirtual) printf_s("virtual ");
			if (method_def->attributes & mdNewSlot) printf_s("new ");
			if (method_def->attributes & mdAbstract) printf_s("abstract ");
		}

		printf_s("%s;", format_method_name_by_token(method_def->token).c_str());

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

		bool is_iface = (type_def->attributes & tdClassSemanticsMask & tdInterface) == tdInterface;

		if ((type_def->attributes & tdAbstract) && (type_def->attributes & tdSealed)) printf_s("static ");
		else {
			if (!is_iface && (type_def->attributes & tdAbstract)) printf_s("abstract ");
			if (type_def->attributes & tdSealed) printf_s("sealed ");
		}

		bool known_base_type = false;

		if (is_iface) {
			printf_s("interface ");
		} else {
			if (!IsNilToken(type_def->base_type_token)) {
				if (type_def->base_type_token == 0x01000001) { // Enum
					printf_s("enum "); known_base_type = true;
				} else if (type_def->base_type_token == 0x01000002) { // Object
					printf_s("class "); known_base_type = true;
				} else if (type_def->base_type_token == 0x01000005) { // ValueType
					printf_s("struct "); known_base_type = true;
				}
			}
			if (!known_base_type) printf_s("class ");
		}

		printf_s("%s", format_type_name_by_token(type_def->token, true).c_str());

		if (((!known_base_type) && (!IsNilToken(type_def->base_type_token))) || (type_def->impl_interfaces.size() > 0))
			printf_s(" : ");

		bool has_base = false;

		if ((!known_base_type) && (!IsNilToken(type_def->base_type_token))) {
			has_base = true;
			if (TypeFromToken(type_def->base_type_token) == mdtTypeSpec) {
				dump_type_spec_by_token(type_def->base_type_token);
			} else printf_s("%s", format_type_name_by_token(type_def->base_type_token).c_str());
		};

		for(auto it = begin(type_def->impl_interfaces); it != end(type_def->impl_interfaces); ++it) {
			if (has_base) printf_s(", "); else has_base = true;
			printf_s("%s", m_metadata->format_token(**it).c_str());
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
					if (TypeFromToken(*it->get()->boxing_type_token) == mdtTypeSpec) {
						dump_type_spec_by_token(*it->get()->boxing_type_token);
						printf_s(" ");
					} else printf_s("%s ", m_metadata->format_token(*it->get()->boxing_type_token).c_str());
				} else {
					printf_s("%s ", format_element_type(it->get()->element_type));
				}
				printf_s("%s;", m_metadata->format_token(it->get()->token).c_str());

				if (it->get()->explicit_offset) printf_s(" // offset = %04X", *it->get()->explicit_offset);
				printf_s("\n");
			} else {
				printf_s("\t/*invalid*/ field;\n");
				break;
			}
		}

		if(type_def->methods.size() > 0) printf_s("// methods\n");
		for(auto it = begin(type_def->methods); it != end(type_def->methods); ++it) {
			dump_method_def(it->get(), is_iface);
			printf_s("\n");
		}

		for(auto it = begin(type_def->impl_interface_methods); it != end(type_def->impl_interface_methods); ++it) {
			auto method = *it;
			if (method) {
				printf_s("\t%s : %s", m_metadata->format_token(method->token).c_str(),
					m_metadata->format_token(method->overridden_method_token).c_str());
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
			printf_s("TYPD(%04X)=TYPE(%04X) :", i, m_data.type_map.raw->at(i));
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
			printf_s("METD(%04X)=%s(%04X) : ", i, mapping->is_generic_inst ? "GENI" : "CODE", mapping->offset);
			if (mapping->method_def) printf_s("%s", format_method_name_by_token(mapping->method_def->token, true).c_str());
			printf_s("\n");
		}
		printf_s("\n");
	} else dump_ulongs(m_data.method_map.raw, title, description);
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
				printf_s("%08X: InstCount = %d, ArgCount = %d\n", pos, genInst->InstCount, genInst->Arity);
				if ((genInst->InstCount == 0) || (genInst->Arity == 0)) break;
				pos += sizeof(GenericInst);
				for (WORD i = 0; i < genInst->InstCount; i++) {
					printf_s("\tInst[%d] = [CODE: %08X DBUG: %08X]\n", i, *(DWORD*) &m_data.generic_instances->at(pos), *(DWORD*) &m_data.generic_instances->at(pos + 4));
					pos += 4 * 2;
				}
				for (uint32_t i = 0; i < genInst->InstCount; ++i) {
					for (uint32_t a = 0; a < genInst->Arity; ++a) {
						uint32_t types = *(DWORD*) (m_data.generic_instances->data() + pos);
						pos += 4;
						printf_s("\tInst[%d] Arg[%d] = ", i, a, types);
						for (uint32_t b = 0; b < 32; ++b) {
							if (types & (1 << b)) {
								if (b <= 0x12) printf_s("%s,", format_element_type((CorElementType)b));
								else if (b == 0x17) printf_s("NULLABLE,");
								else if (b == 0x1e) printf_s("SHARED_VALUETYPE,");
								else if (b == 0x1f) printf_s("SHARED_NULLABLE,");
								else printf_s("???,");
							}
						}
						printf_s("\n");
					}
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


void console_dumper::dump_type_spec( mdil_type_spec* type_spec, bool prefix, bool generic_params )
{
	if (type_spec == nullptr) {
		printf_s("(invalid type spec)");
		return;
	}

	switch (type_spec->element_type)
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
	case ELEMENT_TYPE_PTR: dump_type_spec(((mdil_type_spec_with_child*) type_spec)->child.get()); printf_s("*"); break;
	case ELEMENT_TYPE_BYREF: dump_type_spec(((mdil_type_spec_with_child*) type_spec)->child.get()); printf_s("@"); break;
	case ELEMENT_TYPE_VALUETYPE: printf_s("%s%s",prefix ? "struct " : "", format_type_name_by_token(((mdil_type_spec_with_type*)type_spec)->type_token, true, false).c_str()); break;
	case ELEMENT_TYPE_CLASS: printf_s("%s%s", prefix ? "class " : "", format_type_name_by_token(((mdil_type_spec_with_type*)type_spec)->type_token, true, false).c_str()); break;
	case ELEMENT_TYPE_VAR: printf_s("VAR_%04X", ((mdil_type_spec_with_number*)type_spec)->number); break;
	case ELEMENT_TYPE_ARRAY: {
		auto array = (mdil_type_spec_array*) type_spec;
		dump_type_spec(array->child.get());
		printf_s("[");
		for (uint32_t i = 0; i < array->rank; ++i) {
			if ((i < array->lbounds.size()) && (array->lbounds[i] > 0)) printf_s("%d", array->lbounds[i]);
			if (((i < array->lbounds.size()) && (array->lbounds[i] > 0)) || (i < array->bounds.size())) printf_s(":");
			if (i < array->bounds.size()) printf_s("%d", array->bounds[i]);
			if (i < (array->rank - 1)) printf_s(",");
		}
		printf_s("]");
		break;
							 }
	case ELEMENT_TYPE_GENERICINST: {
		auto inst = (mdil_type_spec_generic*) type_spec;
		dump_type_spec(inst->child.get(), prefix);
		printf_s("<");
		uint32_t count = inst->type_arguments.size();
		for (uint32_t i=0; i < count; ++i) {
			dump_type_spec(inst->type_arguments[i].get());
			if (i < (count-1)) printf_s(",");
		}
		printf_s(">");
		break;
								   }
	case ELEMENT_TYPE_TYPEDBYREF: dump_type_spec(((mdil_type_spec_with_child*)type_spec)->child.get(), prefix); break;
	case ELEMENT_TYPE_I: printf_s("IntPtr"); break;
	case ELEMENT_TYPE_U: printf_s("UIntPtr"); break;
	case ELEMENT_TYPE_FNPTR: printf_s("Function Pointer"); break;
	case ELEMENT_TYPE_OBJECT: printf_s("object"); break;
	case ELEMENT_TYPE_SZARRAY: dump_type_spec(((mdil_type_spec_with_child*)type_spec)->child.get());printf_s("[]"); break;
	case ELEMENT_TYPE_MVAR:	printf_s("MVAR_%04X", ((mdil_type_spec_with_number*)type_spec)->number); break;
	default: printf_s("#%02X#", type_spec->element_type); break;
	}
}

void console_dumper::dump_type_specs( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	if (m_data.type_specs.type_specs && m_data.type_specs.raw)  {
		print_vector_size(m_data.type_specs.type_specs, title, description);

		for (unsigned long i = 1; i < m_data.type_specs.type_specs.size(); i++) {
			printf_s("TYPS(%04X)=TYPE(%04X) : ", i, m_data.type_specs.raw->at(i));
			dump_type_spec(m_data.type_specs.type_specs->at(i).get(), true);
			printf_s("\n");
		}
		printf_s("\n");
	} else dump_ulongs(m_data.type_specs.raw, title, description);
}

void console_dumper::dump_type_spec_by_token( mdTypeSpec token )
{
	if (TypeFromToken(token) == mdtTypeSpec) {
		uint32_t rid = RidFromToken(token);
		if (rid < m_data.type_specs.type_specs.size()) {
			dump_type_spec(m_data.type_specs.type_specs->at(rid).get());
		}
	} else printf_s("(invalid)");
}

void console_dumper::dump_method_specs( const char* title /*= nullptr*/, const char* description /*= nullptr*/ )
{
	if (m_data.method_specs.method_specs && m_data.method_specs.raw)  {
		print_vector_size(m_data.method_specs.method_specs, title, description);

		for (unsigned long i = 1; i < m_data.method_specs.method_specs.size(); i++) {
			auto method_spec = m_data.method_specs.method_specs->at(i);
			printf_s("METS(%04X)=TYPE(%04X) : ", i, m_data.method_specs.raw->at(i));
			printf_s("%s", format_method_name_by_token(method_spec->method_token, true, true).c_str());
			printf_s("<");
			for (uint32_t i = 0; i < method_spec->parameters.size(); ++i) {
				dump_type_spec(method_spec->parameters->at(i).get());
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

void console_dumper::dump_code( const mdil_code& code, const char* title, const char* description )
{
	print_vector_size(code.raw, title, description);

	size_t pos = 0;
	if (code.raw.size() >= 4) {
		unsigned long sig = * (unsigned long*) code.raw->data();
		printf_s("Signature: %s\n", ulong_as_chars(sig).chars);
		pos += 4;
	}
	
	printf_s("\n");

	for (auto m = begin(code.methods); m != end(code.methods); ++m) {
		printf_s("METHOD_%06X:\nSize = %4d (0x%04X) bytes, Routine = %4d (0x%04X) bytes, Exceptions = %d\n",
			m->global_offset, m->size, m->size, m->routine_size, m->routine_size, m->exception_count);
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
			if (!i->annotation.empty()) {
				len = set_to_column(65, len);
				len += printf_s(" ; %s", i->annotation.c_str());
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
