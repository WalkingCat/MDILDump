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
	std::string format_const_data (unsigned long length);
	std::string read_native_quote (unsigned long length);
	std::string format_byte();
	static std::string format_byte(unsigned char val);
	std::string format_dword();
	static std::string format_dword(unsigned long val);
	std::string format_type_token();
	std::string format_method_token();
	std::string format_string_token();
	std::string format_method_token(unsigned long val);
	std::string format_immediate();
	std::string format_address_modifier(uint8_t modifier, const char* str, bool& bracketed);
	std::string format_address_base(uint8_t base_reg, uint8_t flags);
	std::string format_address(std::function<std::string(uint8_t, uint8_t, uint8_t)> formatter = nullptr);
	std::string format_address_no_reg();
	std::string format_field_token();
	std::string format_jump_distance(bool jump_long = false);
public:
	mdil_decoder(const unsigned char* buffer, const unsigned long length);
	std::vector<std::shared_ptr<mdil_instruction>> decode();
};

