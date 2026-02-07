#pragma once

#include <kbf/data/armour/armor_set_id.hpp>
#include <kbf/util/re_engine/reinvoke.hpp>
#include <kbf/util/re_engine/guid_to_string.hpp>
#include <kbf/util/re_engine/re_singleton.hpp>

#define ARMOUR_DATA_FETCH_CAP 1000

namespace kbf {

	// Look at natives\STM\GameDesign\Common\Equip ...\ArmorData.user.3 and ...\ArmorSeriesData.user.3 for mappings
	// we can probably extract this info at runtime to auto generate the mapping.

	enum ArmorSeriesModelVariety {
		INVALID = 0b00,
		MALE    = 0b01,
		FEMALE  = 0b10,
		BOTH    = 0b11
	};

	typedef uint8_t ArmorSeriesDisplayRank;
	enum ArmorSeriesDisplayRankFlags {
		RANK_NONE  = 0b000,
		RANK_ALPHA = 0b001,
		RANK_BETA  = 0b010,
		RANK_GAMMA = 0b100
	};

	struct ArmorSeriesData {
		std::string name;
		bool female;
		ArmorSeriesDisplayRank ranks = ArmorSeriesDisplayRankFlags::RANK_NONE;
	};

	struct NpcPrefabData {
		std::string name;
		bool femaleCanUse = false;
		bool maleCanUse = false;
	};

	// Note, two ArmorSetIDs can map to the same Armor due to alpha/beta sets.
	using ArmorSeriesIDMap   = std::unordered_map<ArmorSetID, ArmorSeriesData>;
	using NpcPrefabToArmorSetMap = std::unordered_map<std::string, NpcPrefabData>;

	class ArmourDataManager {
	public:
		static ArmourDataManager& get();

		void initialize();
		void uninitialize() { initialized = false; }
		void reinitialize() { uninitialize(); initialize(); }
		std::vector<ArmourSet> getFilteredArmourSets(const std::string& filter);
		ArmourSet getArmourSetFromArmourID(const ArmorSetID& setId) const;
		ArmourSet getArmourSetFromNpcPrefab(const std::string& npcPrefabPath, bool female) const;

	private:
		ArmourDataManager() = default;
		bool initialized = false;

		ArmorSeriesIDMap armourSeriesIDMappings;
		NpcPrefabToArmorSetMap npcPrefabToArmourSetMap;

		ArmorSeriesIDMap getArmorSeriesData();
		ArmorSeriesIDMap getHunterArmorData();
		ArmorSeriesIDMap getInnersArmorData();

		NpcPrefabToArmorSetMap getNpcArmorData();
		size_t getNpcArmorDataDefaultSelectors(REApi::ManagedObject* cNpcCatalogHolder, NpcPrefabToArmorSetMap& map, bool verbose = false);
		void getNpcArmorDataUniqueSelectors(REApi::ManagedObject* cNpcCatalogHolder, NpcPrefabToArmorSetMap& map);
		bool addPrefabToArmorSetMap(
			NpcPrefabToArmorSetMap& map, 
			const std::string& npcName,
			const std::string& npcStrID,
			const std::string& prefabPath, 
			size_t variant,
			bool female,
			bool verbose = false);

		static ArmorSeriesDisplayRank getArmorSeriesDisplayRank(const std::string& fullName);
		static std::string getArmorSeriesNameStem(const std::string& fullName);
		static void postProcessArmorSeriesData(ArmorSeriesIDMap& map);

		static std::string getPrefabFromVisualSetting(REApi::ManagedObject* visualSetting);

		RESingleton npcManager{ "app.NpcManager" };

		inline static constexpr FixedString LOG_TAG{ "[ArmourDataManager]" };
	};

}