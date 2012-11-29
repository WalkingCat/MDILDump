#pragma once

#include "mdil_data.h"

class mdil_ctl_parser
{
	mdil_data& m_data;

	const unsigned char* m_buffer;
	const unsigned long m_length;
	unsigned long m_pos;
	bool m_error;

	uint8_t peek_byte();
	uint8_t read_byte();
	uint32_t read_uint32_le();
	int32_t read_compressed_int32();
	uint32_t read_compressed_uint32();
	mdToken read_compressed_type_token();
	mdToken read_compressed_method_token();

	mdFieldDef current_field_token;
	mdMethodDef current_method_token;

	std::shared_ptr<mdil_field_def> parse_field_def(bool peek = false);
	std::shared_ptr<mdil_method_def> parse_method_def();
	std::shared_ptr<mdil_type_def> parse_type_def(const uint32_t index);
	mdil_type_spec* parse_type_spec();

	void log_type_def( const char* format, ... );
public:
	mdil_ctl_parser(mdil_data& data) : m_data(data), m_buffer(m_data.types ? m_data.types->data() : nullptr), m_length(m_data.types.size()), m_pos(0), m_error(false) {}
	void parse();
};

