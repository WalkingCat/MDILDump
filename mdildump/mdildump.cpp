#include "stdafx.h"
#include "mdil_data.h"
#include "mdil_parser.h"
#include "mdil_decoder.h"
#include "mdil_ctl_parser.h"
#include "console_dumper.h"

using namespace std;

enum dump_options {
	dumpNone			= 0x0,
	dumpHeader			= 0x1,
	dumpHeader2			= 0x2,
	dumpPlatformData	= 0x4,
	dumpWellKnownTypes	= 0x8,
	dumpTypeMap			= 0x10,
	dumpMethodMap		= 0x20,
	dumpGenericInstances= 0x40,
	dumpExtModuleRefs	= 0x80,
	dumpExtTypeRefs		= 0x100,
	dumpExtMemberRefs	= 0x200,
	dumpTypeSpecs		= 0x400,
	dumpMethodSpecs		= 0x800,
	dumpSection10		= 0x1000,
	dumpNamePool		= 0x2000,
	dumpTypes			= 0x4000,
	dumpUserStringPool	= 0x8000,
	dumpCode1			= 0x10000,
	dumpCode2			= 0x20000,
	dumpSection16		= 0x40000,
	dumpSection17		= 0x80000,
	dumpDebugMap		= 0x100000,
	dumpDebugInfo1		= 0x200000,
	dumpDebugInfo2		= 0x400000,
	dumpSection21		= 0x800000,
	dumpSection22		= 0x1000000,
	dumpCodeDasm		= 0x10000000,
	dumpHelp			= 0x80000000,
};

const struct { const char* arg; const char* arg_alt; const char* description; const int options; } cmd_options[] = {
	{ "?",		"help",				"show this help",					dumpHelp },
	{ "all",	nullptr,			"dump all the data",				0xffffffff & (~dumpHelp) },
	{ "h",		"header",			"dump MDIL header",					dumpHeader },
	{ "h2",		"header2",			"dump MDIL header 2",				dumpHeader2 },
	{ nullptr,	"headers",			"dump MDIL headers",				dumpHeader | dumpHeader2 },
	{ "pd",		"platformdata",		"dump platform data",				dumpPlatformData },
	{ "wkt",	"wellknowntypes",	"dump well known types",			dumpWellKnownTypes },
	{ "tm",		"typemap",			"dump type map",					dumpTypeMap },
	{ "mm",		"methodmap",		"dump method map",					dumpMethodMap },
	{ "gi",		"genericinstances",	"dump generic instances",			dumpGenericInstances },
	{ "emor",	"extmodulerefs",	"dump external module references",	dumpExtModuleRefs },
	{ "etr",	"exttyperefs",		"dump external type references",	dumpExtTypeRefs },
	{ "emr",	"extmemberrefs",	"dump external member references",	dumpExtMemberRefs },
	{ nullptr,	"extrefs",			"dump external references",			dumpExtModuleRefs | dumpExtTypeRefs | dumpExtMemberRefs },
	{ "ts",		"typespecs",		"dump type specs",					dumpTypeSpecs },
	{ "ms",		"methodspecs",		"dump method specs",				dumpMethodSpecs },
	{ "s10",	"section10",		"dump section 10",					dumpSection10 },
	{ "np",		"namepool",			"dump name pool",					dumpNamePool },
	{ "t",		"types",			"dump types",						dumpTypes },
	{ "usp",	"userstringpool",	"dump user string pool",			dumpUserStringPool },
	{ "c1",		"code1",			"dump code 1",						dumpCode1 },
	{ "c2",		"code2",			"dump code 2",						dumpCode2 },
	{ "c",		"code",				"dump code 1 & 2",					dumpCode1 | dumpCode2 },
	{ "s16",	"section16",		"dump section 16",					dumpSection16 },
	{ "s17",	"section17",		"dump section 17",					dumpSection17 },
	{ "dm",		"debugmap",			"dump debug map",					dumpDebugMap },
	{ "di1",	"debuginfo1",		"dump debug info 1",				dumpDebugInfo1 },
	{ "di2",	"debuginfo2",		"dump debug info 2",				dumpDebugInfo2 },
	{ nullptr,	"debug",			"dump debug map & info 1 & 2",		dumpDebugMap | dumpDebugInfo1 | dumpDebugInfo2 },
	{ "s21",	"section21",		"dump section 21",					dumpSection21 },
	{ "s22",	"section22",		"dump section 22",					dumpSection22 },
	{ "cd",		"codedasm",			"dump disassembled code",			dumpCodeDasm },
};

void print_usage() {
	printf_s("\tUsage: mdildump <assemblyfile> [options]\n\n");
	for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
		if (o->arg != nullptr) printf_s("\t-%s", o->arg); else printf_s("\t");

		int len = 0;
		if (o->arg_alt != nullptr){
			len = strlen(o->arg_alt);
			printf_s("\t--%s", o->arg_alt);
		} else printf_s("\t");

		if (len < 6) printf_s("\t");
		if (len < 14) printf_s("\t");
		printf_s("\t: %s\n", o->description);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int options = dumpNone;
	const char* assembly = nullptr;
	const char* err_arg = nullptr;

	printf_s("\nMDILDump v0.2 https://github.com/WalkingCat/mdildump\n\n");

	for(int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		if ((arg[0] == '-') || ((arg[0] == '/'))) {
			bool valid = false;
			if ((arg[0] == '-') && (arg[1] == '-')) {
				for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
					if ((o->arg_alt != nullptr) &&(_stricmp(arg + 2, o->arg_alt) == 0)) { valid = true; options |= o->options; }
				}
			} else {
				for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
					if ((o->arg != nullptr) && (_stricmp(arg + 1, o->arg) == 0)) { valid = true; options |= o->options; }
				}
			}
			if ((!valid) && (err_arg == nullptr)) err_arg = arg;
		} else { if (assembly == nullptr) assembly = arg; else err_arg = arg; }
	}

	if ((assembly == nullptr) || (err_arg != nullptr) || (options & dumpHelp)) {
		if (err_arg != nullptr) printf_s("\tUnknown option: %s\n\n", err_arg);
		print_usage();
		return 0;
	}

	printf_s("Dumping assembly file: %s\n\n", assembly);

	if ((options & dumpCodeDasm) && (!(options & (dumpCode1 | dumpCode2)))) options |= dumpCode1 | dumpCode2;

	if (options == dumpNone) options = dumpHeader | dumpHeader2 | dumpTypeMap | dumpMethodMap | dumpGenericInstances |
		dumpExtModuleRefs | dumpExtTypeRefs | dumpExtMemberRefs | dumpTypeSpecs | dumpCode1 | dumpCode2 | dumpCodeDasm;

	mdil_data data;
	std::string error = mdil_parser().parse(assembly, data);

	if ((!data.header) && (!error.empty())) { printf_s("Parsing Error: %s\n", error.c_str()); return 0; }

	mdil_architecture arch = archX86;
	uint32_t arch_flags = data.header->flags & mdil_header::TargetArch_Mask;

	if (arch_flags == mdil_header::TargetArch_X86) arch = archX86;
	else if (arch_flags == mdil_header::TargetArch_AMD64) arch = archX64;
	else if (arch_flags == (mdil_header::TargetArch_AMD64 | mdil_header::TargetArch_IA64)) arch = archARM;

	mdil_ctl_parser(data).parse();

	auto dumper = std::make_shared<console_dumper>(data);

	if (options & dumpHeader)			dumper->dump_mdil_header("MDIL Header");
	if (options & dumpHeader2)			dumper->dump_mdil_header_2("MDIL Header 2");
	if (options & dumpPlatformData)		dumper->dump_bytes(data.platform_data, "Platform Data");
	if (options & dumpWellKnownTypes)	dumper->dump_ulongs(data.well_known_types, "Well Known Types Table");
	if (options & dumpTypeMap)			dumper->dump_type_map("Type Map", "Type Def Offsets in Types section");
	if (options & dumpMethodMap)		dumper->dump_method_map("Method Map", "Offsets in Generic Instances or Code section");
	if (options & dumpGenericInstances)	dumper->dump_generic_instances("Generic Instances");
	if (options & dumpExtModuleRefs)	dumper->dump_ext_module_refs("External Module References", "Offsets in Name Pool section");
	if (options & dumpExtTypeRefs)		dumper->dump_ext_type_refs("External Type References", "Index in External Module References section, and an index");
	if (options & dumpExtMemberRefs)	dumper->dump_ext_member_refs("External Member References", "Index in Type Spec or External Type References section, and an index");
	if (options & dumpTypeSpecs)		dumper->dump_type_specs("Type Specs", "Offsets in Types section");
	if (options & dumpMethodSpecs)		dumper->dump_method_specs("Method Specs", "Offsets in Types section");
	if (options & dumpSection10)		dumper->dump_ulongs(data.section_10, "Section 10");
	if (options & dumpNamePool)			dumper->dump_chars(data.name_pool, "Name Pool");
	if (options & dumpTypes)			dumper->dump_types("Types", "Compact Type Layout");
	if (options & dumpUserStringPool)	dumper->dump_chars(data.user_string_pool, "User String Pool");
	if (options & dumpCode1) {
		if (options & dumpCodeDasm)
			for (auto m = begin(data.code_1.methods); m != end(data.code_1.methods); ++m) {
				m->routine.swap(mdil_decoder(data.code_1.raw->data() + m->offset + m->routine_offset, m->routine_size, arch).decode());
			}
		dumper->dump_code(data.code_1, "Code 1");
	}
	if (options & dumpCode2) {
		if (options & dumpCodeDasm)
			for (auto m = begin(data.code_2.methods); m != end(data.code_2.methods); ++m) {
				m->routine.swap(mdil_decoder(data.code_2.raw->data() + m->offset + m->routine_offset, m->routine_size, arch).decode());
			}
		dumper->dump_code(data.code_2, "Code 2");
	}
	if (options & dumpSection16)		dumper->dump_bytes(data.section_16, "Section 16");
	if (options & dumpSection17)		dumper->dump_bytes(data.section_17, "Section 17");
	if (options & dumpDebugMap)			dumper->dump_ulongs(data.debug_map, "Debug Map", "Offsets in Debug Info section");
	if (options & dumpDebugInfo1)		dumper->dump_debug_info(data.debug_info_1, true, "Debug Info 1");
	if (options & dumpDebugInfo2)		dumper->dump_debug_info(data.debug_info_2, false, "Debug Info 2");
	if (options & dumpSection21)		dumper->dump_bytes(data.section_21, "Section 21");
	if (options & dumpSection22)		dumper->dump_bytes(data.section_22, "Section 21");
	
	if (!error.empty()) printf_s("Parsing Error: %s\n", error.c_str());

	return 0;
}

