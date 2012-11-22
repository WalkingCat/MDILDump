#pragma once

#include "mdil_data.h"

class mdil_decoder {
	const unsigned char* m_buffer;
	const unsigned long m_length;
	unsigned long m_pos;
	bool m_error;
	char read_signed_byte();
	unsigned char read_byte();
	unsigned short read_word_le();
	unsigned short read_word_be();
	unsigned long read_dword_le();
	std::string read_native_quote (unsigned long length);
	static std::string format_byte(unsigned char val);
	static std::string format_dword(unsigned long val);
	std::string format_type_token();
	std::string format_method_token(unsigned long val);
	std::string format_immediate();
	unsigned char read_addr_regs_byte(unsigned char& op_reg, unsigned char& base_reg, unsigned char& flags);
	std::string format_address();
	std::string format_address_call_indirect();
	std::string format_field_token();
	std::string format_jump_distance(bool jump_long = false);
public:
	mdil_decoder(const unsigned char* buffer, const unsigned long length);
	std::vector<std::shared_ptr<mdil_instruction>> decode();
};

