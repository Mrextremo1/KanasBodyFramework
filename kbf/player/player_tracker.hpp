#pragma once

#include <kbf/data/kbf_data_manager.hpp>
#include <kbf/player/player_cache.hpp>
#include <kbf/player/player_info.hpp>
#include <kbf/player/persistent_player_info.hpp>
#include <kbf/player/player_fetch_flags.hpp>
#include <kbf/situation/lobby_type.hpp>
#include <kbf/situation/situation_watcher.hpp>
#include <kbf/enums/armor_parts.hpp>

#include <unordered_map>
#include <unordered_set>

#include <kbf/util/re_engine/re_singleton.hpp>

//#define PLAYER_LIST_SIZE 103

namespace kbf {

    class PlayerTracker {
    public:
        PlayerTracker(KBFDataManager& dataManager) : dataManager{ dataManager } { initialize(); };

        void updatePlayers();
        void applyPresets();
        void reset();

        const std::vector<PlayerData> getPlayerList() const;

        const PlayerInfo& getPlayerInfo(const PlayerData& playerData) const { return *playerInfos.at(playerSlotTable.at(playerData)); }
        PlayerInfo& getPlayerInfo(const PlayerData& playerData) { return *playerInfos.at(playerSlotTable.at(playerData)); }

		const std::optional<PersistentPlayerInfo>& getPersistentPlayerInfo(const PlayerData& playerData) const { return persistentPlayerInfos.at(playerSlotTable.at(playerData)); }
		std::optional<PersistentPlayerInfo>& getPersistentPlayerInfo(const PlayerData& playerData) { return persistentPlayerInfos.at(playerSlotTable.at(playerData)); }

    private:
        void initialize();
        void setupLists();
		bool getPlayerListSize(size_t& out);

        KBFDataManager& dataManager;
		static PlayerTracker* g_instance;
		size_t playerListSize = 0;

        void fetchPlayers();
        void fetchPlayers_MainMenu();
		bool fetchPlayers_MainMenu_BasicInfo(PlayerInfo& outInfo, int& outSaveIdx);
		bool fetchPlayers_MainMenu_WeaponObjects(const PlayerInfo& info, PersistentPlayerInfo& outPInfo);
		void fetchPlayers_SaveSelect();
		bool fetchPlayers_SaveSelect_BasicInfo(PlayerInfo& outInfo);
		bool fetchPlayers_SaveSelect_WeaponObjects(const PlayerInfo& info, PersistentPlayerInfo& outPInfo);
		void fetchPlayers_CharacterCreator();
        bool fetchPlayers_CharacterCreator_BasicInfo(PlayerInfo& outInfo);
		void fetchPlayers_HunterGuildCard();
        bool fetchPlayers_HunterGuildCard_BasicInfo(PlayerInfo& outInfo);
        void fetchPlayers_NormalGameplay();
        void fetchPlayers_NormalGameplay_SinglePlayer(size_t i, bool useCache, bool inQuest, bool online);
        PlayerFetchFlags fetchPlayer_BasicInfo(size_t i, bool inQuest, bool online, PlayerInfo& outInfo);
        bool fetchPlayer_PersistentInfo(size_t i, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
		void fetchPlayer_Visibility(PlayerInfo& info);
        bool fetchPlayer_EquippedArmours(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
		bool fetchPlayer_EquippedArmours_FromSaveFile(const PlayerInfo& info, PersistentPlayerInfo& pInfo, int saveIdx = -1, bool overrideInner = false);
        bool fetchPlayer_EquippedArmours_FromCharaMakeSceneController(REApi::ManagedObject* controller, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
		bool fetchPlayer_EquippedArmours_FromGuildCardHunter(REApi::ManagedObject* hunter, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_ArmourTransforms(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_ArmourTransforms_FromEventModel(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_ArmourTransforms_FromSaveSelectSceneController(REApi::ManagedObject* sceneController, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_ArmourTransforms_FromCharaMakeSceneController(REApi::ManagedObject* sceneController, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_ArmourTransforms_FromGuildCardHunter(REApi::ManagedObject* hunter, const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_WeaponObjects(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_Bones(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_Parts(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        bool fetchPlayer_Materials(const PlayerInfo& info, PersistentPlayerInfo& pInfo);
        void clearPlayerSlot(size_t index);

        static int onIsEquipBuildEndHook(int argc, void** argv, REFrameworkTypeDefinitionHandle* arg_tys, unsigned long long ret_addr);
        int onIsEquipBuildEnd(int argc, void** argv, REFrameworkTypeDefinitionHandle* arg_tys, unsigned long long ret_addr);
        static int onWarpHook(int argc, void** argv, REFrameworkTypeDefinitionHandle* arg_tys, unsigned long long ret_addr);
		int onWarp(int argc, void** argv, REFrameworkTypeDefinitionHandle* arg_tys, unsigned long long ret_addr);
        static int saveSelectListSelectHook(int argc, void** argv, REFrameworkTypeDefinitionHandle* arg_tys, unsigned long long ret_addr);

        int detectPlayer(void* hunterCharacterPtr, const std::string& logStrSuffix);

        // TODO: A lot of this could go in re_engine util calss
        REApi::ManagedObject* getSaveDataObject(int saveIdx = -1);
		static bool isSaveActive(REApi::ManagedObject* save);
        bool getSavePlayerData(int saveIdx, PlayerData& out);
        bool getActiveSavePlayerData(PlayerData& out);
        
        // Helpers for reading armour from save file
        std::optional<ArmourSet> getArmourForPartFromSave(
            REApi::ManagedObject* save,
            REApi::ManagedObject* equip,
            REApi::ManagedObject* outerSet,
            REApi::ManagedObject* visible,
            ArmorParts part,
            bool overrideInner = false);
        ArmorSetID resolveSaveVisibleArmour(
            REApi::ManagedObject* equip,
            ArmorParts part,
            uint32_t outerSeries,
            bool female);
        bool resolveHunterAndController(
            PlayerInfo& outInfo,
            const PlayerData& hunter,
            REApi::ManagedObject*& controllerOut,
            REApi::ManagedObject*& hunterTransformCache,
            REApi::ManagedObject*& sceneControllerCache,
            const char* transformPrefixXX,
            const char* transformPrefixXY,
            const char* sceneControllerName,
            const char* componentTypeName); // false = SaveSelect
        std::optional<ArmorSetID> resolveSaveInnerArmour(REApi::ManagedObject* save, ArmorParts part);
        std::optional<std::pair<uint32_t, bool>> getSaveOuterPartInfo(REApi::ManagedObject* outerSet, ArmorParts part);

        REApi::ManagedObject* getCurrentScene() const;

        void updateApplyDelays();

        //std::unordered_set<size_t> playersToFetch{};
        std::unordered_map<PlayerData, size_t> playerSlotTable{};
        std::unordered_map<PlayerData, std::optional<std::chrono::steady_clock::time_point>> playerApplyDelays{};

        std::vector<bool> playersToFetch;
        std::vector<bool> occupiedNormalGameplaySlots{};
        std::vector<std::optional<PlayerInfo>> playerInfos;
		std::vector<std::optional<PersistentPlayerInfo>> persistentPlayerInfos;

        std::vector<std::optional<NormalGameplayPlayerCache>> playerInfoCaches;

        // Main Menu Refs
        RENativeSingleton sceneManager{ "via.SceneManager" };
        RESingleton saveDataManager{ "app.SaveDataManager" };

        // Save Select Refs
		int lastSelectedSaveIdx = -1;
        reframework::API::ManagedObject* saveSelectHunterTransformCache = nullptr;
        reframework::API::ManagedObject* saveSelectSceneControllerCache = nullptr;
        std::optional<size_t> saveSelectHashedArmourTransformsCache = std::nullopt;

        // Character Creator Refs
        reframework::API::ManagedObject* characterCreatorHunterTransformCache = nullptr;
        reframework::API::ManagedObject* charaMakeSceneControllerCache        = nullptr;
        std::optional<size_t> characterCreatorHashedArmourTransformsCache = std::nullopt;

        // Guild Card Refs
        RESingleton guiManager{ "app.GUIManager" };
        reframework::API::ManagedObject* guildCardHunterTransformCache = nullptr;
        reframework::API::ManagedObject* guildCardHunterGameObjCache   = nullptr;
		reframework::API::ManagedObject* guildCardSceneControllerCache = nullptr;
        std::optional<size_t> guildCardHashedArmourTransformsCache = std::nullopt;

        // Normal Gameplay Refs
        RESingleton playerManager{ "app.PlayerManager" };
        RESingleton networkManager{ "app.NetworkManager" };
        reframework::API::ManagedObject* netUserInfoManager = nullptr;
        reframework::API::ManagedObject* netContextManager = nullptr;
		reframework::API::ManagedObject* Net_UserInfoList = nullptr;

        bool needsAllPlayerFetch = false;

        std::optional<CustomSituation> lastSituation = std::nullopt;
        size_t frameBoneFetchCount = 0;

        // Cutscene & Guild card start-end tracking
        bool frameIsCutscene = false;
		bool frameIsGuildCard = false;
    };

}