#pragma once

#include "mdil_data.h"
#include "cli_metadata_reader.h"

class console_dumper
{
	const mdil_data& m_data;
	std::shared_ptr<cli_metadata_reader> m_metadata;

	void dump_bytes_int (const shared_vector<unsigned char>& data, size_t offset, size_t count);
	std::unordered_map<unsigned long, std::string> ext_modules;
	std::wstring format_generic_params(const shared_vector<std::shared_ptr<const mdil_generic_param>>& generic_params);
	std::wstring format_type_name(mdToken token, bool qualified = false, bool omit_generic_params = false); // type def/ref/spec
	std::wstring format_method_name(mdToken token, bool qualified = false, bool omit_generic_params = false);
	void dump_method_def(const mdil_method_def* method_def, bool is_interface = false);
	void dump_type_def(mdil_type_def* type_def);
	std::wstring format_type_spec(mdil_type_spec* type_spec, bool prefix = false);

	struct code_mapping {
		mdMethodDef method_token;
		std::vector<uint32_t> argument_types;
		uint16_t generic_inst;
		bool is_generic_inst;
	};
	std::unordered_map<uint32_t, code_mapping> m_code_map; // code offsets to method token
	void build_code_map();
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

