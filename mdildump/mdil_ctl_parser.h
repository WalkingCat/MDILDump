#pragma once

#include "mdil_data.h"

class mdil_ctl_parser
{
	const mdil_data& m_data;

	const unsigned char* m_buffer;
	const unsigned long m_length;
	unsigned long m_pos;
	bool m_error;

	uint8_t read_byte();
	uint32_t read_uint32_le();
	int32_t read_compressed_int32();
	uint32_t read_compressed_uint32();
	uint32_t read_compressed_type_token();
	uint32_t read_compressed_method_token();

	bool dump_known_unknowns();
	bool dump_type_def_members(uint32_t fieldCount, uint32_t methodCount, uint32_t interfaceCount);
	bool dump_type_def();
	bool dump_type_spec(uint32_t level = 0);
public:
	mdil_ctl_parser(const mdil_data& data) : m_data(data), m_buffer(m_data.types ? m_data.types->data() : nullptr), m_length(m_data.types.size()), m_pos(0), m_error(false) {}
	void dump_type_map(const char* title = nullptr, const char* description = nullptr);
	void dump_type_specs(const char* title = nullptr, const char* description = nullptr);
};

