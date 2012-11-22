#include "stdafx.h"
#include "mdil_data.h"
#include "mdil_parser.h"
#include "mdil_decoder.h"
#include "console_dumper.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if ((argc <= 1)) {
		printf_s("Usage: mdildump <assemblyfile>\n");
		return 0;
	}

	mdil_data data;
	std::string error = mdil_parser().parse(argv[1], data);

	std::shared_ptr<console_dumper> dumper = std::make_shared<console_dumper>(data);

	if (data.header) dumper->dump_mdil_header("MDIL Header");
	if (data.header_2) dumper->dump_mdil_header_2("MDIL Header 2");
	if (data.platform_data) dumper->dump_bytes(data.platform_data, "Platform Data");
	if (data.well_known_types)	dumper->dump_ulongs(data.well_known_types, "Well Known Types Table");
	if (data.type_map) dumper->dump_ulongs(data.type_map, "Type Map", "Offsets in Types section");
	if (data.method_map) dumper->dump_method_map("Method Map", "Offsets in Generic Instances (GI) or Code (CD) section");
	if (data.generic_instances)	dumper->dump_generic_instances("Generic Instances");
	if (data.ext_module_refs) dumper->dump_ext_module_refs("External Module References", "Offsets in Name Pool section");
	if (data.ext_type_refs) dumper->dump_ext_type_refs("External Type References", "Index in External Module References section, and an index");
	if (data.ext_member_refs) dumper->dump_ext_member_refs("External Member References", "Index in Type Spec (TS) or External Type References (TR) section, and an index");
	if (data.type_specs) dumper->dump_ulongs(data.type_specs, "Type Specs", "Offsets in Compact Type Layout section");
	if (data.method_specs) dumper->dump_ulongs(data.method_specs, "Method Specs", "Offsets in Compact Type Layout section");
	if (data.section_10) dumper->dump_ulongs(data.section_10, "Section 10");
	if (data.name_pool) dumper->dump_chars(data.name_pool, "Name Pool");
	if (data.types) dumper->dump_types("Types", "Compact Type Layout");
	if (data.user_string_pool) dumper->dump_chars(data.user_string_pool, "User String Pool");
	if (data.code_1.data) {
		for (auto m = begin(data.code_1.methods); m != end(data.code_1.methods); ++m) {
			m->routine.swap(mdil_decoder(data.code_1.data->data() + m->offset + m->routine_offset, m->routine_size).decode());
		}
		dumper->dump_code(data.code_1, "Code 1");
	}
	if (data.code_2.data) {
		for (auto m = begin(data.code_2.methods); m != end(data.code_2.methods); ++m) {
			m->routine.swap(mdil_decoder(data.code_2.data->data() + m->offset + m->routine_offset, m->routine_size).decode());
		}
		dumper->dump_code(data.code_2, "Code 2");
	}
	if (data.section_16) dumper->dump_bytes(data.section_16, "Section 16");
	if (data.section_17) dumper->dump_bytes(data.section_17, "Section 17");
	if (data.debug_map) dumper->dump_ulongs(data.debug_map, "Debug Map", "Offsets in Debug Info section");
	if (data.debug_info_1) dumper->dump_debug_info(data.debug_info_1, true, "Debug Info 1");
	if (data.debug_info_2) dumper->dump_debug_info(data.debug_info_2, false, "Debug Info 2");
	if (data.section_21) dumper->dump_bytes(data.section_21, "Section 21");
	if (data.section_22) dumper->dump_bytes(data.section_22, "Section 21");
	
	if (!error.empty()) printf_s("Parsing Error: %s\n", error.c_str());

	return 0;
}

