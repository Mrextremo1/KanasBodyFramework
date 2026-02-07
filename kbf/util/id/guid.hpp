#pragma once

#include <cstdint>

namespace kbf {

	struct Guid {
		uint32_t Data1;
		uint16_t Data2;
		uint16_t Data3;
		uint8_t  Data4[8];
	};
	static_assert(sizeof(Guid) == 16, "Guid must be 16 bytes");

}