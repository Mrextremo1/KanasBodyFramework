#pragma once

#include <kbf/data/npc/npc_type.hpp>
#include <kbf/debug/debug_stack.hpp>

#include <unordered_map>
#include <unordered_set>
#include <string>

namespace kbf {

	using NpcNamedTypeToIdsMap = std::unordered_map<NpcType, std::unordered_set<size_t>>;
	using NpcIdToNamedTypeMap  = std::unordered_map<size_t, NpcType>;

	struct NpcData {
		size_t id;
		std::string name;
		bool female;
		NpcType type = NpcType::NPC_TYPE_UNKNOWN;
	};
	using NpcDataMap = std::unordered_map<size_t, NpcData>;

	class NpcDataManager {
	public:
		static NpcDataManager& get();

		std::unordered_set<size_t> getNamedNpcIDs(NpcType type) const { 
			return npcTypeToIdMappings.contains(type)
				? npcTypeToIdMappings.at(type)
				: std::unordered_set<size_t>{}; 
		}

		NpcType getNpcTypeFromID(size_t id) const {
			return idToNpcTypeMappings.contains(id)
				? idToNpcTypeMappings.at(id)
				: NpcType::NPC_TYPE_UNKNOWN;
		}

		std::string getNpcNameFromID(size_t id) const {
			return npcDataMappings.contains(id)
				? npcDataMappings.at(id).name
				: "";
		}

		bool isFemaleNpc(size_t id) const {
			return npcDataMappings.contains(id)
				? npcDataMappings.at(id).female
				: false;
		}

		void initialize();
		void uninitialize() { initialized = false; }
		void reinitialize() { uninitialize(); initialize(); }

		static bool isPartnerNpcID(const std::string& npcStrID);

	private:
		NpcDataManager() = default;
		bool initialized = false;

		NpcDataMap npcDataMappings;
		NpcDataMap getNpcData();

		inline const static std::unordered_set<std::string> ALMA_NPC_IDS  = { "NPC102_00_001", "NPC102_00_906", "NPC112_50_021" };					 // 8,  13,  529
		inline const static std::unordered_set<std::string> GEMMA_NPC_IDS = { "NPC102_00_010", "NPC112_50_009", "NPC112_50_012", "NPC112_50_022" };	 // 10, 527, 528, 530
		inline const static std::unordered_set<std::string> ERIK_NPC_IDS  = { "NPC101_00_002", "NPC111_50_023" };									 // 0,  525

		NpcNamedTypeToIdsMap npcTypeToIdMappings;
		NpcIdToNamedTypeMap idToNpcTypeMappings;
		std::pair<NpcNamedTypeToIdsMap, NpcIdToNamedTypeMap> getNpcNamedIDs();
		std::unordered_set<size_t> getValidIDsFromNpcPaths(const std::vector<std::string>& npcPaths);

		inline static constexpr FixedString LOG_TAG{ "[NpcDataManager]" };
	};

}