#include "stdafx.h"
#include "mdil_ctl_parser.h"

using namespace std;

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

uint8_t mdil_ctl_parser::read_byte()
{
	return m_buffer[m_pos++];
}

uint32_t mdil_ctl_parser::read_uint32_le()
{
	unsigned long ret = m_buffer[m_pos] + (m_buffer[m_pos+1] << 8) + (m_buffer[m_pos+2] << 16) + (m_buffer[m_pos+3] << 24);
	m_pos += 4;
	return ret;
}

int32_t mdil_ctl_parser::read_compressed_int32()
{
	int32_t ret = 0;

	int8_t b = int8_t(m_buffer[m_pos++]);

	if ((b & 1) == 0) {
		ret = b >> 1;
	} else if ((b & 2) == 0) {
		ret = (b >> 2) + (int8_t(m_buffer[m_pos++]) << 6);
	} else if ((b & 4) == 0) {
		ret = (b >> 3) + (int8_t(m_buffer[m_pos]) << 5) + (int8_t(m_buffer[m_pos+1]) << 13);
		m_pos += 2;
	} else if ((b & 8) == 0) {
		ret = (b >> 4) + (int8_t(m_buffer[m_pos]) << 4) + (int8_t(m_buffer[m_pos+1]) << 12) + (int8_t(m_buffer[m_pos+2]) << 20);
		m_pos += 3;
	} else { // 0x0F
		ret = read_uint32_le();
	}

	return ret;
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

void mdil_ctl_parser::debug( const char* format, ... )
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	char msg[512];
	vsprintf_s(msg, format, args);
	printf_s("DBG@%04X: %s\n", m_pos, msg);
	va_end(args);
#endif
}

mdToken mdil_ctl_parser::read_compressed_type_token()
{
	mdToken ret = read_compressed_uint32();
	uint8_t token_type_flag = ret & 3;
	CorTokenType token_type = (token_type_flag == 0) ? mdtModule : (token_type_flag == 1) ? mdtTypeDef : (token_type_flag == 2) ? mdtTypeRef : mdtTypeSpec;
	return (ret >> 2) | token_type;
}

mdToken mdil_ctl_parser::read_compressed_method_token()
{
	mdToken ret = read_compressed_uint32();
	uint8_t token_type_flag = ret & 3;
	CorTokenType token_type = (token_type_flag == 0) ? mdtModule : (token_type_flag == 1) ? mdtMethodDef : (token_type_flag == 2) ? mdtMemberRef : mdtMethodSpec;
	return (ret >> 2) | token_type;
}

bool mdil_ctl_parser::skip_type_def_unknowns()
{
	while (true) {
		uint8_t byte = m_buffer[m_pos];
		if (byte == 0x6a) {
			debug("\t??? 6A");
			m_pos += 1;
		} else if (byte == 0x6b) {
			debug("\t??? 6B");
			m_pos += 1;
		} else if (byte == 0x71) {
			debug("\t??? 71");
			m_pos += 1;
		} else if (byte == 0x73) {
			debug("\t??? 79 %02X", m_buffer[m_pos+1]);
			m_pos += 2;
		} else if (byte == 0x6f) {
			debug("\t??? 6F %02X %02X", m_buffer[m_pos+1], m_buffer[m_pos+2]);
			m_pos += 3;
		} else if (byte == 0x79) {
			debug("\t??? 79 %02X", m_buffer[m_pos+1]);
			m_pos += 2;
		} else if (byte == 0xe5) {
			debug("\t??? E5 %02X", m_buffer[m_pos+1]);
			m_pos += 2;
		} else if (byte == 0x25) {
			debug("\t??? 25 %02X", m_buffer[m_pos+1]);
			m_pos += 2;
		} else if (byte == 0x95) {
			debug("\t??? 95 %02X", m_buffer[m_pos+1]);
			m_pos += 2;
		} else break;
	}
	return true;
}

mdil_type_def* mdil_ctl_parser::parse_type_def(const uint32_t index)
{
	mdil_type_def* ret = nullptr;

	bool fine = true;

	skip_type_def_unknowns();
	
	uint8_t byte = read_byte();

 	mdToken enclosing_type_token = current_enclosing_type_token;

	if (byte == ADVANCE_ENCLOSING_TYPEDEF) {
// 		int32_t diff = read_compressed_int32();
// 		enclosing_type_token += diff;
		enclosing_type_token = mdtTypeDef + read_compressed_uint32(); // looks like its different from Bartok now
		byte = read_byte();
	}

	current_enclosing_type_token = enclosing_type_token;
	
	CorTypeAttr attributes = tdNotPublic; mdToken base_type = mdTokenNil;
	uint32_t field_count = 0, method_count = 0, interface_count = 0;

	switch (byte)
	{
	case START_TYPE:
		{
			attributes = (CorTypeAttr) read_compressed_uint32();
			base_type = read_compressed_type_token();
			interface_count = read_compressed_uint32();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			uint32_t new_count = read_compressed_uint32();
			uint32_t virtual_count = read_compressed_uint32();
			debug("START_TYPE attr=%08X base=%08X fld=%d mtd=%d new=%d vir=%d int=%d", attributes, base_type, field_count, method_count, new_count, virtual_count, interface_count);
			break;
		}
	case SMALL_START_TYPE:
		{
			attributes = (CorTypeAttr) read_compressed_uint32();
			base_type = read_compressed_type_token();
			uint32_t counts = read_compressed_uint32();
			field_count = counts & 7;
			method_count = counts >> 3;
			debug("SMALL_START_TYPE attr=%08X base=%08X fld=%d mtd=%d int=%d", attributes, base_type, field_count, method_count, interface_count);
			break;
		}
	case SIMPLE_START_TYPE:
		{
			attributes = (CorTypeAttr) read_compressed_uint32();
			base_type = read_compressed_type_token();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			debug("SIMPLE_START_TYPE attr=%08X base=%08X int=%d fld=%d mtd=%d", attributes, base_type, interface_count, field_count, method_count);
			break;
		}
	case MODEST_START_TYPE:
		{
			attributes = (CorTypeAttr) read_compressed_uint32();
			base_type = read_compressed_type_token();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			uint32_t counts = read_compressed_uint32();
			uint32_t new_count = (counts >> 2) & 3;
			uint32_t virtual_count = counts >> 4;
			interface_count = counts & 3;
			debug("MODEST_START_TYPE attr=%08X base=%08X int=%d fld=%d mtd=%d new=%d, vir=%d", attributes, base_type, interface_count, field_count, method_count, new_count, virtual_count);
			break;
		}
	default: printf_s("Unknown %02X\n", byte); fine = false; break;
	}

	vector<shared_ptr<mdil_field_def>> fields;

	if (fine) {
		skip_type_def_unknowns();

		fields.resize(field_count);
		for (uint32_t i = 0; i < field_count; i++) {
			debug("\tField %d", i);
			fields[i] = parse_field_def();
			fine = fields[i];
		}
	}

	vector<shared_ptr<mdil_method_def>> methods;
	if (fine) {
		methods.resize(method_count);
		for (uint32_t i = 0; i < method_count; i++) {
			debug("\tMethod %d", i);
			methods[i] = parse_method_def();
			fine = methods[i];
		}
	}

	vector<shared_ptr<mdToken>> interfaces;
	if (fine) {
		interfaces.resize(interface_count, mdTokenNil);
		for (uint32_t i = 0; i < interface_count; i++) {
			debug("\tInterface %d", i);
			uint8_t byte = read_byte();
			if (byte == IMPLEMENT_INTERFACE) {
				interfaces[i].reset(new uint32_t(read_compressed_type_token()));
				debug("\t\tIMPLEMENT_INTERFACE=%08X", *interfaces[i]);
			} else {
				fine = false;
			}

			if (!fine) break;
		}
	}

	vector<shared_ptr<mdil_method_def>> impl_interface_methods;
	if (fine) {
		bool go_on = true;
		do {
			go_on = true;

			uint8_t byte = m_buffer[m_pos]; // peeking

			unique_ptr<int32_t> diff;
			if (byte == ADVANCE_METHODDEF) {
				m_pos++;
				diff.reset(new int32_t(read_compressed_int32()));
				byte = m_buffer[m_pos]; // peeking
			} else if ((byte >= ADVANCE_METHODDEF_SHORT_MINUS_8) && (byte <= ADVANCE_METHODDEF_SHORT_PLUS_8)) {
				m_pos++;
				diff.reset(new int32_t(byte - ADVANCE_METHODDEF_SHORT_0));
				byte = m_buffer[m_pos]; // peeking
			}

			if (byte == IMPLEMENT_INTERFACE_METHOD) {
				m_pos++;
				debug("\tInterface Method %d", impl_interface_methods.size());
				auto method = make_shared<mdil_method_def>();
				method->token = current_method_token;
				if (diff) method->token += *diff;
				current_method_token = method->token;
				method->overridden_method_token = read_compressed_method_token();
				debug("\t\tIMPLEMENT_INTERFACE_METHOD=%08X", method->overridden_method_token);
				impl_interface_methods.push_back(method);
			} else go_on = false;

		} while (go_on);
	}

	skip_type_def_unknowns();

	if (fine) {
		byte = read_byte();
		if (byte != END_TYPE) { debug("expecting END_TYPE got %02X\n", byte); fine = false; }
		else debug("END_TYPE");
	}

	if (!fine) debug("*ERROR");

	if (true) {
		ret = new mdil_type_def;
		ret->token = mdtTypeDef + index;
		ret->enclosing_type_token = enclosing_type_token;
		ret->attributes = attributes;
		ret->base_type_token = base_type;
		ret->fields = fields;
		ret->methods = methods;
		ret->impl_interfaces = interfaces;
		ret->impl_intface_methods = impl_interface_methods;
	}

	return ret;
}

shared_ptr<mdil_field_def> mdil_ctl_parser::parse_field_def()
{
	static const uint32_t field_encodings[] = { 0x0112, 0x1112, 0x0608, 0x0108, 0x0102, 0x0312, 0x0612, 0x1108,
		0x0308, 0x1612, 0x0111, 0x1312, 0x0618, 0x0309, 0x0609, 0x0311, };

	auto field = make_shared<mdil_field_def>();
	bool fine = true;

	field->token = current_field_token + 1;

	uint8_t byte = m_buffer[m_pos++];

	if (byte == ADVANCE_FIELDDEF) {
		field->token += read_compressed_int32();
		byte = read_byte();
	} else if ((byte >= ADVANCE_FIELDDEF_SHORT_MINUS_8) && (byte <= ADVANCE_FIELDDEF_SHORT_PLUS_8)) {
		field->token += (int) byte - ADVANCE_FIELDDEF_SHORT_0;
		byte = read_byte();
	}

	current_field_token = field->token;

	if (byte == FIELD_OFFSET) {
		field->explicit_offset.reset(new uint32_t(read_compressed_uint32()));
		byte = read_byte();
	}

	if ((byte >= FIELD_SIMPLE) && (byte < FIELD_MAX)) {
		uint32_t encoding = field_encodings[byte - FIELD_SIMPLE];
		field->storage = mdil_field_def::field_storage(encoding >> 12);
		field->protection = mdil_field_def::field_protection((encoding >> 8) & 0xf);
		field->element_type = CorElementType(encoding & 0xff);
	} else if ((byte >= FIELD_INSTANCE) && (byte <= FIELD_RVA)) {
		field->storage = mdil_field_def::field_storage(byte - FIELD_INSTANCE);
		uint8_t b = m_buffer[m_pos++];
		field->protection = mdil_field_def::field_protection(b >> 5);
		field->element_type = CorElementType(b & 0x1F);
	} else {
		fine = false;
	}

	if (fine && (field->element_type == ELEMENT_TYPE_VALUETYPE)) {
		field->boxing_type_token = read_compressed_type_token();
	}

	if (!fine) field.reset();
	
	return field;
}

std::shared_ptr<mdil_method_def> mdil_ctl_parser::parse_method_def()
{
	auto method = make_shared<mdil_method_def>();
	bool fine = true;

	skip_type_def_unknowns();

	method->token = current_method_token + 1;

	uint8_t byte = read_byte();

	if (byte == ADVANCE_METHODDEF) {
		int32_t advance_diff = read_compressed_int32();
		debug("\t\tADVANCE_METHODDEF=%d", advance_diff);
		method->token += advance_diff;
		byte = read_byte();
	} else if ((byte >= ADVANCE_METHODDEF_SHORT_MINUS_8) && (byte <= ADVANCE_METHODDEF_SHORT_PLUS_8)) {
		debug("\t\tADVANCE_METHODDEF_SHORT=%d", byte - ADVANCE_METHODDEF_SHORT_0);
		method->token += byte - ADVANCE_METHODDEF_SHORT_0;
		byte = read_byte();
	}

	current_method_token = method->token;

	if (byte == METHOD) {
		debug("\t\tMETHOD");
		method->kind = mdil_method_def::mkNormal;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
	} else if (byte == NORMAL_METHOD) {
		debug("\t\tNORMAL_METHOD");
		method->kind = mdil_method_def::mkNormal;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
	} else if ((byte >= SIMPLE_METHOD) && (byte < (SIMPLE_METHOD + 32))) {
		debug("\t\tSIMPLE_METHOD");
		method->kind = mdil_method_def::mkNormal;
		static const uint32_t method_encodings[] = { 2182u, 129u, 100664774u, 134u, 198u, 131u, 2246u, 268441734u, 2179u, 150u, 196u, 2198u, 481u, 147u, 145u, 805312646u, 454u, 486u, 452u,
													268441731u, 2502u, 1073748113u, 100666822u, 50528710u, 41951382u, 707u, 41951379u, 2177u, 2534u, 2529u, 132u, 109053382u };
		uint32_t encoding = method_encodings[byte - SIMPLE_METHOD];
		method->attributes = (CorMethodAttr) (encoding & 0xffff);
		method->impl_attributes = (CorMethodImpl) ((encoding >> 16) & 0xff);
		method->impl_hints = (mdil_method_def::method_impl_hints) (encoding >> 24);
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
	} else if (byte == PINVOKE_METHOD) {
		debug("\t\tPINVOKE_METHOD");
		method->kind = mdil_method_def::mkPInvoke;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->module_name = read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		read_byte();
	} else if (byte == NATIVECALLABLE_METHOD) {
		debug("\t\tNATIVECALLABLE_METHOD");
		method->kind = mdil_method_def::mkNativeCallable;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		method->calling_convention = read_compressed_uint32();
	} else if (byte == RUNTIME_IMPORT_METHOD) {
		debug("\t\tRUNTIME_IMPORT_METHOD");
		method->kind = mdil_method_def::mkRuntimeImport;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		method->calling_convention = read_compressed_uint32();
	} else if (byte == RUNTIME_EXPORT_METHOD) {
		debug("\t\tRUNTIME_EXPORT_METHOD");
		method->kind = mdil_method_def::mkRuntimeExport;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
	} else {
		fine = false;
	}

	if (!fine) {
		debug("MethodDef parsing error: unknown %02X", byte);
	}

	if (!fine) method.reset();
	return method;
}

mdil_type_spec* mdil_ctl_parser::parse_type_spec()
{
	uint8_t byte = read_byte();

	switch (byte) {
	case ELEMENT_TYPE_VOID:
	case ELEMENT_TYPE_BOOLEAN:
	case ELEMENT_TYPE_CHAR:
	case ELEMENT_TYPE_I1:
	case ELEMENT_TYPE_U1:
	case ELEMENT_TYPE_I2:
	case ELEMENT_TYPE_U2:
	case ELEMENT_TYPE_I4:
	case ELEMENT_TYPE_U4:
	case ELEMENT_TYPE_I8:
	case ELEMENT_TYPE_U8:
	case ELEMENT_TYPE_R4:
	case ELEMENT_TYPE_R8:
	case ELEMENT_TYPE_STRING:
	case ELEMENT_TYPE_I:
	case ELEMENT_TYPE_U:
	case ELEMENT_TYPE_FNPTR: // TODO
	case ELEMENT_TYPE_OBJECT:
		return new mdil_type_spec((CorElementType) byte);
	case ELEMENT_TYPE_PTR:
	case ELEMENT_TYPE_BYREF:
	case ELEMENT_TYPE_TYPEDBYREF:
		return new mdil_type_spec_with_child((CorElementType) byte, parse_type_spec());
	case ELEMENT_TYPE_VALUETYPE:
	case ELEMENT_TYPE_CLASS:
		return new mdil_type_spec_with_token((CorElementType) byte, read_compressed_type_token());
	case ELEMENT_TYPE_VAR:
	case ELEMENT_TYPE_MVAR:
		return new mdil_type_spec_with_number((CorElementType) byte, read_compressed_uint32());
	case ELEMENT_TYPE_ARRAY: {
		auto type = parse_type_spec();
		if (type) {
			uint32_t rank = read_compressed_uint32();

			uint32_t bcount = read_compressed_uint32();
			vector<uint32_t> bounds(bcount);
			for (uint32_t i = 0; i < bcount; ++i) bounds[i] = read_compressed_uint32();

			uint32_t lbcount = read_compressed_uint32();
			vector<uint32_t> lbounds(lbcount);
			for (uint32_t i = 0; i < lbcount; ++i) lbounds[i] = read_compressed_uint32();

			return new mdil_type_spec_array(type, rank, bounds, lbounds);
		} else return new mdil_type_spec_array(nullptr, 0); // invalid
	}
	case ELEMENT_TYPE_SZARRAY:
		return new mdil_type_spec_array(parse_type_spec(), 1);
	case ELEMENT_TYPE_GENERICINST: {
		auto type = parse_type_spec();
		if (type != nullptr) {
			uint32_t count = read_compressed_uint32();
			auto args = vector<shared_ptr<mdil_type_spec>>(count);
			for (uint32_t i=0; i < count; ++i) {
				auto arg = parse_type_spec();
				if (arg == nullptr) break;
				args[i].reset(arg);
			}
			return new mdil_type_spec_generic(type, args);
		} else return new mdil_type_spec_generic(nullptr); // invalid
	}
	default: return nullptr;
	}
}

void mdil_ctl_parser::parse()
{
	current_enclosing_type_token = mdTypeDefNil;
	current_field_token = mdFieldDefNil;
	current_method_token = mdMethodDefNil;

	m_data.type_map.type_defs.resize(m_data.type_map.raw.size());

	for (unsigned long i = 1; i < m_data.type_map.raw.size(); i++) {
		auto type_offset = m_data.type_map.raw->at(i);
		if (type_offset != 0) {
			m_pos = type_offset;
			m_data.type_map.type_defs->at(i).reset(parse_type_def(i));
		}
	}

	m_data.type_specs.type_specs.resize(m_data.type_specs.raw.size());

	for (unsigned long i = 1; i < m_data.type_specs.raw.size(); i++) {
		auto offset = m_data.type_specs.raw->at(i);;
		if (offset != 0) {
			m_pos = offset;
			m_data.type_specs.type_specs->at(i).reset(parse_type_spec());
		}
	}
}
