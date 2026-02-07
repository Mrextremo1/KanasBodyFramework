#pragma once

#include <kbf/data/armour/armour_set.hpp>

#include <unordered_map>

namespace kbf {

	struct ArmorSetID {
		uint32_t id;
		uint32_t subId;

		bool operator==(const ArmorSetID& other) const noexcept {
			return id == other.id && subId == other.subId;
		}
	};

	struct WholeBodyArmorSetID {
		ArmorSetID helm;
		ArmorSetID body;
		ArmorSetID arms;
		ArmorSetID coil;
		ArmorSetID legs;
	};

}

namespace std {
	template <>
	struct hash<kbf::ArmorSetID> {
		size_t operator()(const kbf::ArmorSetID& v) const noexcept {
			size_t h1 = std::hash<uint32_t>{}(v.id);
			size_t h2 = std::hash<uint32_t>{}(v.subId);

			// standard hash combine
			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};
}