#pragma once

namespace kbf {

	class NpcPrefabAliasMappings {
	public:
		static std::string getPrefabAlias(const std::string& NpcStrID, size_t variant = 0) {
			if (mappings.find(NpcStrID) == mappings.end()) return "";
			const std::vector<std::string>& variants = mappings.at(NpcStrID);
			if (variant >= variants.size()) return "";
			return variants[variant];
		}

	private:
		inline static const std::unordered_map<std::string, std::vector<std::string>> mappings{
			// NPCs with variants
			{ "NPC102_00_001", {
				"[NPC] Alma's Handler's Outfit"          ,
				"[NPC] Alma's Scrivener's Coat"          ,
				"[NPC] Alma's Spring Blossom Kimono"     ,
				"[NPC] Alma's Summer Poncho"             ,
				"[NPC] Alma's New World Commission"      ,
				"[NPC] Alma's Chun Li Outfit"            ,
				"[NPC] Alma's Cammy Outfit"              ,
				"[NPC] Alma's Autumn Witch Outfit"       ,
				"[NPC] Alma's Featherskirt Seikret Dress",
			} },
			{ "NPC102_00_010", {
				"[NPC] Gemma's Smithy's Outfit"       ,
				"[NPC] Gemma's Summer Coveralls"      ,
				"[NPC] Gemma's Redveil Seikret Dress" ,
			} },
			{ "NPC101_00_002", {
				"[NPC] Erik's Handler's Outfit"         ,
				"[NPC] Erik's Summer Hat"               ,
				"[NPC] Erik's Autumn Therian"           ,
				"[NPC] Erik's Crestcollar Seikret Suit" ,
			} },
			{ "NPC101_00_004", {
				"[NPC] Nata's Outfit",
				"[NPC] Nata's Outfit",
				"[NPC] Nata's Hunter Outfit",
			} },
			{ "NPC101_00_006", {
				"[NPC] Fabius's Outfit",
				"[NPC] Fabius's Outfit",
				"[NPC] Fabius's Outfit"
			} },
			{ "NPC102_00_007", {
				"[NPC] Olivia's Outfit",
				"[NPC] Olivia's Outfit",
				"[NPC] Olivia's Outfit",
				"[NPC] Olivia's Outfit"
			} },
			{ "NPC112_00_040", { // Felicita
				"[NPC] Clerk",
				"[NPC] Clerk (Blossomdance)"          ,
				"[NPC] Clerk (Flamefete)"             ,
				"[NPC] Clerk (Dreamspell)"            ,
				"[NPC] Clerk (Lumenhymn)"             ,
			} },
			{ "NPC111_01_003", { // Ricardo
				"[NPC] Clerk",
				"[NPC] Clerk (Blossomdance)"          ,
				"[NPC] Clerk (Flamefete)"             ,
				"[NPC] Clerk (Dreamspell)"            ,
				"[NPC] Clerk (Lumenhymn)"             ,
			} },

			// NPCs without variants
			{ "NPC101_00_003", { "[NPC] Nata's Sild Poncho" } },
			{ "NPC102_00_041", { "[NPC] Nadia's Outfit" } },
			{ "NPC202_01_018", { "[NPC] Diva's Outfit" } },
			{ "NPC101_00_009", { "[NPC] Werner's Outfit" } },
			{ "NPC201_00_035", { "[NPC] Tetsuzuan's Outfit" } },
			{ "NPC102_13_014", { "[NPC] Maki's Outfit" } },
			{ "NPC101_13_012", { "[NPC] Dogard's Outfit" } },
			{ "NPC102_13_013", { "[NPC] Aida's Outfit" } },
			{ "NPC101_13_011", { "[NPC] Yabran's Outfit" } },
			{ "NPC112_11_039", { "[NPC] Amone's Outfit" } },
			{ "NPC111_21_058", { "[NPC] Santiago's Outfit" } },
			{ "NPC202_14_002", { "[NPC] The Allhearken's Outfit" } },
			{ "NPC101_11_066", { "[NPC] Y'sai's Outfit" } },
			{ "NPC101_11_069", { "[NPC] Zatoh's Outfit" } },
			{ "NPC102_11_016", { "[NPC] Elder Ela's Outfit" } },
			{ "NPC102_11_067", { "[NPC] Nona's Outfit" } },
			{ "NPC222_14_025", { "[NPC] Vio's Outfit" } },
			{ "NPC221_01_008", { "[NPC] Legendary Artisan's Outfit" } },
			{ "NPC101_50_024", { "[NPC] Akuma's Outfit" } },
			{ "NPC101_00_005", { "[NPC] Sild Poncho B" } }, // Tasheen
			{ "NPC102_00_031", { "[NPC] Explorer" } }, // Kanya
			{ "NPC111_00_008", { "[NPC] Explorer" } }, // Blazej
			{ "NPC111_00_037", { "[NPC] Defender Armour A" } }, // Rex
			{ "NPC112_00_902", { "[NPC] Defender Armour B" } }, // Nina
			{ "NPC111_11_022", { "[NPC] Windward Plains Outfit A" } }, // Rugul
			{ "NPC111_11_070", { "[NPC] Windward Plains Outfit B" } }, // Zareh
			{ "NPC112_11_026", { "[NPC] Windward Plains Outfit C" } }, // Sauba
			{ "NPC111_11_023", { "[NPC] Windward Plains Outfit D" } }, // Karab
			{ "NPC112_11_028", { "[NPC] Windward Plaius Outfit E" } }, // Kaisa
			{ "NPC121_11_045", { "[NPC] Windward Plains Elder Outfit" } }, // Strabi
			{ "NPC122_11_060", { "[NPC] Windward Plains Elder Outfit" } }, // Nyek
			{ "NPC131_11_050", { "[NPC] Windward Plains Child Outfit A" } }, // ??
			{ "NPC131_11_048", { "[NPC] Windward Plains Child Outfit B" } }, // Vuno
			{ "NPC111_13_015", { "[NPC] Oilwell Basin Outfit A" } }, // Roqul
			{ "NPC112_13_023", { "[NPC] Oilwell Basin Outfit A" } }, // Luluru
			{ "NPC111_13_029", { "[NPC] Oilwell Basin Outfit B" } }, // Goltora
			{ "NPC112_13_040", { "[NPC] Oilwell Basin Outfit B" } }, // Hama
			{ "NPC111_13_056", { "[NPC] Oilwell Basin Outfit C" } }, // Lavala
			{ "NPC112_13_065", { "[NPC] Oilwell Basin Outfit C" } }, // Kyana
			{ "NPC121_13_026", { "[NPC] Oilwell Basin Elder Outfit" } }, // Dotoma
			{ "NPC131_13_028", { "[NPC] Oilwell Basin Child Outfit" } }, // Noruth
			{ "NPC112_01_004", { "[NPC] Provisions Outfit A" } }, // Catherine, stock provisions lady
			{ "NPC101_21_001", { "[NPC] Provisions Outfit A" } }, // Jack
			{ "NPC211_00_023", { "[NPC] Provisions Outfit B" } }, // Enrai
			{ "NPC212_01_052", { "[NPC] Provisions Outfit B" } }, // ??
			{ "NPC211_14_003", { "[NPC] Suja Outfit" } }, // Shimakee
			{ "NPC212_14_012", { "[NPC] Suja Outfit" } }, // Rasa
			{ "NPC211_14_011", { "[NPC] Suja Guard" } }, // Usurai
			{ "NPC212_14_021", { "[NPC] Suja Guard" } }, // Apar
			{ "NPC221_14_022", { "[NPC] Suja Elder Outfit A" } }, // Neepas
			{ "NPC111_15_003", { "[NPC] Sild Poncho A" } }, // Ren
			{ "NPC112_15_011", { "[NPC] Sild Poncho A" } }, // Velre
			{ "NPC111_15_004", { "[NPC] Sild Poncho B" } }, // Lauruel
			{ "NPC112_15_012", { "[NPC] Sild Poncho B" } }, // Anolia
			{ "NPC221_21_035", { "[NPC] Elder Researcher's Outfit" } }, // Ferdinand
			{ "NPC211_21_036", { "[NPC] Researcher's Outfit" } }, // Samin
			{ "NPC212_01_016", { "[NPC] Researcher's Outfit" } }, // Jutta
			{ "NPC112_21_049", { "[NPC] Innerwear" } }, // Nikiya
			{ "NPC111_22_008", { "[NPC] Innerwear" } }, // ??
		};
	};

}