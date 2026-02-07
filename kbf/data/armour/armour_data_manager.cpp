#include <kbf/data/armour/armour_data_manager.hpp>

#include <kbf/data/npc/npc_prefab_alias_mappings.hpp>
#include <kbf/data/npc/npc_data_manager.hpp>

#include <kbf/util/string/to_lower.hpp>
#include <kbf/data/ids/special_armour_ids.hpp>
#include <kbf/util/re_engine/print_re_object.hpp>

#include <unordered_set>
#include <set>
#include <numeric>

#define NPC_ARMOR_PREFAB_FETCH_CAP 2000
#define NPC_UNIQUE_PREFAB_SETS_FETCH_CAP 4
#define NPC_UNIQUE_PREFAB_VARIANTS_FETCH_CAP 1000

namespace kbf {

	ArmourDataManager& ArmourDataManager::get() {
		static ArmourDataManager instance;
		if (!instance.initialized) instance.initialize();

		return instance;
	}

	void ArmourDataManager::initialize() {
		if (initialized) return;

		armourSeriesIDMappings = getArmorSeriesData();
		npcPrefabToArmourSetMap = getNpcArmorData();
		initialized = true;
	}

	std::vector<ArmourSet> ArmourDataManager::getFilteredArmourSets(const std::string& filter) {
		const bool noFilter = filter.empty();
		std::string filterLower = toLower(filter);

		std::set<ArmourSet> filteredSets;
		// Hunter Sets
		for (const auto& [id, data] : armourSeriesIDMappings) {
			std::string setLower = toLower(data.name);

			if (noFilter || setLower.find(filterLower) != std::string::npos) {
				filteredSets.insert(getArmourSetFromArmourID(id));
			}
		}

		// NPC Sets
		for (const auto& [prefabPth, data] : npcPrefabToArmourSetMap) {
			std::string setLower = toLower(data.name);

			if (noFilter || setLower.find(filterLower) != std::string::npos) {
				if (data.femaleCanUse) filteredSets.insert(getArmourSetFromNpcPrefab(prefabPth, true));
				if (data.maleCanUse)   filteredSets.insert(getArmourSetFromNpcPrefab(prefabPth, false));
			}
		}

		// TODO: This is tragically inefficient
		std::vector<ArmourSet> sortedSets(filteredSets.begin(), filteredSets.end());
		std::sort(sortedSets.begin(), sortedSets.end(),
			[](const ArmourSet& a, const ArmourSet& b) {
				if (a.name == ANY_ARMOUR_ID) return true;
				if (b.name == ANY_ARMOUR_ID) return false;
				int cmp = strcmp(a.name.c_str(), b.name.c_str());
				if (cmp == 0) return !a.female;
				return cmp < 0; // a.name < b.name
			});

		return sortedSets;
	}

	ArmourSet ArmourDataManager::getArmourSetFromArmourID(const ArmorSetID& setId) const {
		const auto& it = armourSeriesIDMappings.find(setId);
		if (it != armourSeriesIDMappings.end()) {
			return ArmourSet{ it->second.name, it->second.female };
		}

		return ArmourSet::DEFAULT;
	}

	ArmourSet ArmourDataManager::getArmourSetFromNpcPrefab(const std::string& npcPrefabPath, bool female) const {
		const auto& it = npcPrefabToArmourSetMap.find(npcPrefabPath);
		if (it != npcPrefabToArmourSetMap.end()) {
			if (female && it->second.femaleCanUse) return ArmourSet{ it->second.name, true };
			if (!female && it->second.maleCanUse)  return ArmourSet{ it->second.name, false };
		}

		return ArmourSet::DEFAULT;
	}

	ArmorSeriesIDMap ArmourDataManager::getArmorSeriesData() {
		// Hunter Armors
		ArmorSeriesIDMap hunterArmors = getHunterArmorData();
		ArmorSeriesIDMap innerArmors = getInnersArmorData();

		// coalesce
		hunterArmors.insert(innerArmors.begin(), innerArmors.end());

		// Post process names
		postProcessArmorSeriesData(hunterArmors);

		return hunterArmors;
	}

	ArmorSeriesIDMap ArmourDataManager::getHunterArmorData() {
		ArmorSeriesIDMap map{};

		// Hunter Armour Sets
		static reframework::API::TypeDefinition* td_ArmorSeries = reframework::API::get()->tdb()->find_type("app.ArmorDef.SERIES");
		bool gotNumSeries = false;
		const size_t numSeries = REEnum(td_ArmorSeries, "MAX", gotNumSeries);
		if (!gotNumSeries) {
			DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_ERROR, "Failed to get number of Armor Series definitions!");
			return map;
		}

		size_t cappedNumSeries = std::min<size_t>(ARMOUR_DATA_FETCH_CAP, numSeries);
		DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_INFO, "Attempting to fetch {} armor series", cappedNumSeries);

		for (size_t i = 1; i < cappedNumSeries; i++) {
			std::string armorSeriesName = REInvokeGuidStatic("app.ArmorDef", "Name(app.ArmorDef.SERIES)", { (void*)i }, LocalizationLanguage::English);
			if (armorSeriesName == "-") continue;

			ArmorSeriesDisplayRank ranks = getArmorSeriesDisplayRank(armorSeriesName);

			int rawVariety = REInvokeStatic<int>("app.ArmorDef", "ModelVariety(app.ArmorDef.SERIES)", { (void*)i }, InvokeReturnType::DWORD);
			ArmorSeriesModelVariety variety = ArmorSeriesModelVariety::INVALID;
			switch (rawVariety) {
			case 0: variety = ArmorSeriesModelVariety::BOTH;   break;
			case 1: variety = ArmorSeriesModelVariety::MALE;   break;
			case 2: variety = ArmorSeriesModelVariety::FEMALE; break;
			}

			uint32_t modId = REInvokeStatic<uint32_t>("app.ArmorDef", "ModId(app.ArmorDef.SERIES)", { (void*)i }, InvokeReturnType::DWORD);
			auto handleInsertVariantMapping = [&](ArmorSeriesModelVariety flag, const char* subIdFuncSignature, bool isFemale) {
				if (!(variety & flag)) return;

				uint32_t modSubId = REInvokeStatic<uint32_t>("app.ArmorDef", subIdFuncSignature, { (void*)i }, InvokeReturnType::DWORD);
				ArmorSetID setId{ modId, modSubId };

				std::string seriesStem = getArmorSeriesNameStem(armorSeriesName);

				// Try insert, if already exists then update ranks
				auto [it, inserted] = map.try_emplace(
					setId,
					ArmorSeriesData{ seriesStem, isFemale, ranks }
				);

				if (!inserted) {
					it->second.ranks |= ranks;
				}
			};

			handleInsertVariantMapping(variety, "ModSubMaleId(app.ArmorDef.SERIES)", false);
			handleInsertVariantMapping(variety, "ModSubFemaleId(app.ArmorDef.SERIES)", true);

			DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_SUCCESS, "Fetched Armor Series Data for Idx {}: {}", i, armorSeriesName);
		}

		return map;
	}

	ArmorSeriesIDMap ArmourDataManager::getInnersArmorData() {
		ArmorSeriesIDMap map{};

		// Inner Sets
		static reframework::API::TypeDefinition* td_InnerStyle = reframework::API::get()->tdb()->find_type("app.characteredit.Definition.INNER_STYLE");
		bool gotNumInners = false;
		size_t numInners = REEnum(td_InnerStyle, "MAX", gotNumInners);
		if (!gotNumInners) {
			DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_ERROR, "Failed to get number of Inner Armour definitions!");
			return map;
		}
		DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_INFO, "Attempting to fetch {} inners", numInners);

		for (size_t i = 0; i < numInners; i++) {
			std::string innerRawName = REInvokeGuidStatic("app.ArmorUtil", "getInnerStyleName(app.characteredit.Definition.INNER_STYLE)", { (void*)i }, LocalizationLanguage::English);
			if (innerRawName == "-") continue;

			DEBUG_STACK.fpush<LOG_TAG>("Attempting to fetch inner Data for Idx {}: {}", i, innerRawName);

			// trim off ending <ICON_EQUIP_TYPE1> or <ICON_EQUIP_TYPE2> for gender
			constexpr std::string_view MALE_ICON   = "<ICON EQUIP_TYPE1>";
			constexpr std::string_view FEMALE_ICON = "<ICON EQUIP_TYPE2>";
			std::string innerName = innerRawName;
			bool female = false;
			if (innerName.ends_with(MALE_ICON)) {
				innerName.resize(innerName.size() - MALE_ICON.size());
			}
			else if (innerName.ends_with(FEMALE_ICON)) {
				innerName.resize(innerName.size() - FEMALE_ICON.size());
				female = true;
			}

			// ID and ranks
			ArmorSetID innerID = REInvokeStatic<ArmorSetID>("app.ArmorUtil", "getArmorSetIDFromInnerStyle(app.characteredit.Definition.INNER_STYLE)", { (void*)i }, InvokeReturnType::WORD);
			ArmorSeriesDisplayRank ranks = getArmorSeriesDisplayRank(innerName);
			std::string innerStem = getArmorSeriesNameStem(innerName);

			map.emplace(innerID, ArmorSeriesData{ innerStem, female, ranks });

			DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_SUCCESS, "Fetched Inner Armour Set Idx {}: {}", i, innerName);
		}

		return map;
	}

	NpcPrefabToArmorSetMap ArmourDataManager::getNpcArmorData() {
		NpcPrefabToArmorSetMap map{};

		DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_INFO, "Attempting to fetch NPC Armor Data...");

		REApi::ManagedObject* cNpcCatalogHolder = REInvokePtr<REApi::ManagedObject>(npcManager.get(), "get_Catalog()", {});
		if (cNpcCatalogHolder == nullptr) {
			DEBUG_STACK.push("Failed to get NPC Catalog from NpcManager!", DebugStack::Color::COL_ERROR);
			return {};
		}

		// pass this twice to get an accurate count of how many NPCs were ignored
		size_t ignoredCnt = getNpcArmorDataDefaultSelectors(cNpcCatalogHolder, map, false);
		ignoredCnt        = getNpcArmorDataDefaultSelectors(cNpcCatalogHolder, map, true);

		getNpcArmorDataUniqueSelectors(cNpcCatalogHolder, map);

		DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_SUCCESS, "Successfully aliased {} NPC prefabs to armor sets!", map.size());
		if (ignoredCnt > 0) {
			DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_WARNING, "Ignored {} NPC prefabs which had no aliases defined.", ignoredCnt);
		}

		//app.user_data.NpcHunterEquipData.cArmorData

		return map;
	}

	size_t ArmourDataManager::getNpcArmorDataDefaultSelectors(REApi::ManagedObject* cNpcCatalogHolder, NpcPrefabToArmorSetMap& map, bool verbose) {
		size_t ignoredCount = 0;

		// 1. DEFAULT Selectors NpcVisualSettings
		for (size_t i = 0; i < NPC_ARMOR_PREFAB_FETCH_CAP; i++) {
			REApi::ManagedObject* NpcResidentPackage = REInvokePtr<REApi::ManagedObject>(cNpcCatalogHolder, "getResidentData(System.Int32)", { (void*)i });
			if (NpcResidentPackage == nullptr) continue;

			// Get name of NPC from System.Enum.GetName using index i
			static REApi::ManagedObject* td_NpcDefID = REApi::get()->typeof("app.NpcDef.ID");
			REApi::ManagedObject* boxedEnumValue = REInvokeStaticPtr<REApi::ManagedObject>("System.Enum", "InternalBoxEnum(System.RuntimeType, System.Int64)", { (void*)td_NpcDefID, (void*)i });
			std::string npcStrID = REInvokeStaticStr("System.Enum", "GetName(System.Type, System.Object)", { (void*)td_NpcDefID, (void*)boxedEnumValue });
			std::string npcName = NpcDataManager::get().getNpcNameFromID(i);

			REApi::ManagedObject* NpcVisualBase = REFieldPtr<REApi::ManagedObject>(NpcResidentPackage, "_VisualSetting");
			if (NpcVisualBase == nullptr) continue;

			static REApi::TypeDefinition* td_NpcVisualSetting = REApi::get()->tdb()->find_type("app.user_data.NpcVisualSetting");
			static REApi::TypeDefinition* td_NpcVisualSelector = REApi::get()->tdb()->find_type("app.user_data.NpcVisualSelector");
			REApi::TypeDefinition* td_visualSetting = NpcVisualBase->get_type_definition();
			if (td_visualSetting == nullptr) continue;

			// No Variants
			if (td_visualSetting == td_NpcVisualSetting) {
				REApi::ManagedObject* NpcVisualSetting = REInvokePtr<REApi::ManagedObject>(NpcResidentPackage, "get_VisualSetting()", {});
				std::string prefabPath = getPrefabFromVisualSetting(NpcVisualSetting);
				if (prefabPath.empty()) continue;

				size_t species = REInvoke<size_t>(NpcVisualSetting, "get_Species()", {}, InvokeReturnType::DWORD);
				if (species > 1) continue; // non human npc

				size_t gender = REInvoke<size_t>(NpcVisualSetting, "get_Gender()", {}, InvokeReturnType::DWORD);

				bool added = addPrefabToArmorSetMap(map, npcName, npcStrID, prefabPath, 0, gender == 1, verbose);
				if (!added) ignoredCount++;
			}
			// Variants
			else if (td_visualSetting == td_NpcVisualSelector) {
				REApi::ManagedObject* selector = REFieldPtr<REApi::ManagedObject>(NpcVisualBase, "_Selector");
				if (selector == nullptr) continue;

				const REApi::TypeDefinition* td_selector = selector->get_type_definition();
				const std::vector<REApi::Field*> selector_fields = td_selector->get_fields();

				for (size_t variantIdx = 0; variantIdx < selector_fields.size(); variantIdx++) {
					auto& field = selector_fields[variantIdx];

					if (field == nullptr) continue;
					if (field->get_type() != td_NpcVisualSetting) continue;

					REApi::ManagedObject* NpcVisualSetting = REFieldPtr<REApi::ManagedObject>(selector, field->get_name());
					std::string prefabPath = getPrefabFromVisualSetting(NpcVisualSetting);
					if (prefabPath.empty()) continue;

					size_t species = REInvoke<size_t>(NpcVisualSetting, "get_Species()", {}, InvokeReturnType::DWORD);
					if (species > 1) continue; // non human npc

					size_t gender = REInvoke<size_t>(NpcVisualSetting, "get_Gender()", {}, InvokeReturnType::DWORD);

					bool added = addPrefabToArmorSetMap(map, npcName, npcStrID, prefabPath, variantIdx, gender == 1, verbose);
					if (!added) ignoredCount++;
				}
			}

		}

		// Do a quick pass to filter out any sets we couldn't resolve
		std::unordered_set<std::string> mappingsToRemove{};
		for (const auto& [pth, data] : map) {
			if (data.name.empty() || (!data.femaleCanUse && !data.maleCanUse)) mappingsToRemove.insert(pth);
		}
		for (const std::string& pth : mappingsToRemove) {
			map.erase(pth);
		}

		return ignoredCount;
	}

	void ArmourDataManager::getNpcArmorDataUniqueSelectors(REApi::ManagedObject* cNpcCatalogHolder, NpcPrefabToArmorSetMap& map) {
		// TODO: Make sure to OVERWRITE any existing entries here as the non unique selectors before this pass are not reliable.

		// 2. UNIQUE visuals 
		//  Note: app.NpcDef.UNIQUE_VISUAL_Fixed mappings:
		//   2: Alma
		//   3: Erik
		//   4: Gemma
		// Update Note: If any other partners are added, you'll have to specifying their mappings here
		for (size_t i = 2; i <= NPC_UNIQUE_PREFAB_SETS_FETCH_CAP; i++) {
			for (size_t j = 0; j < NPC_UNIQUE_PREFAB_VARIANTS_FETCH_CAP; j++) {
				REApi::ManagedObject* NpcVisualSetting = REInvokePtr<REApi::ManagedObject>(cNpcCatalogHolder, "getCustomVari(app.NpcDef.UNIQUE_VISUAL_Fixed, System.Int32)", { (void*)i, (void*)j });
				std::string prefabPath = getPrefabFromVisualSetting(NpcVisualSetting);
				if (prefabPath.empty()) continue;

				DEBUG_STACK.fpush<LOG_TAG>("NPC UNIQUE_VISUAL_FIXED Idx {}-{} has prefab path: {}", i, j, prefabPath);
			}
		}
	}

	bool ArmourDataManager::addPrefabToArmorSetMap(
		NpcPrefabToArmorSetMap& map,
		const std::string& npcName,
		const std::string& npcStrID,
		const std::string& prefabPath,
		size_t variant,
		bool female,
		bool verbose
	) {
		// TODO: Ignore partner NPCs here to not generate shit mappings

		if (prefabPath.empty()) return false; // No prefab to map
		if (npcName == "Amone") {
			int test = 1;
		}

		// A subset of npcs will be chosen to generate mappings from.
		std::string prefabAlias = NpcPrefabAliasMappings::getPrefabAlias(npcStrID, variant);
		// If we don't find a mapping, no issue, just TRY and find a matching prefab later cataloged by a seed NPC

		auto it = map.find(prefabPath);
		if (it != map.end()) {
			// Resolve Collision - Another NPC already mapped it, so just potentially update the gender that can use it, so long as the name matches.
			if (!prefabAlias.empty() && !it->second.name.empty() && prefabAlias != it->second.name) {
				if (verbose) DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_WARNING, "Prefab collision detected when mapping alias for NPC ({}, {}, [{}]): Old: {} | New: {}. This NPC's prefab alias will be ignored.", npcStrID, npcName, female ? "F" : "M", it->second.name, prefabAlias);
				return false;
			}
			else {
				// update the gender
				if (!prefabAlias.empty()) it->second.name = prefabAlias;
				it->second.femaleCanUse |= female;
				it->second.maleCanUse |= !female;
			}
		}
		else {
			// Add new entry
			NpcPrefabData newData{ prefabAlias, female, !female };
			map.emplace(prefabPath, newData);
		}

		if (verbose) {
			if (map.at(prefabPath).name.empty()) {
				DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_WARNING, "Pushed an empty NPC Prefab Mapping: ({}, {} [{}]) {} [{}] - Variant {}", npcStrID, npcName, female ? "F" : "M", map.at(prefabPath).name, prefabPath, variant);
			}
			else {
				DEBUG_STACK.fpush<LOG_TAG>(DebugStack::Color::COL_SUCCESS, "Loaded NPC Prefab Mapping: ({}, {} [{}]) {} [{}] - Variant {}", npcStrID, npcName, female ? "F" : "M", map.at(prefabPath).name, prefabPath, variant);
			}
		}

		return !map.at(prefabPath).name.empty();
	}

	ArmorSeriesDisplayRank ArmourDataManager::getArmorSeriesDisplayRank(const std::string& fullName) {
		constexpr const char* alphaChrU8 = "\xCE\xB1"; // α
		constexpr const char* betaChrU8  = "\xCE\xB2"; // β
		constexpr const char* gammaChrU8 = "\xCE\xB3"; // γ
		ArmorSeriesDisplayRank ranks = ArmorSeriesDisplayRankFlags::RANK_NONE;
		if (fullName.ends_with(alphaChrU8))      ranks |= ArmorSeriesDisplayRankFlags::RANK_ALPHA;
		else if (fullName.ends_with(betaChrU8))  ranks |= ArmorSeriesDisplayRankFlags::RANK_BETA;
		else if (fullName.ends_with(gammaChrU8)) ranks |= ArmorSeriesDisplayRankFlags::RANK_GAMMA;

		return ranks;
	}

	std::string ArmourDataManager::getArmorSeriesNameStem(const std::string& fullName) {
		// Remove any trailing α/β/γ symbols
		constexpr const char* alphaChrU8 = "\xCE\xB1"; // α
		constexpr const char* betaChrU8  = "\xCE\xB2"; // β
		constexpr const char* gammaChrU8 = "\xCE\xB3"; // γ

		// Note: these characters are 2 bytes, but also trim an extra byte for trailing space.
		if (fullName.ends_with(alphaChrU8)) {
			return fullName.substr(0, fullName.size() - 3);
		}
		else if (fullName.ends_with(betaChrU8)) {
			return fullName.substr(0, fullName.size() - 3);
		}
		else if (fullName.ends_with(gammaChrU8)) {
			return fullName.substr(0, fullName.size() - 3);
		}
		return fullName;
	}

	void ArmourDataManager::postProcessArmorSeriesData(ArmorSeriesIDMap& map) {
		// Update names to include correct display ranks.
		// I.e. 
		// RANK_NONE  = Armor Set
		// RANK_ALPHA = Armor Set 0
		// RANK_ALPHA | RANK_BETA = Armor Set 0/1
		// RANK_ALPHA | RANK_BETA | RANK_GAMMA = Armor Set 0/1/2

		for (auto& [id, data] : map) {
			std::string rankSuffix = "";
			if (data.ranks == ArmorSeriesDisplayRankFlags::RANK_NONE) {
				// No suffix
				continue;
			}
			else {
				std::vector<std::string> rankParts;
				if (data.ranks & ArmorSeriesDisplayRankFlags::RANK_ALPHA) rankParts.push_back("0");
				if (data.ranks & ArmorSeriesDisplayRankFlags::RANK_BETA)  rankParts.push_back("1");
				if (data.ranks & ArmorSeriesDisplayRankFlags::RANK_GAMMA) rankParts.push_back("2");
				rankSuffix = " " + std::accumulate(std::next(rankParts.begin()), rankParts.end(), rankParts[0],
					[](std::string a, std::string b) { return a + "/" + b; });
			}
			data.name = data.name + rankSuffix;
		}

	}

	std::string ArmourDataManager::getPrefabFromVisualSetting(REApi::ManagedObject* visualSetting) {
		if (visualSetting == nullptr) return "";

		REApi::ManagedObject* cNpcBaseModelData = REInvokePtr<REApi::ManagedObject>(visualSetting, "get_ModelData()", {});
		if (cNpcBaseModelData == nullptr) return "";

		REApi::ManagedObject* cBaseModelInfo = REInvokePtr<REApi::ManagedObject>(cNpcBaseModelData, "get_ModelInfo()", {});
		if (cBaseModelInfo == nullptr) return "";

		REApi::ManagedObject* prefab = REInvokePtr<REApi::ManagedObject>(cBaseModelInfo, "get_Prefab()", {});
		if (prefab == nullptr) return "";

		std::string prefabPath = REInvokeStr(prefab, "get_Path()", {});
		return prefabPath;
	}

}