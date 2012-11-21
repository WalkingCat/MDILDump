#pragma once

#include "mdil_data.h"

class console_dumper
{
	const mdil_data& m_data;
	void dump_bytes_int (const shared_vector<unsigned char>& data, size_t offset, size_t count);
public:
	console_dumper(const mdil_data& data) : m_data(data) {}
	void dump_mdil_header(const char* title = nullptr, const char* description = nullptr);
	void dump_mdil_header_2(const char* title = nullptr, const char* description = nullptr);
	void dump_bytes (const shared_vector<unsigned char>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_chars (const shared_vector<char>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_ulongs (const shared_vector<unsigned long>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_method_map (const char* title = nullptr, const char* description = nullptr);
	void dump_generic_instances (const char* title = nullptr, const char* description = nullptr);
	void dump_ext_module_refs(const char* title = nullptr, const char* description = nullptr);
	void dump_ext_type_refs(const char* title = nullptr, const char* description = nullptr);
	void dump_ext_member_refs(const char* title = nullptr, const char* description = nullptr);
	void dump_types(const char* title = nullptr, const char* description = nullptr);
	void dump_code(const mdil_code& code, const char* title = nullptr, const char* description = nullptr);
	void dump_debug_info (const shared_vector<unsigned char>& data, bool hasSig, const char* title = nullptr, const char* description = nullptr);
	void dump_instructions(const std::vector<std::shared_ptr<mdil_instruction>>& code, unsigned char* data, unsigned long count);
};

