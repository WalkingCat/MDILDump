#include "stdafx.h"
#include "mdil_decoder.h"

using namespace std;


mdil_decoder::mdil_decoder(const unsigned char* buffer, const unsigned long length, const mdil_architecture _architecture )
	: m_buffer(buffer), m_length(length), m_pos(0), m_error(false), architecture(_architecture) {}

string mdil_decoder::format_reg_byte (uint8_t reg) {
	switch (reg & 0xF) {
	case 0x0 : return is_arm() ? "R0" : "EAX";
	case 0x1 : return is_arm() ? "R1" : "ECX";
	case 0x2 : return is_arm() ? "R2" : "EDX";
	case 0x3 : return is_arm() ? "R3" : "EBX";
	case 0x4 : return is_arm() ? "R4" : "ESP?";
	case 0x5 : return is_arm() ? "R5" : "EBP";
	case 0x6 : return is_arm() ? "R6" : "ESI";
	case 0x7 : return is_arm() ? "R7" : "EDI";
	case 0x8 : return "R8";
	case 0x9 : return "R9";
	case 0xA : return "R10";
	case 0xB : return "R11";
	case 0xC : return "R12";
	case 0xD : return "SP";
	case 0xE : return "LR";
	case 0xF : return "PC";
	default  : return "!!!";
	}
}

string mdil_decoder::format_reg_byte () {
	return format_reg_byte(read_byte());
}

uint32_t mdil_decoder::read_immediate_uint32()
{
	unsigned char first_byte = m_buffer[m_pos++];
	switch (first_byte)
	{
	case 0xbb: { 
		short value = (short) (m_buffer[m_pos] + (m_buffer[m_pos+1] << 8));
		m_pos += 2;
		return value;
	}
	case 0xdd: return read_dword_le();
	case 0xbd:
	case 0xdb:
		m_error = true;
		return 0;
	default: return first_byte;
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
		ret << "*" << format_immediate(); // see ArgSize ???
		ret << "+" << format_immediate(); // see ArgSize ???
		break;
	case 0xdb:
		ret << "ArgumentSize[" << format_type_token() << "]";
		ret << "*" << format_byte(); // different from the patent doc !!!??? // format_immediate();
		ret << "+" << format_immediate(); // so how about this one ???
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

	m_pos = 0;
	m_error = false;

	while (m_pos < m_length) {
		auto i = m_instr = make_shared<mdil_instruction>(mdil_instruction::ilMDIL, m_pos);

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
		case 0x1a: i->set(is_arm() ? "VLOAD" : "ADD", format_address()); break;
		case 0x1b: i->set(is_arm() ? "VSTORE" : "ADC", format_address()); break;
		case 0x1c: i->set(is_arm() ? "LOAD_IMM" : "AND", format_address()); break;
		case 0x1d: i->set(is_arm() ? "PUSH_VREGS" : "CMP", format_address()); break; // actually, just 2 bytes
		case 0x1e: i->set(is_arm() ? "LOAD_LABEL" : "OR", format_address()); break;
		case 0x1f: i->set(is_arm() ? "NULL_CHECK" : "SUB", format_address()); break; // actually, just 1 byte
		case 0x20: i->set(is_arm() ? "TBD_6" : "SBB", format_address()); break;
		case 0x21: i->set(is_arm() ? "TBD_7" : "XOR", format_address()); break;
		case 0x22: i->set(is_arm() ? "TBD_8" : "ADD_TO", format_address()); break;
		case 0x23: i->set(is_arm() ? "TBD_9" : "ADC_TO", format_address()); break;
		case 0x24: i->set(is_arm() ? "TBD_10" : "AND_TO", format_address()); break;
		case 0x25: i->set(is_arm() ? "TBD_11" : "CMP_TO", format_address()); break;
		case 0x26: i->set(is_arm() ? "TBD_12" : "OR_TO", format_address()); break;
		case 0x27: i->set(is_arm() ? "TBD_13" : "SUB_TO", format_address()); break;
		case 0x28: i->set(is_arm() ? "TBD_14" : "SBB_TO", format_address()); break;
		case 0x29: i->set(is_arm() ? "TBD_15" : "XOR_TO", format_address()); break;
		case 0x2a: i->set(is_arm() ? "TBD_16" : "TEST", format_address()); break;
		case 0x2b: i->set(is_arm() ? "TBD_17" : "MUL_DIV_EAX", format_address()); break;
		case 0x2c: i->set(is_arm() ? "TBD_18" : "IMUL", format_address()); break;
		case 0x2d: i->set(is_arm() ? "TBD_19" : "IMUL_IMM", format_address()); break;
		case 0x2e:
			i->set("ELEM_SCALE");
			i->operands = format_reg_byte();
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
		case 0x31: i->set("OP_IMM", format_address_no_reg()); i->operands += ", " + format_immediate(); break;
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
		case 0x41: i->set("LOAD_RESULT", format_address()); break;
		case 0x42: i->set("STORE_RESULT"); break;
		case 0x43: i->set("PUSH_RESULT"); break;
		case 0x44: i->set("DISCARD_RESULT"); break;
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
		case 0x4b: i->set("HELPER_CALL", format_helper_call()); break;
		case 0x4c: i->set("CONSTRAINT", format_type_token()); break;
		case 0x4d: i->set("CALL_DEF", format_method_token(0x06000000 + read_word_le())); break;
		case 0x4e: i->set("CALL_REF", format_method_token(0x0A000000 + read_word_le())); break;
		case 0x4f: i->set("CALL_VIRT_DEF", format_method_token(0x06000000 + read_word_le())); break;
		case 0x50: i->set("CALL_VIRT_REF", format_method_token(0x0A000000 + read_word_le())); break;
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
		case 0x67: i->set("BEGIN_FILTER"); break;
		case 0x68: i->set("END_FILTER"); break;
		case 0x69: i->set("ISINST", format_type_token()); break;
		case 0x6a: i->set("CASTCLASS", format_type_token()); break;
		case 0x6b: i->set("BOX", format_type_token()); break;
		case 0x6c: i->set("UNBOX", format_type_token()); break;
		case 0x6d: i->set("ALLOC_OBJECT", format_type_token()); break;
		case 0x6e: i->set("ALLOC_ARRAY", format_type_token()); break;
		case 0x6f: i->set(is_arm() ? "REF_BIRTH_R0" : "REF_BIRTH_EAX"); break;
		case 0x70: i->set(is_arm() ? "REF_BIRTH_R1" : "REF_BIRTH_ECX"); break;
		case 0x71: i->set(is_arm() ? "REF_BIRTH_R2" : "REF_BIRTH_EDX"); break;
		case 0x72: i->set(is_arm() ? "REF_BIRTH_R3" : "REF_BIRTH_EBX"); break;
		case 0x73: {
			i->set(is_arm() ? "REF_BIRTH_R4" : "REF_BIRTH_REG");
			if (!is_arm()) {
				uint8_t b = read_byte();
				i->operands = format_reg_byte(( b >> 3) & 0x1f);
			}
			break;
		}
		case 0x74: i->set(is_arm() ? "REF_BIRTH_R5" : "REF_BIRTH_EBP"); break;
		case 0x75: i->set(is_arm() ? "REF_BIRTH_R6" : "REF_BIRTH_ESI"); break;
		case 0x76: i->set(is_arm() ? "REF_BIRTH_R7" : "REF_BIRTH_EDI"); break;
		case 0x77: i->set(is_arm() ? "REF_DEATH_R0" : "REF_DEATH_EAX"); break;
		case 0x78: i->set(is_arm() ? "REF_DEATH_R1" : "REF_DEATH_ECX"); break;
		case 0x79: i->set(is_arm() ? "REF_DEATH_R2" : "REF_DEATH_EDX"); break;
		case 0x7a: i->set(is_arm() ? "REF_DEATH_R3" : "REF_DEATH_EBX"); break;
		case 0x7b: {
			i->set(is_arm() ? "REF_DEATH_R4" : "REF_DEATH_REG"); 
			if (!is_arm()) {
				uint8_t b = read_byte();
				i->operands = format_reg_byte(( b >> 3) & 0x1f);
			}
			break;
		}
		case 0x7c: i->set(is_arm() ? "REF_DEATH_R5" : "REF_DEATH_EBP"); break;
		case 0x7d: i->set(is_arm() ? "REF_DEATH_R6" : "REF_DEATH_ESI"); break;
		case 0x7e: i->set(is_arm() ? "REF_DEATH_R7" : "REF_DEATH_EDI"); break;
		case 0x7f: i->set(is_arm() ? "REF_BIRTH_FP_V" : "REF_BIRTH_EBP_V", format_immediate()); break;
		case 0x80: i->set(is_arm() ? "REF_DEATH_FP_V" : "REF_DEATH_EBP_V", format_immediate()); break;
		case 0x81: i->set(is_arm() ? "REF_BIRTH_SP_V" : "REF_BIRTH_ESP_V", format_immediate()); break;
		case 0x82: i->set(is_arm() ? "REF_DEATH_SP_V" : "REF_DEATH_ESP_V", format_immediate()); break;
		case 0x83: i->set("REF_BIRTH_LOCAL", format_immediate()); break;
		case 0x84: i->set("REF_DEATH_LOCAL", format_immediate()); break;
		case 0x85: i->set("REF_BIRTH_LCLFLD", format_address()); break;
		case 0x86: i->set("REF_DEATH_LCLFLD", format_address()); break;
		case 0x87: i->set(is_arm() ? "REF_UNTR_FP_V" : "REF_UNTR_EBP_V", format_immediate()); break;
		case 0x88: i->set(is_arm() ? "REF_UNTR_SP_V" : "REF_UNTR_ESP_V", format_immediate()); break;
		case 0x89: i->set(is_arm() ? "REF_UNTR_FP_VS" : "REF_UNTR_EBP_VS", format_immediate()); break;
		case 0x8a: i->set(is_arm() ? "REF_UNTR_SP_VS" : "REF_UNTR_ESP_VS", format_immediate()); break;
		case 0x8b: i->set("REF_UNTR_LOCAL", format_immediate()); break;
		case 0x8c: i->set("REF_UNTR_LCLFLD", format_var_number()); i->operands += format_immediate(); break;
		case 0x8d: i->set("START_FULLY_INTERRUPTIBLE"); break;
		case 0x8e: i->set("END_FULLY_INTERRUPTIBLE"); break;
		case 0x8f: i->set("GC_PROBE"); break;
		case 0x90: i->set("THIS_REG"); break;
		case 0x91: i->set("THIS_STACK"); break;
		case 0x92: i->set(is_arm() ? "REF_BIRTH_REG" : "NONREF_PUSH", is_x86() ? "" : format_byte()); break; //!!! flags in low 3 bits
		case 0x93: i->set(is_arm() ? "REF_DEATH_REG" : "GCREF_PUSH", is_x86() ? "" : format_byte()); break; //!!! flags in low 3 bits
		case 0x94: i->set("BYREF_PUSH"); break;
		case 0x95: i->set("REF_PUSH"); break;
		case 0x96: i->set("REF_POP_1"); break;
		case 0x97: i->set("REF_POP_N", format_immediate()); break;
		case 0x98:
			i->set("REF_INV_N");
			if (peek_byte() != 0xdb) i->operands == format_immediate();
			else { read_byte(); format_signature_token(); }
			break;
		case 0x99: i->set(is_arm() ? "REF_DEATH_REGS" : "REF_DEATH_REGS_POP_N");
				   {
					   unsigned char reg = read_byte();
					   string s;
					   if (reg & 1) s += is_arm() ? "R0 " : "EAX ";
					   if (reg & (1 << 1)) s += is_arm() ? "R2 " :  "EDX ";
					   if (reg & (1 << 2)) s += is_arm() ? "R1 " :  "ECX ";
					   if (reg & (1 << 3)) s += is_arm() ? "R3 " :  "EBX ";
					   if (reg & (1 << 4)) s += is_arm() ? "R6 " :  "ESI ";
					   if (reg & (1 << 5)) s += is_arm() ? "R7 " :  "EDI ";
					   s += format_byte(reg >> 6);
					   i->operands = s;
				   }
			break;
		case 0x9a:
			i->opcode = "SWITCH";
			i->operands = format_reg_byte() + ", ";
			i->operands += format_dword(read_dword_le());
			break;
		case 0x9b: {
			i->opcode = "SWITCH_TABLE";
			uint32_t count = read_dword_le();
			i->operands += format_dword(count) += ":";
			if ((m_pos + (count * 4)) <= m_length) {
				for (uint32_t c = 0; c < count; c++) {
					i->operands += " " + format_dword();
				}
			} else { i->operands += "INVALID: TOO LONG"; m_error = true; }
			break;
		}
		case 0x9c:
			i->opcode = "LOAD_TOKEN";
			i->operands = format_reg_byte() + ", ";
			i->operands += format_token_dword();
			break;
		case 0x9d:
			i->opcode = "PUSH_TOKEN";
			i->operands = format_reg_byte() + ", ";
			i->operands += format_token_dword();
			break;
		case 0x9e:
			i->opcode = "LOAD_STRING";
			i->operands = format_reg_byte() + ", ";
			i->operands += format_string_token();
			break;
		case 0x9f:
			i->opcode = "PUSH_STRING";
			i->operands = format_reg_byte() + ", ";
			i->operands += format_string_token();
			break;
		case 0xa0:
			i->set("LOAD_FUNCTION");
			i->operands += format_reg_byte() + ", ";
			i->operands += format_method_token(read_dword_le());
			break;
		case 0xa1:
			i->set("LOAD_VIRT_FUNCTION");
			i->operands += format_reg_byte() + ", ";
			i->operands += format_method_token(read_dword_le());
			break;
		case 0xa2: i->set("PUSH_FUNCTION", format_method_token()); break;
		case 0xa3:
			i->set("LOAD_RVA_FIELD_ADDR");
			i->operands = format_reg_byte();
			i->operands += ", " + format_field_token();
			break;
		case 0xa4:
			i->set("LOAD_RVA_FIELD");
			i->operands = format_reg_byte();
			i->operands += ", " + format_field_token();
			break;
		case 0xa5: i->set("LOAD_GS_COOKIE", format_reg_byte()); break;
		case 0xa6: i->set("LOAD_STATIC_SYNC_OBJ", format_reg_byte()); break;
		case 0xa7: i->set("LOCAL_BLOCK", ((peek_byte() != 0xff) && (peek_byte() != 0xfe)) ? format_immediate() : format_byte()); break;
		case 0xa8: i->set("LOCAL_STRUCT", format_type_token()); break;
		case 0xa9: i->set("COND_LOCAL"); break;
		case 0xaa: i->set("PARAM_BLOCK", ((peek_byte() != 0xff) && (peek_byte() != 0xfe)) ? format_immediate() : format_byte()); break;
		case 0xab: i->set("PARAM_STRUCT", format_type_token()); break;
		case 0xac: i->set("INST_ARG"); break;
		case 0xad: i->set("LOAD_INST"); break;
		case 0xae: i->set("PUSH_INST"); break;
		case 0xaf: i->set("INIT_VAR", format_immediate()); break;
		case 0xb0: i->set("INIT_STRUCT", format_address()); break;
		case 0xb1: i->set("ARG_COUNT", format_immediate()); break;
		case 0xb2: i->set(is_arm() ? "FP_FRAME" : "EBP_FRAME"); break;
		case 0xb3: i->set("DOUBLE_ALIGN_ESP"); break;
		case 0xb4: {
			i->opcode = "PUSH_REGS";
			string s;
			uint16_t regs = read_byte();
			if (regs == 0xBB) regs = read_word_le(); // arm. how about x86 ??
			if (regs & 1) s += is_arm() ? "R3 " : "EBX ";
			if (regs & (1 << 1)) s += is_arm() ? "R6 " : "ESI ";
			if (regs & (1 << 2)) s += is_arm() ? "R7 " : "EDI ";
			if (regs & (1 << 3)) s += is_arm() ? "R5 " : "EBP ";
			if (regs & (1 << 4)) s += "R12 ";
			if (regs & (1 << 5)) s += "R13 ";
			if (regs & (1 << 6)) s += "R14 ";
			if (regs & (1 << 7)) s += "R15 ";
			i->operands = s;
			break;
		}
		case 0xb5: i->set("SAVE_REG"); break;
		case 0xb6: i->set("SAVE_XMM_REG"); break;
		case 0xb7: i->set("FRAME_PTR"); break;
		case 0xb8: i->set("FRAME_SIZE", format_immediate()); break;
		case 0xb9: i->set("END_PROLOG"); break;
		case 0xba: i->set("EPILOG"); break;
		case 0xbb: i->set("EPILOG_RET"); break;
		case 0xbc: i->set("END_EPILOG"); break;
		case 0xbd: i->set("SECURITY_OBJECT", format_immediate()); break;
		case 0xbe: i->set("GS_COOKIE_OFFSET", format_immediate()); break;
		case 0xbf: i->set("LOCALLOC_USED", format_byte()); break;
		case 0xc0: i->set("VAR_ARGS"); break;
		case 0xc1: i->set("PROFILER_CALLBACKS"); break;
		case 0xc2: i->set("EDIT_AND_CONTINUE"); break;
		case 0xc3: i->set("SYNC_START"); break;
		case 0xc4: i->set("SYNC_END"); break;
		case 0xc5: i->set("GENERIC_LOOKUP", format_token_dword()); break;
		case 0xc6: i->set("FUNCLET", format_byte()); i->operands += format_immediate(); break;
		case 0xc7: i->set("COLDCODE"); break;
		case 0xc8: i->set("CONST_DATA", format_const_data(read_dword_le())); break;
		case 0xc9:
			i->set("LOAD_VARARGS_COOKIE");
			i->operands = format_reg_byte(read_byte()) + ", ";
			i->operands += format_dword(); break;
		case 0xca: i->set("PUSH_VARARGS_COOKIE", format_dword(read_dword_le())); break;
		case 0xcb: i->set("GET_TLS_BASE", format_field_token()); break;
		case 0xcc: i->set(is_arm() ? "GET_THREADSTATIC_BASE" : "PINVOKE_RESERVE_FRAME", format_byte()); break;
		case 0xcd: i->set(is_arm() ? "GET_THREADSTATIC_BASE_GC" : "PINVOKE_LEAVE_RUNTIME", format_byte()); break;
		case 0xce: i->set(is_arm() ? "PINVOKE_RESERVE_FRAME" : "PINVOKE_ENTIRE_RUNTIME", format_byte()); break;
		case 0xcf:
			i->set(is_arm() ? "PINVOKE_LEAVE_RUNTIME" : "PINVOKE_RESERVE_FRAME_WITH_CURRENTMETHOD_DESCRIPTOR");
			i->operands = format_byte();
			i->operands += ", " + format_dword();
			break;
		case 0xd0: i->set(is_arm() ? "PINVOKE_ENTER_RUNTIME" : "PRESERVE_REGISTER_ACROSS_PROLOG"); break;
		case 0xd1: i->set(is_arm() ? "PINVOKE_RESERVE_FRAME_WITH_CURRENTMETHO" : "UNKNOWN_D1"); break;
		case 0xd2: i->set(is_arm() ? "PRESERVE_REGISTER_ACROSS_PROLOG" : "UNKNOWN_D2"); break;
		case 0xd3: i->set(is_arm() ? "CALL_PINVOKE" : "UNKNOWN_D3"); break;
		case 0xd4: i->set("REMOVEME_CALL_INDIRECT_STACK_ARGUMENT_S"); break;
		case 0xd5: i->set("REVERSE_PINVOKE_METHOD", format_immediate()); i->operands += format_immediate(); break;
		case 0xd6: { //!!
			i->set("OUTGOING_ARG_SIZE");
			if (peek_byte() == 0xdb) {
				read_byte();
				i->operands = format_signature_token();
			} else i->operands = format_immediate();
			break;
		}
		case 0xd7: //!!
			i->set("GENERICS_CONTEXT", is_x86() ? format_byte() :format_var_number()); // REALLY ?
			break;
		case 0xd8: i->set("ALIGN_ESP", format_byte()); break;
		case 0xd9: i->set("END", format_byte()); break;
		case 0xda: i->set("NATIVE_SHORTCUT", format_byte()); break;
		case 0xdb: i->set("TBD_21"); break;
		case 0xdc: i->set("TBD_22"); break;
		case 0xdd: i->set("TBD_23"); break;
		case 0xde: i->set("TBD_24"); break;
		case 0xdf: i->set("TBD_25"); break;
		case 0xe0: i->set("TBD_26"); break;
		case 0xe1: i->set("TBD_27"); break;
		case 0xe2: i->set("TBD_28"); break;
		case 0xe3: i->set("TBD_29"); break; //OK
		case 0xe4: i->set("TBD_30"); break;
		case 0xe5: i->set("TBD_31"); break;
		case 0xe6: i->set("TBD_32"); break;
		case 0xe7: i->set("TBD_33"); break;
		case 0xe8: i->set("TBD_34"); break; //??
		case 0xe9: i->set("TBD_35"); break; //!!
		case 0xea: i->set("TBD_36"); break;
		case 0xeb: i->set("LOCAL_BLOCK_PTR_SIZE"); break; //!!
		case 0xec: i->set("PARAM_BLOCK_PTR_SIZE"); break; //!!
		case 0xed: i->set("NULL_CHECK_R0"); break;
		case 0xee: i->set("INIT_VARS", is_arm() ? format_dword(read_word_le()) : format_immediate());  break;
		case 0xef: i->set("REF_UNTR_LOCALS", is_arm() ? format_dword(read_word_le()) : format_immediate()); break; // this cant be right
		case 0xf0: i->set("PINVOKE_INIT_FRAME", format_immediate()); break;
		case 0xf1: i->set("REG_PARAMS"); break; //!!
		case 0xf2: //!!
			i->set("HOME_PARAM");
			i->operands = ((peek_byte() != 0xff) && (peek_byte() != 0xfe)) ? format_immediate() : format_byte();
			i->operands += ", " + format_immediate();
			break;
		case 0xf3: {
			i->set("LOAD_ARG");
			uint8_t b = read_byte(); // first operand is a var number
			i->operands = format_byte(b);
			if ((b >> 4) == 0xf) i->operands += ", " + format_immediate(); // var num >= 0xf, encoded as a imm, or its just (b >> 4)
			i->operands += ", " + format_byte(); // 8 ?
			i->operands += ", " + format_immediate();
			i->operands += ", " + format_immediate();
			break;
		}
		case 0xf4: {
			i->set("LOAD_ARG_S");
			uint8_t b = read_byte(); // first operand is a var number
			i->operands = format_byte(b);
			if (((b >> 4) & 0x7) == 0x7) i->operands += ", " + format_immediate(); // var num >= 0x7, encoded as a imm, or its just (b >> 4)
			if ((b & 0xf) == 0xf) i->operands += ", " + format_immediate();
			break;
		}
		case 0xf5: i->set("START_ARG_LIST", format_signature_token()); break;
		case 0xf6: //!!!
			i->set("SIZEOF_STRUCT", format_byte());
			i->operands += ", " + format_dword(read_word_le()); // 01 BD
			i->operands += ", " + format_type_token();
			read_byte(); // 0 ???
			break;
		case 0xf7: i->set("RETURN_VALUE"); break;
		case 0xf8: i->set("CALL_DELEGATE_NO_TOKEN", format_immediate()); break;
		case 0xf9: i->set("CALL_DELEGATE", format_dword()); break;
		case 0xfa: i->set("MDIL_INSTR_COUNT"); break;
		case 0xfb: i->set("UNKNOWN_FB"); break;
		case 0xfc: i->set("UNKNOWN_FC"); break;
		case 0xfd: i->set("UNKNOWN_FD"); break;
		case 0xfe: i->set("UNKNOWN_FE"); break;
		//case 0xff: i->set("UNKNOWN_FF"); break;
		default: m_error = true; break;
		}

		if (!m_error) {
			i->length = m_pos - i->offset;
		} else {
			i->length = 0; // failed
		}
		routine.push_back(i);

		m_instr.reset();

		if (m_error) break;
	}
	return routine;
}

char mdil_decoder::read_signed_byte()
{
	return (char) m_buffer[m_pos++];
}

unsigned char mdil_decoder::peek_byte()
{
	return m_buffer[m_pos];
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

std::string mdil_decoder::format_token_dword()
{
	uint32_t token = read_dword_le();
	m_instr->setref(mdil_instruction::rtMetadataToken, token);
	return format_dword(token);
}

std::string mdil_decoder::format_type_token()
{
	uint32_t token = 0;

	unsigned char first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf) token = 0x02000000 + (first_byte << 8) + read_byte();
		else if (first_byte <= 0xef) token = 0x01000000 + ((first_byte - 0xc0) << 8) + read_byte();
		else token = 0x1b000000 + ((first_byte - 0xf0) << 8) + read_byte();
	} else token = read_dword_le();

	m_instr->setref(mdil_instruction::rtMetadataToken, token);

	return format_dword(token);
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
		default: ret += "###"; break;
		}
		ret += " + " + format_reg_byte(index & 0x0f) + " * " + string(1, scale);
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
		if (flags == 0x0) ret = format_address_modifier(0x01, format_reg_byte(base_reg).c_str(), bracketed);
		else if (flags == 0x1) ret = format_address_modifier(0x02, format_reg_byte(base_reg).c_str(), bracketed);
		else if (flags == 0x2) ret = "DWORD [" + format_reg_byte(base_reg) + "]";
		else if (flags == 0x3) ret = format_address_modifier(read_byte(), format_reg_byte(base_reg).c_str(), bracketed);
	}

	return ret;
}

std::string mdil_decoder::format_address( std::function<std::string(uint8_t, uint8_t, uint8_t)> formatter )
{
	unsigned char addr_regs_byte = read_byte();

	uint8_t op_reg, base_reg, flags;

	if (is_x86()) {
		op_reg = (addr_regs_byte >> 3) & 0x7;
		base_reg = addr_regs_byte & 0x7;
		flags = (addr_regs_byte >> 6) & 0x3;
	} else { // ARM ???
		op_reg = (addr_regs_byte >> 4) & 0xF;
		base_reg = addr_regs_byte & 0xF;
		flags = 0x03;
	}

	if (formatter != nullptr)
		return formatter(op_reg, base_reg, flags);
	else {
		return format_reg_byte(op_reg) + ", " + format_address_base(base_reg, flags);
	}
}

std::string mdil_decoder::format_address_no_reg()
{
	unsigned char addr_regs_byte = read_byte();

	uint8_t op_reg, base_reg, flags;

	if (is_x86()) {
		op_reg = (addr_regs_byte >> 3) & 0x7;
		base_reg = addr_regs_byte & 0x7;
		flags = (addr_regs_byte >> 6) & 0x3;
	} else { // ARM ???
		op_reg = (addr_regs_byte >> 4) & 0xF;
		base_reg = addr_regs_byte & 0xF;
		flags = 0x03;
	}

	if (op_reg == 0) return format_address_base(base_reg, flags);
	else return "(ERR:" + format_byte(op_reg) + ")" + format_address_base(base_reg, flags);
}

std::string mdil_decoder::format_string_token()
{
	uint32_t token = 0;

	uint8_t first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf)	token = 0x70000000 + (first_byte << 8) + read_byte();
		else token = 0x7000c000 + ((first_byte - 0xc0) << 16) + read_word_le();
	} else token = read_dword_le();

	m_instr->setref(mdil_instruction::rtMetadataToken, token);

	return format_dword(token);
}

std::string mdil_decoder::format_field_token()
{
	uint32_t token = 0;

	uint8_t first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte <= 0xbf)	token = 0x04000000 + (first_byte << 8) + read_byte();
		else if (first_byte <= 0xdf) token = 0x0a000000 + ((first_byte - 0xc0) << 8) + read_byte();
		else if (first_byte <= 0xef) token = 0x04000000 + ((first_byte - 0xe0) << 16) + read_word_le();
		else token = 0x0a002000 + ((first_byte - 0xf0) << 16) + read_word_le();
	} else token = read_dword_le();

	m_instr->setref(mdil_instruction::rtMetadataToken, token);

	return format_dword(token);
}


std::string mdil_decoder::format_signature_token()
{
	uint32_t token = 0;

	uint8_t first_byte = read_byte();
	if (first_byte != 0xff) {
		if (first_byte < 0xc0) token = 0x06000000 + (first_byte << 8) + read_byte();
		else if (first_byte < 0xf0) token = 0x0a000000 + ((first_byte - 0xc0) << 8) + read_byte();
		else token = 0x2b000000 + ((first_byte - 0xf0) << 8) + read_byte();
	} else token = read_dword_le();

	m_instr->setref(mdil_instruction::rtMetadataToken, token);

	return format_dword(token);
}


std::string mdil_decoder::format_method_token( unsigned long val )
{
	m_instr->setref(mdil_instruction::rtMetadataToken, val);
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

	m_instr->setref(mdil_instruction::rtJumpDistance, distance);

	stringstream s;
	if (distance < 0) { s << "-"; distance = -distance; }
	s << uppercase << hex << setfill('0') << setw(jump_long ? 8 : 2) << (int) distance;
	return s.str();
}

std::string mdil_decoder::format_var_number()
{
	uint8_t first_byte = read_byte();
	uint8_t second_byte = read_byte();
	uint32_t ret;
	if ((second_byte & 0xf) == 0x5) //TODO: THIS CANT BE RIGHT !
		ret = read_immediate_uint32();
	else ret = second_byte & 0x1f;
	return format_dword((ret << 4) + (first_byte & 0xf));
}

std::string mdil_decoder::format_helper_call() {
	auto helper = read_immediate_uint32();
	switch (helper)
	{
	case 0: return "MDIL_HELP_UNDEF";
	case 1: return "MDIL_HELP_DIV";
	case 2: return "MDIL_HELP_MOD";
	case 3: return "MDIL_HELP_UDIV";
	case 4: return "MDIL_HELP_UMOD";
	case 5: return "MDIL_HELP_LLSH";
	case 6: return "MDIL_HELP_LRSH";
	case 7: return "MDIL_HELP_LRSZ";
	case 8: return "MDIL_HELP_LMUL";
	case 9: return "MDIL_HELP_LMUL_OVF";
	case 10: return "MDIL_HELP_ULMUL_OVF";
	case 11: return "MDIL_HELP_LDIV";
	case 12: return "MDIL_HELP_LMOD";
	case 13: return "MDIL_HELP_ULDIV";
	case 14: return "MDIL_HELP_ULMOD";
	case 15: return "MDIL_HELP_LNG2DBL";
	case 16: return "MDIL_HELP_ULNG2DBL";
	case 17: return "MDIL_HELP_DBL2INT";
	case 18: return "MDIL_HELP_DBL2INT_OVF";
	case 19: return "MDIL_HELP_DBL2LNG";
	case 20: return "MDIL_HELP_DBL2LNG_OVF";
	case 21: return "MDIL_HELP_DBL2UINT";
	case 22: return "MDIL_HELP_DBL2UINT_OVF";
	case 23: return "MDIL_HELP_DBL2ULNG";
	case 24: return "MDIL_HELP_DBL2ULNG_OVF";
	case 25: return "MDIL_HELP_FLTREM";
	case 26: return "MDIL_HELP_DBLREM";
	case 27: return "MDIL_HELP_FLTROUND";
	case 28: return "MDIL_HELP_DBLROUND";
	case 29: return "MDIL_HELP_NEW_CROSSCONTEXT";
	case 30: return "MDIL_HELP_NEWFAST";
	case 31: return "MDIL_HELP_NEWSFAST";
	case 32: return "MDIL_HELP_NEWSFAST_ALIGN8";
	case 33: return "MDIL_HELP_NEW_MDARR";
	case 34: return "MDIL_HELP_STRCNS";
	case 35: return "MDIL_HELP_INITCLASS";
	case 36: return "MDIL_HELP_INITINSTCLASS";
	case 37: return "MDIL_HELP_ISINSTANCEOFINTERFACE";
	case 38: return "MDIL_HELP_ISINSTANCEOFARRAY";
	case 39: return "MDIL_HELP_ISINSTANCEOFCLASS";
	case 40: return "MDIL_HELP_CHKCASTINTERFACE";
	case 41: return "MDIL_HELP_CHKCASTARRAY";
	case 42: return "MDIL_HELP_CHKCASTCLASS";
	case 43: return "MDIL_HELP_CHKCASTCLASS_SPECIAL";
	case 44: return "MDIL_HELP_UNBOX_NULLABLE";
	case 45: return "MDIL_HELP_GETREFANY";
	case 46: return "MDIL_HELP_ARRADDR_ST";
	case 47: return "MDIL_HELP_LDELEMA_REF";
	case 48: return "MDIL_HELP_USER_BREAKPOINT";
	case 49: return "MDIL_HELP_RNGCHKFAIL";
	case 50: return "MDIL_HELP_OVERFLOW";
	case 51: return "MDIL_HELP_INTERNALTHROW";
	case 52: return "MDIL_HELP_VERIFICATION";
	case 53: return "MDIL_HELP_SEC_UNMGDCODE_EXCPT";
	case 54: return "MDIL_HELP_FAIL_FAST";
	case 55: return "MDIL_HELP_METHOD_ACCESS_EXCEPTION";
	case 56: return "MDIL_HELP_FIELD_ACCESS_EXCEPTION";
	case 57: return "MDIL_HELP_CLASS_ACCESS_EXCEPTION";
	case 58: return "MDIL_HELP_ENDCATCH";
	case 59: return "MDIL_HELP_MON_ENTER";
	case 60: return "MDIL_HELP_MON_EXIT";
	case 61: return "MDIL_HELP_MON_ENTER_STATIC";
	case 62: return "MDIL_HELP_MON_EXIT_STATIC";
	case 63: return "MDIL_HELP_GETCLASSFROMMETHODPARAM";
	case 64: return "MDIL_HELP_GETSYNCFROMCLASSHANDLE";
	case 65: return "MDIL_HELP_SECURITY_PROLOG";
	case 66: return "MDIL_HELP_SECURITY_PROLOG_FRAMED";
	case 67: return "MDIL_HELP_METHOD_ACCESS_CHECK";
	case 68: return "MDIL_HELP_FIELD_ACCESS_CHECK";
	case 69: return "MDIL_HELP_CLASS_ACCESS_CHECK";
	case 70: return "MDIL_HELP_DELEGATE_SECURITY_CHECK";
	case 71: return "MDIL_HELP_VERIFICATION_RUNTIME_CHECK";
	case 72: return "MDIL_HELP_STOP_FOR_GC";
	case 73: return "MDIL_HELP_POLL_GC";
	case 74: return "MDIL_HELP_STRESS_GC";
	case 75: return "MDIL_HELP_CHECK_OBJ";
	case 76: return "MDIL_HELP_ASSIGN_REF";
	// below may be wrong !!!
	case 77: return "MDIL_HELP_CHECKED_ASSIGN_REF";
	case 78: return "MDIL_HELP_ASSIGN_REF_ENSURE_NONHEAP";
	case 79: return "MDIL_HELP_ASSIGN_BYREF";
	case 80: return "MDIL_HELP_ASSIGN_STRUCT";
	case 81: return "MDIL_HELP_GETFIELD32";
	case 82: return "MDIL_HELP_SETFIELD32";
	case 83: return "MDIL_HELP_GETFIELD64";
	case 84: return "MDIL_HELP_SETFIELD64";
	case 85: return "MDIL_HELP_GETFIELDOBJ";
	case 86: return "MDIL_HELP_SETFIELDOBJ";
	case 87: return "MDIL_HELP_GETFIELDSTRUCT";
	case 88: return "MDIL_HELP_SETFIELDSTRUCT";
	case 89: return "MDIL_HELP_GETFIELDFLOAT";
	case 90: return "MDIL_HELP_SETFIELDFLOAT";
	case 91: return "MDIL_HELP_GETFIELDDOUBLE";
	case 92: return "MDIL_HELP_SETFIELDDOUBLE";
	case 93: return "MDIL_HELP_GETFIELDADDR";
	case 94: return "MDIL_HELP_GETSTATICFIELDADDR_CONTEXT";
	case 95: return "MDIL_HELP_GETSTATICFIELDADDR_TLS";
	case 96: return "MDIL_HELP_DBG_IS_JUST_MY_CODE";
	case 97: return "MDIL_HELP_PROF_FCN_ENTER";
	case 98: return "MDIL_HELP_PROF_FCN_LEAVE";
	case 99: return "MDIL_HELP_PROF_FCN_TAILCALL";
	case 100: return "MDIL_HELP_BBT_FCN_ENTER";
	case 101: return "MDIL_HELP_PINVOKE_CALLI";
	case 102: return "MDIL_HELP_TAILCALL";
	case 103: return "MDIL_HELP_GETCURRENTMANAGEDTHREADID";
	case 104: return "MDIL_HELP_INIT_PINVOKE_FRAME";
	case 105: return "MDIL_HELP_CHECK_PINVOKE_DOMAIN";
	case 106: return "MDIL_HELP_MEMSET";
	case 107: return "MDIL_HELP_MEMCPY";
	case 108: return "MDIL_HELP_RUNTIMEHANDLE_METHOD";
	case 109: return "MDIL_HELP_RUNTIMEHANDLE_METHOD_LOG";
	case 110: return "MDIL_HELP_RUNTIMEHANDLE_CLASS";
	case 111: return "MDIL_HELP_RUNTIMEHANDLE_CLASS_LOG";
	case 112: return "MDIL_HELP_TYPEHANDLE_TO_RUNTIMETYPEHANDLE";
	case 113: return "MDIL_HELP_METHODDESC_TO_RUNTIMEMETHODHANDLE";
	case 114: return "MDIL_HELP_FIELDDESC_TO_RUNTIMEFIELDHANDLE";
	case 115: return "MDIL_HELP_TYPEHANDLE_TO_RUNTIMETYPE";
	case 116: return "MDIL_HELP_METHODDESC_TO_STUBRUNTIMEMETHOD";
	case 117: return "MDIL_HELP_FIELDDESC_TO_STUBRUNTIMEFIELD";
	case 118: return "MDIL_HELP_VIRTUAL_FUNC_PTR";
	case 119: return "MDIL_HELP_EE_PRESTUB";
	case 120: return "MDIL_HELP_EE_PRECODE_FIXUP";
	case 121: return "MDIL_HELP_EE_PINVOKE_FIXUP";
	case 122: return "MDIL_HELP_EE_VSD_FIXUP";
	case 123: return "MDIL_HELP_EE_EXTERNAL_FIXUP";
	case 124: return "MDIL_HELP_EE_VTABLE_FIXUP";
	case 125: return "MDIL_HELP_EE_REMOTING_THUNK";
	// below are wrong !!
	case 126: return "MDIL_HELP_EE_PERSONALITY_ROUTINE";
	case 127: return "MDIL_HELP_ALLOCA";
	case 128: return "MDIL_HELP_EE_PERSONALITY_ROUTINE_FILTER_FUNCLET";
	default: return "unknown helper " + format_dword(helper);
	}

	return format_dword(helper);
}