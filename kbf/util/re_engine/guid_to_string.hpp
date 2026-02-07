#pragma once

#include <kbf/util/re_engine/reinvoke.hpp>
#include <kbf/util/id/guid.hpp>
#include <kbf/enums/localization_language.hpp>

#define GUID_TO_STRING_LOG_TAG "[GuidToString]"

namespace kbf {

    inline std::string GuidToString(const Guid& guid) {
		return REInvokeStaticStr("via.gui.message", "get(System.Guid)", { (void*)&guid });
	}

    inline std::string GuidToLocalizedString(const Guid& guid, LocalizationLanguage language) {
        return REInvokeStaticStr("via.gui.message", "get(System.Guid, via.Language)", { (void*)&guid, (void*)language });
    }

    // Extension for convenient GUID parsing
    inline std::string REInvokeGuidStatic(
        std::string callerTypeName,
        std::string methodName,
        std::vector<void*> args,
		LocalizationLanguage language = LocalizationLanguage::Invalid
    ) {
        reframework::API::TypeDefinition* callerType = reframework::API::get()->tdb()->find_type(callerTypeName);
        #if defined(ENABLE_REINVOKE_LOGGING) && defined(REINVOKE_LOGGING_LEVEL_NULL) && defined(REINVOKE_LOGGING_LEVEL_ERROR)
        if (callerType == nullptr) {
            DEBUG_STACK.push(std::format("Failed to fetch caller type definition: {}", callerTypeName), DebugStack::Color::COL_ERROR);
        }
        #endif
        if (callerType == nullptr) return "ERR: Null caller type!";

        reframework::API::Method* callerMethod = callerType->find_method(methodName);
        #if defined(ENABLE_REINVOKE_LOGGING) && defined(REINVOKE_LOGGING_LEVEL_NULL) && defined(REINVOKE_LOGGING_LEVEL_ERROR)
        if (callerMethod == nullptr) {
            DEBUG_STACK.push(std::format("Failed to find method {}. {} has the following properties:\n{}", methodName, callerTypeName, reTypePropertiesToString(callerType)), DebugStack::Color::COL_ERROR);
        }
        #endif
		if (callerMethod == nullptr) return "ERR: Null caller method!";
        
        reframework::InvokeRet ret = callerMethod->invoke(nullptr, args);

        if (ret.exception_thrown) {
            DEBUG_STACK.push(std::format("{} REInvokeGuidStatic: {} threw an exception!", REINVOKE_LOG_TAG, methodName), DebugStack::Color::COL_DEBUG);
        }

        Guid guid;
        std::memcpy(&guid, ret.bytes.data(), sizeof(Guid));

        if (language == LocalizationLanguage::Invalid) 
            return GuidToString(guid);
        else 
			return GuidToLocalizedString(guid, language);
    }

   
}