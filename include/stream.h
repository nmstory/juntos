#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <include/config.h>

struct WriteStream {
	uint8_t* buf;
	size_t pos = 0;

	bool process_bytes(const void* src, size_t n) {
		if (pos + n > JUNTOS_MTU_BITS)
		{
			return false;
		}
		std::memcpy(buf + pos, src, n);
		pos += n;
		return true;
	}
};

struct ReadStream {
	const uint8_t* buf;
	size_t pos = 0;

	bool process_bytes(void* dst, size_t n) {
		if (pos + n > JUNTOS_MTU_BITS)
		{
			return false;
		}
		std::memcpy(dst, buf + pos, n);
		pos += n;
		return true;
	}
};

// process plain old data
template<typename Stream, typename T>
bool process_pod(Stream& s, T& value) {
	return s.process_bytes(&value, sizeof(T));
}
