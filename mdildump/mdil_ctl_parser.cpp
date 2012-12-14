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
	ENCLOSING_TYPEDEF, // ADVANCE_ENCLOSING_TYPEDEF,
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
	RUNTIME_EXPORT_METHOD,
	// below are new
	GENERIC_TYPE_1 = 0x6A,
	GENERIC_TYPE_2 = 0x6B,
	GENERIC_TYPE = 0x6C,
	PACK_TYPE = 0x6D,
	SIZE_TYPE = 0x6E,
	GENERIC_PARAMETER = 0x6F,
	NATIVE_FIELD = 0x70,
	GUID_INFORMATION = 0x71,
	STUB_METHOD = 0x72,
	EXTENDED_TYPE_FLAGS = 0x73,
	SPECIAL_TYPE = 0x74,
};


uint8_t mdil_ctl_parser::peek_byte()
{
	return m_buffer[m_pos];
}

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
		if (b != 0xf) log_type_def("invalid compressed %02X", b);
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
		if (b != 0xf) log_type_def("invalid compressed %02X", b);
		ret = read_uint32_le();
	}

	return ret;
}

void mdil_ctl_parser::log_type_def( const char* format, ... )
{
#if 0
	va_list args;
	va_start(args, format);
	char msg[512];
	vsprintf_s(msg, format, args);
	printf_s("TPD@%04X: %s\n", m_pos, msg);
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
	if (token_type == mdtModule) log_type_def("invalid method token");
	return (ret >> 2) | token_type;
}

shared_ptr<mdil_type_def> mdil_ctl_parser::parse_type_def(const uint32_t index)
{
	auto ret = make_shared<mdil_type_def>(mdtTypeDef | index);

	bool fine = true;

	uint8_t byte = read_byte();

	shared_ptr<uint32_t> generic_parameter_count;
	if (byte == GENERIC_TYPE_1) {
		generic_parameter_count = make_shared<uint32_t>(1);
		log_type_def("GENERIC_TYPE_1");
	} else if (byte == GENERIC_TYPE_2) {
		generic_parameter_count = make_shared<uint32_t>(2);
		log_type_def("GENERIC_TYPE_2");
	} else if (byte == GENERIC_TYPE) {
		generic_parameter_count = make_shared<uint32_t>(read_compressed_uint32());
		log_type_def("GENERIC_TYPE %d", *generic_parameter_count);
	}

	if (generic_parameter_count) {
		ret->generic_params.resize(*generic_parameter_count);
		for (uint32_t i = 0; i < *generic_parameter_count; ++i) {
			byte = read_byte();
			if (byte == GENERIC_PARAMETER) {
				uint32_t rid = read_compressed_uint32(); // what ??
				CorGenericParamAttr attributes = (CorGenericParamAttr) read_compressed_uint32(); // only variance ?
				ret->generic_params->at(i) = make_shared<mdil_generic_param>(mdtGenericParam | rid, attributes);
				log_type_def("GENERIC_PARAMETER %06X %08X", rid, attributes);

			} else { log_type_def("expecting GENERIC_PARAMTER got %02X", byte); fine = false; }
		}
		byte = read_byte();
	}

 	ret->enclosing_type_token = mdTypeDefNil;

	if (byte == ENCLOSING_TYPEDEF) {
		ret->enclosing_type_token = mdtTypeDef + read_compressed_uint32();
		log_type_def("ENCLOSING_TYPEDEF=%08X", ret->enclosing_type_token);
		byte = read_byte();
	}

	uint32_t field_count = 0, method_count = 0, interface_count = 0;

	switch (byte)
	{
	case START_TYPE:
		{
			ret->attributes = (CorTypeAttr) read_compressed_uint32();
			ret->base_type_token = read_compressed_type_token();
			interface_count = read_compressed_uint32();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			uint32_t new_count = read_compressed_uint32();
			uint32_t virtual_count = read_compressed_uint32();
			log_type_def("START_TYPE attr=%08X base=%08X fld=%d mtd=%d new=%d vir=%d int=%d", ret->attributes, ret->base_type_token, field_count, method_count, new_count, virtual_count, interface_count);
			break;
		}
	case SMALL_START_TYPE:
		{
			ret->attributes = (CorTypeAttr) read_compressed_uint32();
			ret->base_type_token = read_compressed_type_token();
			uint32_t counts = read_compressed_uint32();
			field_count = counts & 7;
			method_count = counts >> 3;
			log_type_def("SMALL_START_TYPE attr=%08X base=%08X fld=%d mtd=%d int=%d", ret->attributes, ret->base_type_token, field_count, method_count, interface_count);
			break;
		}
	case SIMPLE_START_TYPE:
		{
			ret->attributes = (CorTypeAttr) read_compressed_uint32();
			ret->base_type_token = read_compressed_type_token();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			log_type_def("SIMPLE_START_TYPE attr=%08X base=%08X int=%d fld=%d mtd=%d", ret->attributes, ret->base_type_token, interface_count, field_count, method_count);
			break;
		}
	case MODEST_START_TYPE:
		{
			ret->attributes = (CorTypeAttr) read_compressed_uint32();
			ret->base_type_token = read_compressed_type_token();
			field_count = read_compressed_uint32();
			method_count = read_compressed_uint32();
			uint32_t counts = read_compressed_uint32();
			uint32_t new_count = (counts >> 2) & 3;
			uint32_t virtual_count = counts >> 4;
			interface_count = counts & 3;
			log_type_def("MODEST_START_TYPE attr=%08X base=%08X int=%d fld=%d mtd=%d new=%d, vir=%d", ret->attributes, ret->base_type_token, interface_count, field_count, method_count, new_count, virtual_count);
			break;
		}
	default: log_type_def("expecting *_START_TYPE start got %02X", byte); fine = false; break;
	}

	if (fine) {
		byte = peek_byte();

		if (byte == PACK_TYPE) {
			read_byte();
			ret->layout_pack = make_shared<uint32_t>(read_compressed_uint32());
			log_type_def("PACK_TYPE=%08X", *ret->layout_pack);
			byte = peek_byte();
		}

		if (byte == EXTENDED_TYPE_FLAGS) {
			read_byte();
			ret->extended_flags = make_shared<uint32_t>(read_compressed_uint32());
			log_type_def("EXTENDED_TYPE_FLAGS=%08X", *ret->extended_flags);
			byte = peek_byte();
		}

		if (byte == SPECIAL_TYPE) {
			read_byte();
			ret->winrt_redirected = make_shared<uint32_t>(read_compressed_uint32());
			log_type_def("SPECIAL_TYPE=%08X", *ret->winrt_redirected);
			byte = peek_byte();
		}

		if (byte == GUID_INFORMATION) {
			read_byte();
			auto guid_info = make_shared<mdil_type_guid>();
			for (int i = 0; i < 16; i++) guid_info->guid[i] = read_byte();
			guid_info->unknown = read_compressed_uint32();
			ret->guid_information = guid_info;
			log_type_def("GUID_INFORMATION %08X", guid_info->unknown);
		}
	}

	if (fine) {
		ret->fields.resize(field_count);
		for (uint32_t i = 0; i < field_count; i++) {
			log_type_def("\tField %d", i);
			ret->fields[i] = parse_field_def();
			fine = ret->fields[i];
			if (!fine) break;
		}
	}

	if (fine) {
		while (true) {
			uint32_t pos = m_pos;
			if(parse_field_def(true) == nullptr) { // NativeField
				m_pos = pos;
				break;
			}
		}
	}

	if (fine) {
		byte = peek_byte();

		if (byte == SIZE_TYPE) {
			read_byte();
			ret->layout_size = make_shared<uint32_t>(read_compressed_uint32());
			log_type_def("SIZE_TYPE=%08X", *ret->layout_size);
			byte = peek_byte();
		}
	}

	if (fine) {
		ret->methods.resize(method_count);
		for (uint32_t i = 0; i < method_count; i++) {
			log_type_def("\tMethod %d", i);
			auto method = parse_method_def();
			if (method) {
				method->type_token = ret->token;
				ret->methods[i] = method;
			} else ret->methods[i].reset();

			fine = ret->methods[i];
			if (!fine) break;
		}
	}

	if (fine) {
		ret->impl_interfaces.resize(interface_count, mdTokenNil);
		for (uint32_t i = 0; i < interface_count; i++) {
			log_type_def("\tInterface %d", i);
			uint8_t byte = read_byte();
			if (byte == IMPLEMENT_INTERFACE) {
				ret->impl_interfaces[i].reset(new uint32_t(read_compressed_type_token()));
				log_type_def("\t\tIMPLEMENT_INTERFACE=%08X", *ret->impl_interfaces[i]);
			} else {
				log_type_def("\t\texpecting IMPLEMENT_INTERFACE got %02X", byte);
				fine = false;
			}
			if (!fine) break;
		}
	}

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
				log_type_def("\tInterface Method %d", ret->impl_interface_methods.size());
				log_type_def("\t\tADVANCE_METHODDEF=%d", *diff);
			} else if ((byte >= ADVANCE_METHODDEF_SHORT_MINUS_8) && (byte <= ADVANCE_METHODDEF_SHORT_PLUS_8)) {
				m_pos++;
				diff.reset(new int32_t(byte - ADVANCE_METHODDEF_SHORT_0));
				byte = m_buffer[m_pos]; // peeking
				log_type_def("\tInterface Method %d", ret->impl_interface_methods.size());
				log_type_def("\t\tADVANCE_METHODDEF_SHORT=%d", *diff);
			}

			if (byte == IMPLEMENT_INTERFACE_METHOD) {
				m_pos++;
				if (!diff) log_type_def("\tInterface Method %d", ret->impl_interface_methods.size());
				auto method = make_shared<mdil_method_def>();
				method->token = current_method_token + 1;
				if (diff) method->token += *diff; // logic here is different from method def ??? bizarre !!!
				current_method_token = method->token;
				method->overridden_method_token = read_compressed_method_token();
				log_type_def("\t\tIMPLEMENT_INTERFACE_METHOD=%08X", method->overridden_method_token);
				ret->impl_interface_methods.push_back(method);
			} else {
				if (diff) {
					log_type_def("\texpecting IMPLEMENT_INTERFACE_METHOD got %02X", byte);
					fine = false;
				}
				go_on = false;
			}

		} while (go_on);
	}

	if (fine) {
		byte = read_byte();
		if (byte != END_TYPE) { log_type_def("expecting END_TYPE got %02X", byte); fine = false; }
		else log_type_def("END_TYPE");
	}

	if (!fine) {
		log_type_def("*ERROR*");
		ret.reset();
	}

	return ret;
}

shared_ptr<mdil_field_def> mdil_ctl_parser::parse_field_def(bool peek)
{
	static const uint32_t field_encodings[] = { 0x0112, 0x1112, 0x0608, 0x0108, 0x0102, 0x0312, 0x0612, 0x1108,
		0x0308, 0x1612, 0x0111, 0x1312, 0x0618, 0x0309, 0x0609, 0x0311, };

	auto field = make_shared<mdil_field_def>();
	bool fine = true;

	field->token = current_field_token;

	uint8_t byte = m_buffer[m_pos++];

	if (byte == ADVANCE_FIELDDEF) {
		int32_t advance_diff = read_compressed_uint32();
		log_type_def("\t\tADVANCE_FIELDDEF=%d", advance_diff);
		field->token = TokenFromRid(advance_diff, mdtFieldDef); // not advancing but reset !?!
		byte = read_byte();
	} else if ((byte >= ADVANCE_FIELDDEF_SHORT_MINUS_8) && (byte <= ADVANCE_FIELDDEF_SHORT_PLUS_8)) {
		log_type_def("\t\tADVANCE_FIELDDEF_SHORT=%d", byte - ADVANCE_FIELDDEF_SHORT_0);
		field->token = TokenFromRid(byte - ADVANCE_FIELDDEF_SHORT_0, mdtFieldDef); // not advancing but reset !?!
		byte = read_byte();
	}

	field->token++;

	if (byte == FIELD_OFFSET) {
		uint32_t offset = read_compressed_uint32();
		log_type_def("\t\tFIELD_OFFSET=%04X", offset);
		field->explicit_offset.reset(new uint32_t(offset));
		byte = read_byte();
	}

	if ((byte >= FIELD_SIMPLE) && (byte < FIELD_MAX)) {
		uint32_t encoding = field_encodings[byte - FIELD_SIMPLE];
		field->storage = mdil_field_def::field_storage(encoding >> 12);
		field->protection = mdil_field_def::field_protection((encoding >> 8) & 0xf);
		field->element_type = CorElementType(encoding & 0xff);
		log_type_def("\t\tFIELD_SIMPLE type=%02X", field->element_type);
	} else if ((byte >= FIELD_INSTANCE) && (byte <= FIELD_RVA)) {
		field->storage = mdil_field_def::field_storage(byte - FIELD_INSTANCE);
		uint8_t b = m_buffer[m_pos++];
		field->protection = mdil_field_def::field_protection(b >> 5);
		field->element_type = CorElementType(b & 0x1F);
		log_type_def("\t\tFIELD type=%02X stor=%08X prot=%08X", field->element_type, field->storage, field->protection);
	} else if (byte == NATIVE_FIELD) {
		uint32_t f = read_compressed_uint32();
		if (f & 0x40) read_compressed_uint32();
		if ((char) f < 0) read_compressed_uint32();
		if (f & 0x100) read_compressed_uint32();
		if (f & 0x200) read_compressed_uint32();
		log_type_def("\t\tNATIVE_FIELD=%08X", f);
	} else {
		if (!peek) log_type_def("\t\texpecting FIELD_* got %02X", byte);
		fine = false;
	}

	if (fine && (field->element_type == ELEMENT_TYPE_VALUETYPE)) {
		field->boxing_type_token = make_shared<mdToken>(read_compressed_type_token());
		log_type_def("\t\tBox Token=%08X", *field->boxing_type_token);
	}

	if (fine) {
		current_field_token = field->token;
	} else {
		field.reset();
	}
	
	return field;
}

std::shared_ptr<mdil_method_def> mdil_ctl_parser::parse_method_def()
{
	auto method = make_shared<mdil_method_def>();
	bool fine = true;

	method->token = current_method_token;

	uint8_t byte = read_byte();

	if (byte == ADVANCE_METHODDEF) {
		int32_t advance_diff = read_compressed_uint32();
		log_type_def("\t\tADVANCE_METHODDEF=%x", advance_diff);
		method->token = TokenFromRid(advance_diff, mdtMethodDef); // not advancing but reset !?!
		byte = read_byte();
	} else if ((byte >= ADVANCE_METHODDEF_SHORT_MINUS_8) && (byte <= ADVANCE_METHODDEF_SHORT_PLUS_8)) {
		log_type_def("\t\tADVANCE_METHODDEF_SHORT=%x", byte - ADVANCE_METHODDEF_SHORT_0);
		method->token = TokenFromRid(byte - ADVANCE_METHODDEF_SHORT_0, mdtMethodDef); // not advancing but reset !?!
		byte = read_byte();
	}

	method->token++;
	current_method_token = method->token;
	log_type_def("\t\tToken = %08x", method->token);
	
	if (byte == METHOD) {
		method->kind = mdil_method_def::mkNormal;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
		log_type_def("\t\tMETHOD%s%s", (method->attributes & mdVirtual) ? " virtual" : "", (method->attributes & mdNewSlot) ? " new" : "");
	} else if (byte == NORMAL_METHOD) {
		log_type_def("\t\tNORMAL_METHOD");
		method->kind = mdil_method_def::mkNormal;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
	} else if ((byte >= SIMPLE_METHOD) && (byte < (SIMPLE_METHOD + 32))) {
		method->kind = mdil_method_def::mkNormal;
		static const uint32_t method_encodings[] = { 2182u, 129u, 100664774u, 134u, 198u, 131u, 2246u, 268441734u,
													2179u, 150u, 196u, 2198u, 481u, 147u, 145u, 805312646u,
													454u, 486u, 452u, 268441731u, 2502u, 1073748113u, 100666822u, 50528710u,
													41951382u, 707u, 41951379u, 2177u, 2534u, 2529u, 132u, 109053382u };
		uint32_t encoding = method_encodings[byte - SIMPLE_METHOD];
		method->attributes = (CorMethodAttr) (encoding & 0xffff);
		method->impl_attributes = (CorMethodImpl) ((encoding >> 16) & 0xff);
		method->impl_hints = (mdil_method_def::method_impl_hints) (encoding >> 24);
		if ((method->attributes & mdVirtual) && ((method->attributes & mdNewSlot) != mdNewSlot))
			method->overridden_method_token = read_compressed_method_token();
		log_type_def("\t\tSIMPLE_METHOD%s%s", (method->attributes & mdVirtual) ? " virtual" : "", (method->attributes & mdNewSlot) ? " new" : "");
	} else if (byte == PINVOKE_METHOD) {
		log_type_def("\t\tPINVOKE_METHOD");
		method->kind = mdil_method_def::mkPInvoke;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->module_name = read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		read_compressed_uint32(); // ??????
	} else if (byte == NATIVECALLABLE_METHOD) {
		log_type_def("\t\tNATIVECALLABLE_METHOD");
		method->kind = mdil_method_def::mkNativeCallable;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		method->calling_convention = (CorUnmanagedCallingConvention) read_compressed_uint32();
	} else if (byte == RUNTIME_IMPORT_METHOD) {
		log_type_def("\t\tRUNTIME_IMPORT_METHOD");
		method->kind = mdil_method_def::mkRuntimeImport;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
		method->calling_convention = (CorUnmanagedCallingConvention) read_compressed_uint32();
	} else if (byte == RUNTIME_EXPORT_METHOD) {
		log_type_def("\t\tRUNTIME_EXPORT_METHOD");
		method->kind = mdil_method_def::mkRuntimeExport;
		method->attributes = (CorMethodAttr) read_compressed_uint32();
		method->impl_attributes = (CorMethodImpl) read_compressed_uint32();
		method->impl_hints = (mdil_method_def::method_impl_hints) read_compressed_uint32();
		method->entry_point_name = read_compressed_uint32();
	} else if (byte == METHOD_IMPL) {
		method->kind = mdil_method_def::mkImplement;
		method->overridden_method_token = read_compressed_method_token();
		log_type_def("\t\tMETHOD_IMPL=%08X", method->overridden_method_token);
	} else if (byte == STUB_METHOD) {
		uint32_t v1 = read_compressed_uint32();
		uint32_t v2 = 0;
		if (v1 & 0x20) v2 = read_compressed_uint32();
		log_type_def("\t\tSTUB_METHOD=%08X %06X", v1, v2);
	} else {
		fine = false;
		log_type_def("expecting *_METHOD got %02X", byte);
	}

	if (fine) {
		while (peek_byte() == GENERIC_PARAMETER) {
			read_byte();
			uint32_t rid = read_compressed_uint32();
			CorGenericParamAttr attributes = (CorGenericParamAttr) read_compressed_uint32();
			if (!method->generic_params) method->generic_params.resize(0);
			method->generic_params->push_back(make_shared<mdil_generic_param>(mdtGenericParam | rid, attributes));
			log_type_def("\t\tGENERIC_PARAMTER %06X %08X", rid, attributes);
		}
	}

	if (fine) {
		auto rid = RidFromToken(method->token);
		if (rid < m_data.method_map.method_def_mappings.size()) {
			m_data.method_map.method_def_mappings->at(rid)->method_def = method;
		}
	} else method.reset();
	return method;
}

shared_ptr<mdil_type_spec> mdil_ctl_parser::parse_type_spec(const uint32_t rid)
{
	shared_ptr<mdil_type_spec> ret;

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
		ret = make_shared<mdil_type_spec>(mdtTypeSpec | rid, (CorElementType) byte);
		break;
	case ELEMENT_TYPE_PTR:
	case ELEMENT_TYPE_BYREF:
	case ELEMENT_TYPE_TYPEDBYREF:
		ret.reset(new mdil_type_spec_with_child(mdtTypeSpec | rid, (CorElementType) byte, parse_type_spec()));
		break;
	case ELEMENT_TYPE_VALUETYPE:
	case ELEMENT_TYPE_CLASS:
		ret.reset(new mdil_type_spec_with_type(mdtTypeSpec | rid, (CorElementType) byte, read_compressed_type_token()));
		break;
	case ELEMENT_TYPE_VAR:
	case ELEMENT_TYPE_MVAR:
		ret.reset(new mdil_type_spec_with_number(mdtTypeSpec | rid, (CorElementType) byte, read_compressed_uint32()));
		break;
	case ELEMENT_TYPE_ARRAY: {
		auto type = parse_type_spec(0);
		if (type) {
			uint32_t rank = read_compressed_uint32();

			uint32_t bcount = read_compressed_uint32();
			vector<uint32_t> bounds(bcount);
			for (uint32_t i = 0; i < bcount; ++i) bounds[i] = read_compressed_uint32();

			uint32_t lbcount = read_compressed_uint32();
			vector<uint32_t> lbounds(lbcount);
			for (uint32_t i = 0; i < lbcount; ++i) lbounds[i] = read_compressed_uint32();

			ret.reset(new mdil_type_spec_array(mdtTypeSpec | rid, type, rank, bounds, lbounds));
		}
		break;
	}
	case ELEMENT_TYPE_SZARRAY:
		ret.reset(new mdil_type_spec_array(mdtTypeSpec | rid, parse_type_spec(), 1));
		break;
	case ELEMENT_TYPE_GENERICINST: {
		auto type = parse_type_spec();
		if (type != nullptr) {
			uint32_t count = read_compressed_uint32();
			auto args = vector<shared_ptr<mdil_type_spec>>(count);
			for (uint32_t i=0; i < count; ++i) {
				auto arg = parse_type_spec();
				if (arg == nullptr) break;
				args[i] = arg;
			}
			ret.reset(new mdil_type_spec_generic(mdtTypeSpec | rid, type, args));
		}
		break;
	}
	default: break;
	}

	return ret;
}

std::shared_ptr<mdil_method_spec> mdil_ctl_parser::parse_method_spec()
{
	auto ret = make_shared<mdil_method_spec>();
	ret->method_token = read_compressed_method_token();
	uint32_t count = read_compressed_uint32();
	ret->parameters.resize(count);
	for(uint32_t i = 0; i < count; i++) {
		auto type_spec = parse_type_spec();
		ret->parameters->at(i) = type_spec;
	}
	return ret;
}

void mdil_ctl_parser::parse()
{
	// method map
	m_data.method_map.method_def_mappings.resize(m_data.method_map.raw.size());
	for (uint32_t i = 1; i < m_data.method_map.raw->size(); ++i) {
		uint32_t mapping = m_data.method_map.raw->at(i);
		auto method_def_mapping = make_shared<mdil_method_def_mapping>();
		method_def_mapping->is_generic_inst = (mapping & (1 << 31)) != 0 ;
		method_def_mapping->offset = mapping & ~(1 << 31);
		m_data.method_map.method_def_mappings->at(i) = method_def_mapping;
	}

	// generic instances
	if (m_data.generic_instances.raw.size() > 4) {
		auto sig = *(DWORD*) m_data.generic_instances.raw->data();
		if (sig == 'MDGI') {
			size_t pos = 4;
			while (pos < m_data.generic_instances.raw.size()) {
				auto generic_method = make_shared<mdil_generic_method>();
				uint32_t offset = pos;
				generic_method->instance_count = *(uint16_t*) &m_data.generic_instances.raw->at(pos); pos += 2;
				generic_method->flags = *(uint8_t*) &m_data.generic_instances.raw->at(pos); pos += 1;
				generic_method->argument_count = *(uint8_t*) &m_data.generic_instances.raw->at(pos); pos += 1;
				generic_method->instances.resize(generic_method->instance_count);
				for (uint16_t i = 0; i < generic_method->instance_count; ++i) {
					generic_method->instances.at(i) = make_shared<mdil_generic_instance>();
					generic_method->instances.at(i)->code_offset = *(DWORD*) &m_data.generic_instances.raw->at(pos); pos += 4;
					generic_method->instances.at(i)->debug_offset = *(DWORD*) &m_data.generic_instances.raw->at(pos); pos += 4;
				}
				for (uint16_t i = 0; i < generic_method->instance_count; ++i) {
					generic_method->instances[i]->argument_types.resize(generic_method->argument_count);
					for (uint32_t a = 0; a < generic_method->argument_count; ++a) {
						generic_method->instances[i]->argument_types[a] = *(DWORD*) (m_data.generic_instances.raw->data() + pos);
						pos += 4;
					}
				}
				m_data.generic_instances.generic_methods[offset] = generic_method;
			}
		} // else { printf("Signature Incorrect, should be 'MDGI'"); }
	}

	// type map
	current_field_token = mdFieldDefNil;
	current_method_token = mdMethodDefNil;

	m_data.type_map.type_defs.resize(m_data.type_map.raw.size());

	for (unsigned long i = 1; i < m_data.type_map.raw.size(); i++) {
		auto offset = m_data.type_map.raw->at(i);
		if (offset != 0) {
			m_pos = offset;
			log_type_def("TYPEDEF_%08X", offset);
			m_data.type_map.type_defs->at(i) = parse_type_def(i);
		}
	}

	// type specs
	m_data.type_specs.type_specs.resize(m_data.type_specs.raw.size());

	for (unsigned long i = 1; i < m_data.type_specs.raw.size(); i++) {
		auto offset = m_data.type_specs.raw->at(i);;
		if (offset != 0) {
			m_pos = offset;
			m_data.type_specs.type_specs->at(i) = parse_type_spec(i);
		}
	}

	// method specs
	m_data.method_specs.method_specs.resize(m_data.method_specs.raw.size());

	for (unsigned long i = 1; i < m_data.method_specs.raw.size(); i++) {
		auto offset = m_data.method_specs.raw->at(i);;
		if (offset != 0) {
			m_pos = offset;
			m_data.method_specs.method_specs->at(i) = parse_method_spec();
		}
	}
}
