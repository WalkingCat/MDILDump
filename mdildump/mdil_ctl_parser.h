#pragma once

#include "mdil_data.h"

class mdil_ctl_parser
{
	mdil_data& m_data;

	const unsigned char* m_buffer;
	const unsigned long m_length;
	unsigned long m_pos;
	bool m_error;

	uint8_t read_byte();
	uint32_t read_uint32_le();
	int32_t read_compressed_int32();
	uint32_t read_compressed_uint32();
	mdToken read_compressed_type_token();
	mdToken read_compressed_method_token();

	bool dump_known_unknowns();

	mdTypeDef current_type_token;
	mdFieldDef current_field_token;
	mdMethodDef current_method_token;

	bool dump_type_def_members(uint32_t field_Count, uint32_t method_count, uint32_t interface_count);
	bool dump_type_def();

	mdil_type_spec* parse_type_spec();
public:
	mdil_ctl_parser(mdil_data& data) : m_data(data), m_buffer(m_data.types ? m_data.types->data() : nullptr), m_length(m_data.types.size()), m_pos(0), m_error(false) {}
	void dump_type_map(const char* title = nullptr, const char* description = nullptr);
	void parse();
};

