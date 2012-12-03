#include "stdafx.h"
#include "mdil_decoder.h"

using namespace std;


mdil_decoder::mdil_decoder(const unsigned char* buffer, const unsigned long length )
	: m_buffer(buffer), m_length(length), m_pos(0), m_error(false) {}

string mdil_reg (unsigned char reg) {
	switch (reg & 0xF) {
	case 0x0 : return "EAX";
	case 0x1 : return "ECX";
	case 0x2 : return "EDX";
	case 0x3 : return "EBX";
	case 0x4 : return "???";
	case 0x5 : return "EBP";
	case 0x6 : return "ESI";
	case 0x7 : return "EDI";
	case 0x8 : return "R8";
	case 0x9 : return "R9";
	case 0xA : return "R10";
	case 0xB : return "R11";
	case 0xC : return "R12";
	case 0xD : return "R13";
	case 0xE : return "R14";
	case 0xF : return "R15";
	default  : return "!!!";
	}
}

std::string mdil_decoder::format_immediate() {
	std::stringstream ret;
	ret.flags(ret.uppercase | ret.hex);

	unsigned char first_byte = m_buffer[m_pos++];
	switch (first_byte)
	{
	case 0xbb: { // SignedWord
		short value = (short) (m_buffer[m_pos] + (m_buffer[m_pos+1] << 8));
		if (value >= 0) ret << setw(4) << setfill('0') << value;
		else ret << "-" << setw(4) << setfill('0') << -value;
		m_pos += 2;
		break;
	}
	case 0xdd: ret << format_dword(read_dword_le()); break;
	case 0xbd: 
		ret << "ArrayElemSize[" << format_type_token() << "]";
		ret << "*" << format_immediate();
		ret << "+" << format_immediate();
		break;
	case 0xdb:
		ret << "ArgumentSize[" << format_type_token() << "]";
		ret << "*" << format_immediate();
		ret << "+" << format_immediate();
		break;
	default: { // SignedByte
		char value = (char) first_byte;
		if (value >= 0) ret << setw(2) << setfill('0') << (int) value;
		else ret << "-" << setw(2) << setfill('0') << (int) -value;
		break;
	}
	}

	return ret.str();
}

string mdil_decoder::format_const_data (unsigned long length) {
	if ((m_pos + length) > m_length) {
		m_error = true;
		return "(INVALID: TOO LONG)";
	}
	stringstream ss;
	ss.flags(ss.hex | ss.uppercase);
	for (unsigned long i = 0; i < length; ++i) {
		ss << setw(2) << setfill('0') << (int) m_buffer[m_pos++] << " ";
	}
	return ss.str();
}

string mdil_decoder::read_native_quote (unsigned long length) {
	if ((m_pos + length) > m_length) {
		m_error = true;
		return "(INVALID: TOO LONG)";
	}

	stringstream ss;
	ss.flags(ss.hex | ss.uppercase);
	for (unsigned long i = 0; i < length; ++i) {
		ss << setw(2) << setfill('0') << (int) m_buffer[m_pos++] << " ";
	}
	return ss.str();
}

std::vector<shared_ptr<mdil_instruction>> mdil_decoder::decode()
{
	std::vector<shared_ptr<mdil_instruction>> routine;
	stringstream ss;
	ss.flags(ss.uppercase | ss.hex);

	m_pos = 0;
	m_error = false;

	while (m_pos < m_length) {
		auto i = make_shared<mdil_instruction>();
		i->offset = m_pos;

		switch (m_buffer[m_pos++]) {
		case 0x00: i->set("LIT_MACHINE_INST_0", read_native_quote(0)); break;
		case 0x01: i->set("LIT_MACHINE_INST_1", read_native_quote(1)); break;
		case 0x02: i->set("LIT_MACHINE_INST_2", read_native_quote(2)); break;
		case 0x03: i->set("LIT_MACHINE_INST_3", read_native_quote(3)); break;
		case 0x04: i->set("LIT_MACHINE_INST_4", read_native_quote(4)); break;
		case 0x05: i->set("LIT_MACHINE_INST_5", read_native_quote(5)); break;
		case 0x06: i->set("LIT_MACHINE_INST_6", read_native_quote(6)); break;
		case 0x07: i->set("LIT_MACHINE_INST_7", read_native_quote(7)); break;
		case 0x08: i->set("LIT_MACHINE_INST_8", read_native_quote(8)); break;
		case 0x09: i->set("LIT_MACHINE_INST_9", read_native_quote(9)); break;
		case 0x0a: i->set("LIT_MACHINE_INST_10", read_native_quote(10)); break;
		case 0x0b: i->set("LIT_MACHINE_INST_11", read_native_quote(11)); break;
		case 0x0c: i->set("LIT_MACHINE_INST_12", read_native_quote(12)); break;
		case 0x0d: i->set("LIT_MACHINE_INST_13", read_native_quote(13)); break;
		case 0x0e: i->set("LIT_MACHINE_INST_14", read_native_quote(14)); break;
		case 0x0f: i->set("LIT_MACHINE_INST_15", read_native_quote(15)); break;
		case 0x10: i->set("LIT_MACHINE_INST_B", read_native_quote(read_byte())); break;
		case 0x11: i->set("LIT_MACHINE_INST_W", read_native_quote(read_word_le())); break;
		case 0x12: i->set("LIT_MACHINE_INST_L", read_native_quote(read_dword_le())); break;
		case 0x13: i->set("LOAD", format_address()); break;
		case 0x14: i->set("STORE", format_address()); break;
		case 0x15: i->set("STORE_REF", format_address()); break;
		case 0x16: i->set("LOAD_SX", format_address()); break;
		case 0x17: i->set("LOAD_ZX", format_address()); break;
		case 0x18: i->set("LOAD_X", format_address()); break;
		case 0x19: i->set("LOAD_ADDR", format_address()); break;
		case 0x1a: i->set("ADD", format_address()); break;
		case 0x1b: i->set("ADC", format_address()); break;
		case 0x1c: i->set("AND", format_address()); break;
		case 0x1d: i->set("CMP", format_address()); break;
		case 0x1e: i->set("OR", format_address()); break;
		case 0x1f: i->set("SUB", format_address()); break;
		case 0x20: i->set("SBB", format_address()); break;
		case 0x21: i->set("XOR", format_address()); break;
		case 0x22: i->set("ADD_TO", format_address()); break;
		case 0x23: i->set("ADC_TO", format_address()); break;
		case 0x24: i->set("AND_TO", format_address()); break;
		case 0x25: i->set("CMP_TO", format_address()); break;
		case 0x26: i->set("OR_TO", format_address()); break;
		case 0x27: i->set("SUB_TO", format_address()); break;
		case 0x28: i->set("SBB_TO", format_address()); break;
		case 0x29: i->set("XOR_TO", format_address()); break;
		case 0x2a: i->set("TEST", format_address()); break;
		case 0x2b: i->set("MUL_DIV_EAX", format_address()); break;
		case 0x2c: i->set("IMUL", format_address()); break;
		case 0x2d: i->set("IMUL_IMM", format_address()); break;
		case 0x2e:
			i->set("ELEM_SCALE");
			i->operands = mdil_reg(read_byte());
			i->operands += ", " + format_address();
			i->operands += ", " + format_type_token();
			break;
		case 0x2f: i->set("INC_DEC_PUSH", format_address()); break;
		case 0x30: i->set("STORE_IMM", format_address([this](uint8_t op_reg, uint8_t base_reg, uint8_t flags){
						string ret = format_address_base(base_reg, flags);
						if (op_reg == 0) ret += ", " + format_immediate(); // sure ?
						else ret += ", " + format_byte(op_reg); // whats this ? some kind of flags ?
						return ret;
				   }));break;
		case 0x31: i->set("UNKNOWN_31", format_address_no_reg()); i->operands += ", " + format_immediate(); break;
		case 0x32: i->set("TEST_IMM", format_address([this](uint8_t op_reg, uint8_t base_reg, uint8_t flags){
			string ret = format_address_base(base_reg, flags);
			if (op_reg == 0) ret += ", " + format_immediate(); // sure ?
			else ret += ", " + format_byte(op_reg); // whats this ? some kind of flags ?
			return ret;
				   }));break;
		case 0x33: i->set("SHIFT_1", format_address_no_reg()); break;
		case 0x34: i->set("SHIFT_IMM", format_address_no_reg()); i->operands += ", " + format_immediate(); break;
		case 0x35: i->set("SHIFT_CL", format_address_no_reg());/* i->operands += ", " + format_byte();*/ break;
		case 0x36: i->set("OP_XMM", format_address_no_reg()); /*i->operands += ", " + format_byte();*/ break;
		case 0x37: i->set("LD_ST_FPU", format_address_no_reg()); break;
		case 0x38: i->set("OP_FPU", format_address_no_reg()); break;
		case 0x39: i->set("ILOAD_FPU", format_address_no_reg()); break;
		case 0x3a: i->set("ISTORE_FPU", format_address_no_reg()); i->operands += ", " + format_byte(); break; // ????
		case 0x3b: i->set("SET_CC"); i->operands = format_address(); i->operands += ", " + format_byte(); break;
		case 0x3c: i->set("XADD", format_address()); break;
		case 0x3d: i->set("XCHG", format_address()); break;
		case 0x3e: i->set("CMPXCHG", format_address()); break;
		case 0x3f: i->set("COPY_STRUCT");
			i->operands += format_address([this](uint8_t op_reg, uint8_t base_reg, uint8_t flags){
				string ret = format_address_base(base_reg, flags);
				if (op_reg != 0) ret += ", " + format_byte(op_reg);
				return ret;
			});
			i->operands += ", " + format_address();
			break;
		case 0x40: i->set("PUSH_STRUCT", format_address()); break;
		case 0x41: i->set("UNKNOWN_41", format_address()); break;
		case 0x42: i->set("UNKNOWN_42"); break;
		case 0x43: i->set("UNKNOWN_43"); break;
		case 0x44: i->set("UNKNOWN_44"); break;
		case 0x45: i->set("GET_STATIC_BASE", format_type_token()); break;
		case 0x46: i->set("GET_STATIC_BASE_GC", format_type_token()); break;
		case 0x47: i->set("CALL", format_method_token()); break;
		case 0x48: i->set("CALL_VIRT", format_method_token()); break;
		case 0x49: i->set("CALL_INDIRECT", format_address([this](uint8_t op_reg, uint8_t base_reg, uint8_t flags){
						string ret = format_address_base(base_reg, flags);
						if (op_reg == 1) { // CIF_METHOD_TOKEN
							ret += "." + format_method_token();
						}
						return ret;
				   }));
			break;
		case 0x4a: i->set("TAIL_CALL", format_method_token()); break;
		case 0x4b: i->set("HELPER_CALL", format_immediate()); break;
		case 0x4c: i->set("CONSTRAINT", format_type_token()); break;
		case 0x4d: i->set("CALL_DEF", format_dword(0x06000000 + read_word_le())); break;
		case 0x4e: i->set("CALL_REF", format_dword(0x0A000000 + read_word_le())); break;
		case 0x4f: i->set("CALL_VIRT_DEF", format_dword(0x06000000 + read_word_le())); break;
		case 0x50: i->set("CALL_VIRT_REF", format_dword(0x0A000000 + read_word_le())); break;
		case 0x51: i->set("JUMP", format_jump_distance()); break;
		case 0x52: i->set("JUMP_LONG", format_jump_distance(true)); break;
		case 0x53: i->set("JUMP_O", format_jump_distance()); break;
		case 0x54: i->set("JUMP_NO", format_jump_distance()); break;
		case 0x55: i->set("JUMP_ULT", format_jump_distance()); break;
		case 0x56: i->set("JUMP_UGE", format_jump_distance()); break;
		case 0x57: i->set("JUMP_EQ", format_jump_distance()); break;
		case 0x58: i->set("JUMP_NE", format_jump_distance()); break;
		case 0x59: i->set("JUMP_ULE", format_jump_distance()); break;
		case 0x5a: i->set("JUMP_UGT", format_jump_distance()); break;
		case 0x5b: i->set("JUMP_S", format_jump_distance()); break;
		case 0x5c: i->set("JUMP_NS", format_jump_distance()); break;
		case 0x5d: i->set("JUMP_PE", format_jump_distance()); break;
		case 0x5e: i->set("JUMP_PO", format_jump_distance()); break;
		case 0x5f: i->set("JUMP_LT", format_jump_distance()); break;
		case 0x60: i->set("JUMP_GE", format_jump_distance()); break;
		case 0x61: i->set("JUMP_LE", format_jump_distance()); break;
		case 0x62: i->set("JUMP_GT", format_jump_distance()); break;
		case 0x63: i->set("THROW"); break;
		case 0x64: i->set("RETHROW"); break;
		case 0x65: i->set("BEGIN_FINALLY", format_dword(read_dword_le())); break;
		case 0x66: i->set("END_FINALLY"); break;
		case 0x67: i->set("UNKNOWN_67"); break;
		case 0x69: i->set("ISINST", format_type_token()); break;
		case 0x6a: i->set("CASTCLASS", format_type_token()); break;
		case 0x6b: i->set("BOX", format_type_token()); break;
		case 0x6c: i->set("UNBOX", format_type_token()); break;
		case 0x6d: i->set("ALLOC_OBJECT", format_type_token()); break;
		case 0x6e: i->set("ALLOC_ARRAY", format_type_token()); break;
		case 0x6f: i->set("REF_BIRTH_EAX"); break;
		case 0x70: i->set("REF_BIRTH_ECX"); break;
		case 0x71: i->set("REF_BIRTH_EDX"); break;
		case 0x72: i->set("REF_BIRTH_EBX"); break;
		case 0x73: i->set("REF_BIRTH_REG", mdil_reg((read_byte() >> 3) & 0x1f)); break;
		case 0x74: i->set("REF_BIRTH_EBP"); break;
		case 0x75: i->set("REF_BIRTH_ESI"); break;
		case 0x76: i->set("REF_BIRTH_EDI"); break;
		case 0x77: i->set("REF_DEATH_EAX"); break;
		case 0x78: i->set("REF_DEATH_ECX"); break;
		case 0x79: i->set("REF_DEATH_EDX"); break;
		case 0x7a: i->set("REF_DEATH_EBX"); break;
		case 0x7b: i->set("REF_DEATH_REG", mdil_reg((read_byte() >> 3) & 0x1f)); break;
		case 0x7c: i->set("REF_DEATH_EBP"); break;
		case 0x7d: i->set("REF_DEATH_ESI"); break;
		case 0x7e: i->set("REF_DEATH_EDI"); break;
		case 0x7f: i->set("REF_BIRTH_EBP_V", format_immediate()); break;
		case 0x80: i->set("REF_DEATH_EBP_V", format_immediate()); break;
		case 0x81: i->set("REF_BIRTH_ESP_V", format_immediate()); break;
		case 0x82: i->set("REF_DEATH_ESP_V", format_immediate()); break;
		case 0x83: i->set("REF_BIRTH_LOCAL", format_immediate()); break;
		case 0x84: i->set("REF_DEATH_LOCAL", format_immediate()); break;
		case 0x85: i->set("REF_BIRTH_LCLFLD", format_address()); break;
		case 0x86: i->set("REF_DEATH_LCLFLD", format_address()); break;
		case 0x87: i->set("REF_UNTR_EBP_V", format_immediate()); break;
		case 0x88: i->set("REF_UNTR_ESP_V", format_immediate()); break;
		case 0x89: i->set("REF_UNTR_EBP_VS", format_immediate()); break;
		case 0x8a: i->set("REF_UNTR_ESP_VS", format_immediate()); break;
		case 0x8b: i->set("REF_UNTR_LOCAL", format_immediate()); break;
		case 0x8c: i->set("REF_UNTR_LCLFLD", format_immediate()); break;
		case 0x8d: i->set("START_FULLY_INTERRUPTIBLE"); break;
		case 0x8e: i->set("END_FULLY_INTERRUPTIBLE"); break;
		case 0x8f: i->set("GC_PROBE"); break;
		case 0x90: i->set("UNKNOWN_90"); break;
		case 0x91: i->set("UNKNOWN_91"); break;
		case 0x92: i->set("NONREF_PUSH"); break;
		case 0x93: i->set("GCREF_PUSH"); break;
		case 0x94: i->set("BYREF_PUSH"); break;
		case 0x95: i->set("REF_PUSH"); break;
		case 0x96: i->set("REF_POP_1"); break;
		case 0x97: i->set("REF_POP_N", format_immediate()); break;
		case 0x98: i->set("REF_INV_N", format_immediate()); break;
		case 0x99: i->set("REF_DEATH_REGS_POP_N");
				   {
					   unsigned char reg = read_byte();
					   if (reg & 1) ss << "EAX,";
					   if (reg & (1 << 1)) ss << "EDX,";
					   if (reg & (1 << 2)) ss << "ECX,";
					   if (reg & (1 << 3)) ss << "EBX,";
					   if (reg & (1 << 4)) ss << "ESI,";
					   if (reg & (1 << 5)) ss << "EDI,";
					   ss << (reg >> 6);
					   i->operands = ss.str();
				   }
			break;
		case 0x9a:
			i->opcode = "SWITCH";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_dword(read_dword_le());
			break;
		case 0x9b: {
			i->opcode = "SWITCH_TABLE";
			uint32_t count = read_dword_le();
			i->operands += format_dword(count) += ":";
			for (uint32_t c = 0; c < count; c++) {
				i->operands += " " + format_dword();
			}
			break;
		}
		case 0x9c:
			i->opcode = "LOAD_TOKEN";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_dword();
			break;
		case 0x9d:
			i->opcode = "PUSH_TOKEN";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_dword();
			break;
		case 0x9e:
			i->opcode = "LOAD_STRING";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_string_token();
			break;
		case 0x9f:
			i->opcode = "PUSH_STRING";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_string_token();
			break;
		case 0xa0:
			i->set("LOAD_FUNCTION");
			i->operands += mdil_reg(read_byte()) + ", ";
			i->operands += format_method_token(read_dword_le());
			break;
		case 0xa1:
			i->set("LOAD_VIRT_FUNCTION");
			i->operands += mdil_reg(read_byte()) + ", ";
			i->operands += format_method_token(read_dword_le());
			break;
		case 0xa2: i->set("PUSH_FUNCTION", format_method_token()); break;
		case 0xa3:
			i->set("LOAD_RVA_FIELD_ADDR");
			i->operands = mdil_reg(read_byte());
			i->operands += ", " + format_field_token();
			break;
		case 0xa4:
			i->set("LOAD_RVA_FIELD");
			i->operands = mdil_reg(read_byte());
			i->operands += ", " + format_field_token();
			break;
		case 0xa5: i->set("LOAD_GS_COOKIE", mdil_reg(read_byte())); break;
		case 0xa6: i->set("LOAD_STATIC_SYNC_OBJ", mdil_reg(read_byte())); break;
		case 0xa7: i->set("LOCAL_BLOCK", format_immediate()); break;
		case 0xa8: i->set("LOCAL_STRUCT", format_type_token()); break;
		case 0xaa: i->set("PARAM_BLOCK", format_immediate()); break;
		case 0xab: i->set("PARAM_STRUCT", format_type_token()); break;
		case 0xac: i->set("UNKNOWN_AC"); break;
		case 0xaf: i->set("INIT_VAR", format_immediate()); break;
		case 0xb0: i->set("INIT_STRUCT", format_address()); break;
		case 0xb1: i->set("ARG_COUNT", format_immediate()); break;
		case 0xb2: i->set("EBP_FRAME"); break;
		case 0xb3: i->set("DOUBLE_ALIGN_ESP"); break;
		case 0xb4: {
			i->opcode = "PUSH_REGS";
			unsigned char regs = read_byte();
			if (regs & 1) ss << "EBX ";
			if (regs & (1 << 1)) ss << "ESI ";
			if (regs & (1 << 2)) ss << "EDI ";
			if (regs & (1 << 3)) ss << "EBP ";
			if (regs & (1 << 4)) ss << "R12 ";
			if (regs & (1 << 5)) ss << "R13 ";
			if (regs & (1 << 6)) ss << "R14 ";
			if (regs & (1 << 7)) ss << "R15 ";
			i->operands = ss.str();
			break;
		}
		case 0xb8: i->set("FRAME_SIZE", format_immediate()); break;
		case 0xb9: i->set("END_PROLOG"); break;
		case 0xba: i->set("EPILOG"); break;
		case 0xbb: i->set("EPILOG_RET"); break;
		case 0xbc: i->set("END_EPILOG"); break;
		case 0xbd: i->set("SECURITY_OBJECT", format_immediate()); break;
		case 0xbe: i->set("GS_COOKIE_OFFSET", format_immediate()); break;
		case 0xbf: i->set("LOCALLOC_USED"); break;
		case 0xc0: i->set("VAR_ARGS"); break;
		case 0xc1: i->set("PROFILER_CALLBACKS"); break;
		case 0xc2: i->set("EDIT_AND_CONTINUE"); break;
		case 0xc3: i->set("SYNC_START"); break;
		case 0xc4: i->set("SYNC_END"); break;
		case 0xc5: i->set("GENERIC_LOOKUP", format_dword(read_dword_le())); break;
		case 0xc6: i->set("UNKNOWN_C6", format_byte()); break;
		case 0xc8: i->set("CONST_DATA", format_const_data(read_dword_le())); break;
		case 0xc9:
			i->set("LOAD_VARARGS_COOKIE");
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_dword(); break;
		case 0xca: i->set("PUSH_VARARGS_COOKIE", format_dword(read_dword_le())); break;
		case 0xcb: i->set("UNKNOWN_CB", format_byte()); break;
		case 0xcc: i->set("PINVOKE_RESERVE_FRAME", format_byte()); break;
		case 0xcd: i->set("PINVOKE_LEAVE_RUNTIME", format_byte()); break;
		case 0xce: i->set("PINVOKE_ENTIRE_RUNTIME", format_byte()); break;
		case 0xcf:
			i->set("PINVOKE_RESERVE_FRAME_WITH_CURRENTMETHOD_DESCRIPTOR");
			i->operands = format_byte();
			i->operands += ", " + format_dword();
			break;
		case 0xd0: i->set("PRESERVE_REGISTER_ACROSS_PROLOG"); break;
		case 0xd5: i->set("UNKNOWN_D5", format_immediate()); break;
		case 0xd6: i->set("UNKNOWN_D6", format_immediate()); break;
		case 0xd7:
			i->set("UNKNOWN_D7");
			i->operands = format_byte();
			i->operands += ", " + format_immediate(); break;
		case 0xd8: i->set("UNKNOWN_D8", format_byte()); break;
		case 0xd9: i->set("UNKNOWN_D9", format_byte()); break;
		case 0xda: i->set("UNKNOWN_DA", format_byte()); break;
		case 0xdb: i->set("UNKNOWN_DB"); break;
		case 0xdc: i->set("UNKNOWN_DC"); break;
		case 0xdd: i->set("UNKNOWN_DD"); break;
		case 0xde: i->set("UNKNOWN_DE"); break;
		case 0xdf: i->set("UNKNOWN_DF"); break;
		case 0xe0: i->set("UNKNOWN_E0"); break;
		case 0xe1: i->set("UNKNOWN_E1"); break;
		case 0xe2: i->set("UNKNOWN_E2"); break;
		case 0xe3: i->set("UNKNOWN_E3"); break; //OK
		case 0xe4: i->set("UNKNOWN_E4"); break;
		case 0xe5: i->set("UNKNOWN_E5"); break;
		case 0xe6: i->set("UNKNOWN_E6"); break;
		case 0xe7: i->set("UNKNOWN_E7"); break;
		case 0xe8: i->set("UNKNOWN_E8"); break; //??
		case 0xe9: i->set("UNKNOWN_E9"); break; //!!
		case 0xea: i->set("UNKNOWN_EA"); break;
		case 0xeb: i->set("UNKNOWN_EB"); break; //!!
		case 0xec: i->set("UNKNOWN_EC"); break;
		case 0xed: i->set("UNKNOWN_ED"); break; //OK
		case 0xee: i->set("UNKNOWN_EE", format_byte()); break; //!!
		case 0xef: i->set("UNKNOWN_EF", format_immediate()); break;
		case 0xf0: i->set("UNKNOWN_F0", format_byte()); break; //!!
		case 0xf1: i->set("UNKNOWN_F1"); break; //!!
		case 0xf2: //!!
			i->set("UNKNOWN_F2");
			i->operands = format_immediate();
			i->operands += ", " + format_immediate();
			break;
		case 0xf3: i->set("UNKNOWN_F3", format_dword()); break;
		case 0xf4: i->set("UNKNOWN_F4"); break;
		case 0xf7: i->set("UNKNOWN_F7"); break;
		case 0xf8: i->set("UNKNOWN_F8"); break;
		case 0xf9: i->set("UNKNOWN_F9", format_dword()); break;
		case 0xfb: i->set("UNKNOWN_FB"); break;
		case 0xfc: i->set("UNKNOWN_FC"); break;
		case 0xfd: i->set("UNKNOWN_FD"); break;
		case 0xfe: i->set("UNKNOWN_FE"); break;
		//case 0xff: i->set("UNKNOWN_FF"); break;
		default: i->opcode = "*ILLEGAL*"; m_error = true; break;
		}

		ss.str("");
		ss.clear();
		i->length = m_pos - i->offset;
		routine.push_back(i);
		if (m_error) break;
	}
	return routine;
}

char mdil_decoder::read_signed_byte()
{
	return (char) m_buffer[m_pos++];
}

unsigned char mdil_decoder::read_byte()
{
	return m_buffer[m_pos++];
}

unsigned short mdil_decoder::read_word_be()
{
	unsigned short ret = (m_buffer[m_pos] << 8) + m_buffer[m_pos+1];
	m_pos += 2;
	return ret;
}

unsigned short mdil_decoder::read_word_le()
{
	unsigned short ret = m_buffer[m_pos] + (m_buffer[m_pos+1] << 8);
	m_pos += 2;
	return ret;
}

unsigned long mdil_decoder::read_dword_le()
{
	unsigned long ret = m_buffer[m_pos] + (m_buffer[m_pos+1] << 8) + (m_buffer[m_pos+2] << 16) + (m_buffer[m_pos+3] << 24);
	m_pos += 4;
	return ret;
}

std::string mdil_decoder::format_byte( unsigned char val )
{
	stringstream s;
	s << uppercase << hex << setfill('0') << setw(2) << (int) val;
	return s.str();
}

std::string mdil_decoder::format_byte()
{
	return format_byte(read_byte());
}

std::string mdil_decoder::format_dword( unsigned long val )
{
	stringstream s;
	s << uppercase << hex << setfill('0') << setw(8) << val;
	return s.str();
}

std::string mdil_decoder::format_dword()
{
	return format_dword(read_dword_le());
}

std::string mdil_decoder::format_type_token()
{
	unsigned char first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf) return format_dword(0x02000000 + (first_byte << 8) + read_byte());
		else if (first_byte <= 0xef) return format_dword(0x01000000 + ((first_byte - 0xc0) << 8) + read_byte());
		else return format_dword(0x1b000000 + ((first_byte - 0xf0) << 8) + read_byte());
	} else return format_dword(read_dword_le());
}

std::string mdil_decoder::format_address_modifier( uint8_t modifier, const char* str, bool& bracketed )
{
	string ret(str);
	
	bool has_next = (modifier & 0x80) != 0;
	modifier &= 0x7f;

	if (modifier == 0x01) {
		if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }
		ret += "." + format_field_token();
	} else if (modifier == 0x02) {
		uint8_t index = read_byte();
		ret += ".";
		char scale = '?';
		switch (index >> 4)
		{
		case 0x0: ret += "BytePtr"; scale = '1'; break;
		case 0x1: ret += "WordPtr"; scale = '2'; break;
		case 0x2: ret += "DWordPtr"; scale = '4'; break;
		case 0x3: ret += "QWordPtr"; scale = '8'; break;
		case 0x4: ret += "ByteArray"; scale = '1'; break;
		case 0x5: ret += "WordArray"; scale = '2'; break;
		case 0x6: ret += "DWordArray"; scale = '4'; break;
		case 0x7: ret += "QWordArray"; scale = '8'; break;
		case 0x8: ret += "StructArray<" + format_type_token() + ">"; scale = 'X'; break;
		case 0xa: ret += "RefArray"; scale = '4'; break;
		case 0xb: ret += "String"; scale = '?'; break;
		case 0xc: ret += "MDimArray " + format_byte(read_byte()); break;
		case 0xe: ret += "Scale " + format_byte(read_byte()); break;
		default: ret += "???"; break;
		}
		ret += " + " + mdil_reg(index & 0x0f) + " * " + string(1, scale);
		if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }
	} else if (modifier == 0x03) {
		ret +=  " + " + format_immediate();
		if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }
	} else if ((modifier >= 0x10) && (modifier <= 0x17)) {
		if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }
		switch (modifier)
		{
		case 0x10: ret += ".ByteArrayLength"; break;
		case 0x12: ret += ".WordArrayLength"; break;
		case 0x13: ret += ".DWordArrayLength"; break;
		case 0x14: ret += ".QWordArrayLength"; break;
		case 0x15: ret += ".RefArrayLength"; break;
		case 0x16: ret += ".StructArrayLength<" + format_type_token() + ">"; break;
		case 0x17: ret += ".MDimArrayLength";
			ret += "( " + format_byte() +", ";
			ret += format_byte() + ")";
			break;
		default: break;
		}
	} else if ((modifier >= 0x18) && (modifier <= 0x1e)) {
		if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }
		switch (modifier)
		{
		case 0x18: ret = "BYTE " + ret; break;
		case 0x19: ret = "WORD " + ret; break;
		case 0x1a: ret = "DWORD " + ret; break;
		case 0x1b: ret = "QWORD " + ret; break;
		case 0x1c: ret = "DQWORD " + ret; break;
		case 0x1d: ret = "TYPE<" + format_type_token() + "> " + ret; break;
		case 0x1e: ret = "REF " + ret; break;
		default: break;
		}
	} else {
		ret += "(AM " + format_byte(modifier) + ")";
	}

	if (has_next) ret = format_address_modifier(read_byte(), ret.c_str(), bracketed);

	if (!bracketed) { ret = "[" + ret + "]"; bracketed = true; }

	return ret;
}

std::string mdil_decoder::format_address_base(uint8_t base_reg, uint8_t flags)
{
	string ret;

	if (true) { // x86
		bool bracketed = false;
		if (flags == 0x0) ret = format_address_modifier(0x01, mdil_reg(base_reg).c_str(), bracketed);
		else if (flags == 0x1) ret = format_address_modifier(0x02, mdil_reg(base_reg).c_str(), bracketed);
		else if (flags == 0x2) ret = "DWORD [" + mdil_reg(base_reg) + "]";
		else if (flags == 0x3) ret = format_address_modifier(read_byte(), mdil_reg(base_reg).c_str(), bracketed);
	}

	return ret;
}

std::string mdil_decoder::format_address( std::function<std::string(uint8_t, uint8_t, uint8_t)> formatter )
{
	unsigned char addr_regs_byte = read_byte();

	if (true) { // x86
		uint8_t op_reg = (addr_regs_byte >> 3) & 0x7;
		uint8_t base_reg = addr_regs_byte & 0x7;
		uint8_t flags = (addr_regs_byte >> 6) & 0x3;

		if (formatter != nullptr)
			return formatter(op_reg, base_reg, flags);
		else {
			return mdil_reg(op_reg) + ", " + format_address_base(base_reg, flags);
		}
	}
}

std::string mdil_decoder::format_address_no_reg()
{
	unsigned char addr_regs_byte = read_byte();

	if (true) { // x86
		uint8_t op_reg = (addr_regs_byte >> 3) & 0x7;
		uint8_t base_reg = addr_regs_byte & 0x7;
		uint8_t flags = (addr_regs_byte >> 6) & 0x3;
		if (op_reg == 0) return format_address_base(base_reg, flags);
		else return "(ERR:" + format_byte(op_reg) + ")" + format_address_base(base_reg, flags);
	}
}

std::string mdil_decoder::format_string_token()
{
	uint8_t first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf)	return format_dword(0x70000000 + (first_byte << 8) + read_byte());
		else return format_dword(0x7000c000 + ((first_byte - 0xc0) << 16) + read_word_le());
	} else return format_dword();
}

std::string mdil_decoder::format_field_token()
{
	uint8_t first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf)	return format_dword(0x04000000 + (first_byte << 8) + read_byte());
		else if (first_byte <= 0xdf) return format_dword(0x0a000000 + ((first_byte - 0xc0) << 8) + read_byte());
		else if (first_byte <= 0xef) return format_dword(0x04000000 + ((first_byte - 0xe0) << 16) + read_word_le());
		else return format_dword(0x0a002000 + ((first_byte - 0xf0) << 16) + read_word_le());
	} else return format_dword();
}

std::string mdil_decoder::format_method_token( unsigned long val )
{
	return format_dword(val);
}

std::string mdil_decoder::format_method_token()
{
	return format_method_token(read_dword_le());
}

std::string mdil_decoder::format_jump_distance(bool jump_long)
{
	int32_t distance;

	if (!jump_long) {
		distance = read_signed_byte();
		if (distance == -1) jump_long = true;
	}

	if (jump_long) distance = (int32_t) read_dword_le();

	stringstream s;
	if (distance < 0) { s << "-"; distance = -distance; }
	s << uppercase << hex << setfill('0') << setw(jump_long ? 8 : 2) << (int) distance;
	return s.str();
}
