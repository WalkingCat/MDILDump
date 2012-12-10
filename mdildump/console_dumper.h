#pragma once

#include "mdil_data.h"
#include "cli_metadata_reader.h"

class console_dumper
{
	const mdil_data& m_data;
	std::shared_ptr<cli_metadata_reader> m_metadata;

	void dump_bytes_int (const shared_vector<unsigned char>& data, size_t offset, size_t count);
	std::unordered_map<unsigned long, std::string> ext_modules;
	void dump_type_def(mdil_type_def* type_def);
	void dump_type_spec(mdil_type_spec* type_spec);
public:
	console_dumper(const mdil_data& data, const std::shared_ptr<cli_metadata_reader>& metadata) : m_data(data), m_metadata(metadata) {}
	void dump_mdil_header(const char* title = nullptr, const char* description = nullptr);
	void dump_mdil_header_2(const char* title = nullptr, const char* description = nullptr);
	void dump_bytes (const shared_vector<unsigned char>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_chars (const shared_vector<char>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_ulongs (const shared_vector<unsigned long>& data, const char* title = nullptr, const char* description = nullptr);
	void dump_type_map (const char* title = nullptr, const char* description = nullptr);
	void dump_method_map (const char* title = nullptr, const char* description = nullptr);
	void dump_generic_instances (const char* title = nullptr, const char* description = nullptr);
	void dump_ext_module_refs(const char* title = nullptr, const char* description = nullptr);
	std::string format_ext_module_ref(unsigned long id);
	void dump_ext_type_refs(const char* title = nullptr, const char* description = nullptr);
	std::string format_ext_type_ref(unsigned long id);
	void dump_ext_member_refs(const char* title = nullptr, const char* description = nullptr);
	void dump_type_specs(const char* title = nullptr, const char* description = nullptr);
	void dump_method_specs(const char* title = nullptr, const char* description = nullptr);
	void dump_types(const char* title = nullptr, const char* description = nullptr);
	void dump_code(const mdil_code& code, const char* title = nullptr, const char* description = nullptr);
	void dump_debug_info (const shared_vector<unsigned char>& data, bool hasSig, const char* title = nullptr, const char* description = nullptr);
	void dump_instructions(const std::vector<std::shared_ptr<mdil_instruction>>& code, unsigned char* data, unsigned long count);
};

