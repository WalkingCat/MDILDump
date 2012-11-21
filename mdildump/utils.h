#pragma once

class auto_file {
	FILE* file;
public:
	auto_file(const char* path, const char* mode) : file(nullptr) { fopen_s(&file, path, mode); }
	~auto_file() { fclose(file); }

	bool valid() { return file != nullptr; }
	long offset() { return ftell(file); }
	bool reset(long offset) { return fseek(file, offset, SEEK_SET) == 0; }
	unsigned char read1() { unsigned char ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	unsigned short read2() { unsigned short ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	unsigned int read4() { unsigned int ret = 0; fread_s(&ret, sizeof(ret), sizeof(ret), 1, file); return ret; }
	void skip(long bytes) { fseek(file, bytes, SEEK_CUR); }
	bool read(void* buffer, size_t size, size_t bytes) { return fread_s(buffer, size, bytes, 1, file) == bytes; }
	bool read(void* buffer, size_t size) { return read(buffer, size, size); }
};

// DO NOT COPY ! always take refs. its not shared_ptr
template<typename T> class auto_array {
	T* m_array;
	size_t m_count;
public:
	auto_array<T>() : m_array(nullptr), m_count(0) {  }
	auto_array<T>(size_t count) : m_count(count), m_array(new T[count]) {  }
	~auto_array<T>() { delete[] m_array; }
	operator T*() const { return m_array; }
	T* get() const { return m_array; }
	T& operator [](size_t index) { return m_array[index]; }
	void reset(size_t count) { delete[] m_array; m_count = count; m_array = new T[count]; }
	size_t count() const { return m_count; }
	size_t size() const { return m_count * sizeof(T); }
};

struct ulong_as_chars {
	char chars[5];
	ulong_as_chars(unsigned long ulong) {
		chars[0] = *((char*)&ulong + 3);
		chars[1] = *((char*)&ulong + 2);
		chars[2] = *((char*)&ulong + 1);
		chars[3] = *((char*)&ulong + 0);
		chars[4] = 0;
	}
};