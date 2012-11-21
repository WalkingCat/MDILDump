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
	default  : return "???";
	}
}

std::string mdil_decoder::format_immediate() {
	std::stringstream ret;
	ret.fill('0');
	ret.flags(ret.uppercase | ret.hex);

	unsigned char first_byte = m_buffer[m_pos++];
	switch (first_byte)
	{
	case 0xbb: // SignedWord
		ret << setw(4) << (short) (m_buffer[m_pos] + (m_buffer[m_pos+1] << 8));
		m_pos += 2;
		break;
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
	default: // SignedByte
		ret << setw(2) << (int) (char) first_byte;
		break;
	}

	return ret.str();
}

string mdil_decoder::read_native_quote (unsigned long length) {
	stringstream ss;
	ss.flags(ss.hex | ss.uppercase);
	ss.width(2);
	ss.fill('0');
	for (unsigned long i = 0; i < length; ++i) {
		ss << (int) m_buffer[m_pos++] << " ";
	}
	return ss.str();
}

std::vector<shared_ptr<mdil_instruction>> mdil_decoder::decode()
{
	std::vector<shared_ptr<mdil_instruction>> routine;
	stringstream ss;
	ss.flags(ss.uppercase | ss.hex);
	ss.fill('0');

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
		case 0x13: i->set("LOAD", format_address()); break;
		case 0x14: i->set("STORE", format_address()); break;
		case 0x16: i->set("LOAD_SX", format_address()); break;
		case 0x17: i->set("LOAD_ZX", format_address()); break;
		case 0x18: i->set("LOAD_X", format_address()); break;
		case 0x19: i->set("LOAD_ADDR", format_address()); break;
		case 0x1a: i->set("ADD", format_address()); break;
		case 0x1b: i->set("ADC", format_address()); break;
		case 0x1C: i->set("AND", format_address()); break;
		case 0x1D: i->set("CMP", format_address()); break;
		case 0x1E: i->set("OR", format_address()); break;
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
		case 0x30: i->set("STORE_IMM"); i->operands += format_address(); i->operands += ", " + format_immediate(); break;
		case 0x3f: i->set("COPY_STRUCT"); i->operands += "SRC:" + format_address(); i->operands += " DST:" + format_address(); break;
		case 0x40: i->set("PUSH_STRUCT", format_address()); break;
		case 0x49: i->set("CALL_INDIRECT", format_address_call_indirect()); break;
		case 0x4b: i->set("HELPER_CALL", format_immediate()); break;
		case 0x4d: i->set("CALL_DEF", format_dword(0x06000000 + read_word_le())); break;
		case 0x4e: i->set("CALL_REF", format_dword(0x0A000000 + read_word_le())); break;
		case 0x4f: i->set("CALL_VIRT_DEF", format_dword(0x06000000 + read_word_le())); break;
		case 0x50: i->set("CALL_VIRT_REF", format_dword(0x0A000000 + read_word_le())); break;
		case 0x6f: i->set("REF_BIRTH_EAX"); break;
		case 0x70: i->set("REF_BIRTH_ECX"); break;
		case 0x71: i->set("REF_BIRTH_EDX"); break;
		case 0x72: i->set("REF_BIRTH_EBX"); break;
		case 0x73: i->set("REF_BIRTH_REG", mdil_reg((read_byte() >> 5) & 0x1f) + " TODO: flags"); break;
		case 0x74: i->set("REF_BIRTH_EBP"); break;
		case 0x75: i->set("REF_BIRTH_ESI"); break;
		case 0x76: i->set("REF_BIRTH_EDI"); break;
		case 0x77: i->set("REF_DEATH_EAX"); break;
		case 0x78: i->set("REF_DEATH_ECX"); break;
		case 0x79: i->set("REF_DEATH_EDX"); break;
		case 0x7a: i->set("REF_DEATH_EBX"); break;
		case 0x7c: i->set("REF_DEATH_EBP"); break;
		case 0x7d: i->set("REF_DEATH_ESI"); break;
		case 0x7e: i->set("REF_DEATH_EDI"); break;
		case 0x8b: i->set("REF_UNTR_LOCAL", format_immediate()); break;
		case 0x97: i->set("REF_POP_N", format_immediate()); break;
		case 0x9e:
			i->opcode = "LOAD_STRING";
			i->operands = mdil_reg(read_byte()) + ", ";
			i->operands += format_dword(0x70000000 + read_word_be());
			break;
		case 0xa7: i->set("LOCAL_BLOCK", format_immediate()); break;
		case 0xa8: i->set("LOCAL_STRUCT", format_type_token()); break;
		case 0xaa: i->set("PARAM_BLOCK", format_immediate()); break;
		case 0xab: i->set("PARAM_STRUCT", format_type_token()); break;
		case 0xaf: i->set("INIT_VAR", format_immediate()); break;
		case 0xb1: i->set("ARG_COUNT", format_immediate()); break;
		case 0xb2: i->set("EBP_FRAME"); break;
		case 0xb3: i->set("DOUBLE_ALIGN_ESP"); break;
		case 0xb4: {
			i->opcode = "PUSH_REGS";
			unsigned char regs = read_byte();
			if (regs & 1) ss << "EBX,";
			if (regs & (1 << 1)) ss << "ESI,";
			if (regs & (1 << 2)) ss << "EDI,";
			if (regs & (1 << 3)) ss << "EBP,";
			if (regs & (1 << 4)) ss << "R12,";
			if (regs & (1 << 5)) ss << "R13,";
			if (regs & (1 << 6)) ss << "R14,";
			if (regs & (1 << 7)) ss << "R15,";
			i->operands = ss.str();
			break;
		}
		case 0xb8: i->set("FRAME_SIZE", format_immediate()); break;
		case 0xb9: i->set("END_PROLOG"); break;
		case 0xbb: i->set("EPILOG_RET"); break;
		case 0xc5: i->set("GENERIC_LOOKUP", format_dword(read_dword_le())); break;
		case 0xd0: i->set("PRESERVE_REGISTER_ACROSS_PROLOG"); break;
		case 0xda: i->set("UNKNOWN_DA", format_byte(read_byte())); break;
		case 0xdf: i->set("UNKNOWN_DF", format_byte(read_byte())); break;
		case 0xe4: i->set("UNKNOWN_E4"); break;
		case 0xe5: i->set("UNKNOWN_E5"); break;
		case 0xeb: i->set("UNKNOWN_EB"); break;
		case 0xec: i->set("UNKNOWN_EC"); break;
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

std::string mdil_decoder::format_dword( unsigned long val )
{
	stringstream s;
	s << uppercase << hex << setfill('0') << setw(8) << val;
	return s.str();
}

std::string mdil_decoder::format_type_token()
{
	unsigned char first_byte = read_byte();
	if (first_byte == 0xff)	return format_dword(read_dword_le());
	else {
		if (first_byte & 0xf0) return format_dword(0x1b000000 + ((first_byte - 0xf0) << 8) + read_byte());
		else if (first_byte & 0xc0) return format_dword(0x01000000 + ((first_byte - 0xf0) << 8) + read_byte());
		else return format_dword(0x02000000 + ((first_byte - 0xf0) << 8) + read_byte());
	}
}

unsigned char mdil_decoder::read_addr_regs_byte( unsigned char& op_reg, unsigned char& base_reg, unsigned char& flags )
{
	unsigned char addr_regs_byte = read_byte();
	if (true) { // x86
		op_reg = (addr_regs_byte >> 3) & 0x7;
		base_reg = addr_regs_byte & 0x7;
		flags = (addr_regs_byte >> 6) & 0x3;
	}
	return addr_regs_byte;
}

std::string mdil_decoder::format_address()
{
	string ret;
	if (true) { // x86
		unsigned char op_reg, base_reg, flags;
		read_addr_regs_byte(op_reg, base_reg, flags);
		ret += mdil_reg(op_reg) + ", [" + mdil_reg(base_reg) + "]";
		if (flags == 0x00) { // AF_FIELD
			ret += "." + format_field_token();
		} else if (flags == 0x03) {
			ret += "(";
			unsigned char modifier;
			do {
				modifier = read_byte();
				ret += format_byte(modifier);
			} while (modifier & 0x80);
			ret += ")";
		} else ret += ".TODO";
	}
	return ret;
}


std::string mdil_decoder::format_address_call_indirect()
{
	string ret;
	if (true) { // x86
		unsigned char op_reg, base_reg, flags;
		read_addr_regs_byte(op_reg, base_reg, flags);
		ret += "[" + mdil_reg(base_reg) + "]";
		if (flags == 0x00) { // AF_FIELD
			ret += "." + format_field_token();
		} else if (flags == 0x03) {
			ret += "(";
			unsigned char modifier;
			do {
				modifier = read_byte();
				ret += format_byte(modifier & 0x7f);
			} while (modifier & 0x80);
			ret += ")";
		} else ret += ".TODO";
		if (op_reg == 1) { // CIF_METHOD_TOKEN
			ret += "." + format_method_token(read_dword_le());
		}
	}
	return ret;
}

std::string mdil_decoder::format_field_token()
{
	return format_dword(0x04000000 + read_word_be());
}

std::string mdil_decoder::format_method_token( unsigned long val )
{
	return format_dword(val);
}
