#pragma once

#include <kbf/data/file/kbf_file_type.hpp>
#include <kbf/util/versioning/semantic_version.hpp>

#include <rapidjson/document.h>

#include <map>
#include <functional>

namespace kbf {


	class KbfFileUpgrader {
	public:
		KbfFileUpgrader() = default;
		~KbfFileUpgrader() = default;

		enum class UpgradeResult {
			SUCCESS,
			SUCCESS_NON_PERSISTENT, // for files which upgrade but shouldn't be rewritten to persistent storage
			NO_UPGRADE_NEEDED,
			FAILED,
		};

		UpgradeResult upgradeFile(rapidjson::Document& doc, KbfFileType fileType);

	private:
		SemanticVersion getFileVersion(const rapidjson::Document& doc) const;

		using UpgradeLUT = std::map<SemanticVersion, std::function<bool(rapidjson::Document&)>>;
		UpgradeResult upgradeFileUsingLUT(SemanticVersion ver, rapidjson::Document& doc, const UpgradeLUT& lut, bool persistent = true);

		// Files that need upgrades
		bool upgradePreset_1_0_4(rapidjson::Document& doc);
		bool upgradePreset_1_0_6(rapidjson::Document& doc);
		UpgradeLUT presetUpgradeLUT{
			{ {1,0,4}, [this](rapidjson::Document& doc) { return upgradePreset_1_0_4(doc); } },
			{ {1,0,6}, [this](rapidjson::Document& doc) { return upgradePreset_1_0_6(doc); } },
		};

		bool upgradePresetGroup_1_2_0(rapidjson::Document& doc);
		UpgradeLUT presetGroupUpgradeLUT{
			{ {1,2,0}, [this](rapidjson::Document& doc) { return upgradePresetGroup_1_2_0(doc); } }
		};

		bool upgradeBoneCache_1_0_6(rapidjson::Document& doc);
		UpgradeLUT boneCacheUpgradeLUT{
			{ {1,0,6}, [this](rapidjson::Document& doc) { return upgradeBoneCache_1_0_6(doc); } },
		};

		bool upgradePartCache_1_0_6(rapidjson::Document& doc);
		UpgradeLUT partCacheUpgradeLUT{
			{ {1,0,6}, [this](rapidjson::Document& doc) { return upgradePartCache_1_0_6(doc); } },
		};

		bool upgradeDotKBF_1_2_0(rapidjson::Document& doc);
		UpgradeLUT dotKBFUpgradeLUT{
			{ {1,2,0}, [this](rapidjson::Document& doc) { return upgradeDotKBF_1_2_0(doc); } },
		};


		UpgradeResult UPGRADE_NO_OP(SemanticVersion ver, rapidjson::Document& doc) { return UpgradeResult::NO_UPGRADE_NEEDED; }
	};

}