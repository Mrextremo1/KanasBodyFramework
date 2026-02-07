#include <kbf/data/npc/npc_data_manager.hpp>

#include <kbf/util/re_engine/reinvoke.hpp>

#define NPC_ID_FETCH_CAP 1500

namespace kbf {

	NpcDataManager& NpcDataManager::get() {
		static NpcDataManager instance;
		if (!instance.initialized) instance.initialize();

		return instance;
	}

	void NpcDataManager::initialize() {
		if (initialized) return;

		// Note, this MUST go first.
		auto namedIds = getNpcNamedIDs();
		npcTypeToIdMappings = std::move(namedIds.first);
		idToNpcTypeMappings = std::move(namedIds.second);

		npcDataMappings = getNpcData();

		initialized = true;
	}

	NpcDataMap NpcDataManager::getNpcData() {
		NpcDataMap data{};

		static reframework::API::TypeDefinition* td_NpcID = reframework::API::get()->tdb()->find_type("app.NpcDef.ID");
		bool gotNumNpcIDs = false;
		const size_t numNpcIDs = REEnum(td_NpcID, "MAX", gotNumNpcIDs);
		if (!gotNumNpcIDs) {
			DEBUG_STACK.push("Failed to get number of NPC ID definitions!", DebugStack::Color::COL_ERROR);
			return data;
		}

		size_t cappedNumNpcIDs = std::min<size_t>(NPC_ID_FETCH_CAP, numNpcIDs);
		DEBUG_STACK.push(std::format("Attempting to fetch {} NPC datas", cappedNumNpcIDs));

		for (size_t i = 0; i < cappedNumNpcIDs; i++) {
			std::string npcName = REInvokeStaticStr("app.NpcUtil", "getNpcName(app.NpcDef.ID)", { (void*)i });
			if (npcName.empty()) continue;
			if (npcName[0] == '<') continue; // Reject names that start with '<', there's a bunch like <COLOR....>Rejected</COLOR>

			DEBUG_STACK.push(std::format("Fetching NPC Data for Idx {}: {}", i, npcName));

			data.emplace(i, NpcData{ i, npcName, false, getNpcTypeFromID(i) });
			
			// Note to self: app.NpcUtil has an interesting function: "isInViewCamera(via.vec3)" that we may be able to use.
		}

		return data;
	}

	bool NpcDataManager::isPartnerNpcID(const std::string& npcStrID) {
		return ALMA_NPC_IDS.contains(npcStrID) || GEMMA_NPC_IDS.contains(npcStrID) || ERIK_NPC_IDS.contains(npcStrID);
	}

	std::pair<NpcNamedTypeToIdsMap, NpcIdToNamedTypeMap> NpcDataManager::getNpcNamedIDs() {
		NpcNamedTypeToIdsMap toIds{};
		NpcIdToNamedTypeMap toName{};

		// Note, duplicate IDs here seem to be for DIFFERENT scenarios (i guess different instances of NPC behaviour).
		// E.g. For alma, stuff like, lobby, on quest, character creation screen etc.
		// TODO: Should probably verify these for cutscenes, etc.
		toIds[NpcType::NPC_TYPE_GENERIC] = {};
		toIds[NpcType::NPC_TYPE_ALMA]      = getValidIDsFromNpcPaths(std::vector(ALMA_NPC_IDS.begin(),  ALMA_NPC_IDS.end()));                  
		toIds[NpcType::NPC_TYPE_GEMMA]     = getValidIDsFromNpcPaths(std::vector(GEMMA_NPC_IDS.begin(), GEMMA_NPC_IDS.end())); 
		toIds[NpcType::NPC_TYPE_ERIK]      = getValidIDsFromNpcPaths(std::vector(ERIK_NPC_IDS.begin(),  ERIK_NPC_IDS.end()));                                   
		toIds[NpcType::NPC_TYPE_OLIVIA]    = getValidIDsFromNpcPaths({ "NPC102_00_007" });  // 9
		toIds[NpcType::NPC_TYPE_ROSSO]     = getValidIDsFromNpcPaths({ "NPC101_00_030" });  // 6
		toIds[NpcType::NPC_TYPE_ALESSA]    = getValidIDsFromNpcPaths({ "NPC112_00_021" });	// 27
		toIds[NpcType::NPC_TYPE_MINA]      = getValidIDsFromNpcPaths({ "NPC112_00_014" });  // 25
		toIds[NpcType::NPC_TYPE_KAI]       = getValidIDsFromNpcPaths({ "NPC111_00_016" });  // 17
		toIds[NpcType::NPC_TYPE_GRIFFIN]   = getValidIDsFromNpcPaths({ "NPC111_00_011" });  // 15
		toIds[NpcType::NPC_TYPE_NIGHTMIST] = getValidIDsFromNpcPaths({ "NPC112_00_024" });  // 29
		toIds[NpcType::NPC_TYPE_FABIUS]    = getValidIDsFromNpcPaths({ "NPC101_00_006" });  // 4
		toIds[NpcType::NPC_TYPE_NADIA]     = getValidIDsFromNpcPaths({ "NPC102_00_041" });  // 12

		// Reverse mapping
		for (const auto& [type, ids] : toIds) {
			for (const size_t id : ids) {
				toName[id] = type;
			}
		}

		return { toIds, toName };
	}

	std::unordered_set<size_t> NpcDataManager::getValidIDsFromNpcPaths(const std::vector<std::string>& npcPaths) {
		std::unordered_set<size_t> ids;

		static reframework::API::TypeDefinition* td_NpcID = reframework::API::get()->tdb()->find_type("app.NpcDef.ID");

		for (const std::string& pth : npcPaths) {
			bool fetched = false;
			const size_t id = REEnum(td_NpcID, pth, fetched);
			if (!fetched) continue;

			ids.insert(id);
		}

		return ids;
	}

}