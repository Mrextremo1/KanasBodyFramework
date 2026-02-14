#pragma once

#include <cstdint>
#include <string>

namespace kbf {

	enum ArmourPiece {
		AP_MIN  = 0,
		AP_SET  = 0, // This is reserved for the player's base transform
		AP_MIN_EXCLUDING_SET = 1,
		AP_ARMS = 1,
		AP_BODY = 2,
		AP_HELM = 3,
		AP_LEGS = 4,
		AP_COIL = 5,
		AP_MAX_EXCLUDING_SLINGER  = 5,
		AP_SLINGER = 6,
		AP_MAX = 6,
		CUSTOM_AP_PARTS = 7,
		CUSTOM_AP_MATS  = 8
	};

	typedef enum ArmourPieceFlagBits {
		APF_NONE         = 0b000000000,
		APF_SET          = 0b000000001,
		APF_HELM         = 0b000000010,
		APF_BODY         = 0b000000100,
		APF_ARMS         = 0b000001000,
		APF_COIL         = 0b000010000,
		APF_LEGS         = 0b000100000,
		APF_SLINGER      = 0b001000000,
		APF_ALL          = 0b001111111,
		CUSTOM_APF_PARTS = 0b010000000,
		CUSTOM_APF_MATS  = 0b100000000
	} ArmourPieceFlagBits;

	typedef uint32_t ArmourPieceFlags;

	inline ArmourPieceFlags getArmourPieceFlag(ArmourPiece piece) {
		switch (piece) {
		case ArmourPiece::AP_SET:  return ArmourPieceFlagBits::APF_SET;
		case ArmourPiece::AP_HELM: return ArmourPieceFlagBits::APF_HELM;
		case ArmourPiece::AP_BODY: return ArmourPieceFlagBits::APF_BODY;
		case ArmourPiece::AP_ARMS: return ArmourPieceFlagBits::APF_ARMS;
		case ArmourPiece::AP_COIL: return ArmourPieceFlagBits::APF_COIL;
		case ArmourPiece::AP_LEGS: return ArmourPieceFlagBits::APF_LEGS;
		case ArmourPiece::AP_SLINGER: return ArmourPieceFlagBits::APF_SLINGER;
		case ArmourPiece::CUSTOM_AP_MATS:  return ArmourPieceFlagBits::CUSTOM_APF_MATS;
		case ArmourPiece::CUSTOM_AP_PARTS: return ArmourPieceFlagBits::CUSTOM_APF_PARTS;
		default: return APF_NONE;
		}
	}

	inline std::string armourPieceToString(ArmourPiece piece) {
		switch (piece) {
		case ArmourPiece::AP_SET:     return "Set";
		case ArmourPiece::AP_HELM:    return "Helm";
		case ArmourPiece::AP_BODY:    return "Body";
		case ArmourPiece::AP_ARMS:    return "Arms";
		case ArmourPiece::AP_COIL:    return "Coil";
		case ArmourPiece::AP_LEGS:    return "Legs";
		case ArmourPiece::AP_SLINGER: return "Slinger";
		case ArmourPiece::CUSTOM_AP_PARTS: return "Parts";
		case ArmourPiece::CUSTOM_AP_MATS:  return "Materials";
		default: return "Unknown";
		}
	}
	
}