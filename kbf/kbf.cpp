#include <kbf/kbf.hpp>

#include <kbf/situation/situation_watcher.hpp>
#include <kbf/debug/debug_stack.hpp>
#include <kbf/cimgui/cimgui_funcs.hpp>

#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>  

#include <iostream>

namespace kbf {

	bool KBF::pluginDisabled = false;

    KBF::KBF() {
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);

        setInitializationTriggers();
    }

    void KBF::setInitializationTriggers() {
        // Do this on a separate thread to avoid freezing the game during the main menu load
        // This is a one time off -> on trigger, so no need to post anything back to the main thread :)
        auto initFn = [&]() {
            // Pre check to avoid unnecessary thread creation
            if (instance.isInitializing() || instance.isInitialized()) return; 
            std::thread([&]() {
                instance.initialize();
            }).detach();
        };

        // Trigger on all situations that are inside of the loaded game loop.
        //  Unfortunately we have to define these manually since c++ has no way of checking whether a certain enum value exists
        //  (i.e. if we wanted to do all but a few 'exceptions')

        SituationWatcher& watcher = SituationWatcher::get();

        watcher.onTriggerSituation(isOnline						     , initFn);
        watcher.onTriggerSituation(isSoloOnline					     , initFn);
        watcher.onTriggerSituation(isOfflineorMainMenu				 , initFn);
        watcher.onTriggerSituation(isinQuestPreparing				 , initFn);
        watcher.onTriggerSituation(isinQuestReady					 , initFn);
        watcher.onTriggerSituation(isinQuestPlayingasHost			 , initFn);
        watcher.onTriggerSituation(isinQuestPlayingasGuest			 , initFn);
        watcher.onTriggerSituation(isinQuestPlayingfromFieldSurvey	 , initFn);
        watcher.onTriggerSituation(DUPLICATE_isinQuestPlayingasGuest , initFn);
        watcher.onTriggerSituation(isinArenaQuestPlayingasHost		 , initFn);
        watcher.onTriggerSituation(isinQuestPressSelectToEnd		 , initFn);
        watcher.onTriggerSituation(isinQuestEndAnnounce			     , initFn);
        watcher.onTriggerSituation(isinQuestResultScreen			 , initFn);
        watcher.onTriggerSituation(isinQuestLoadingResult			 , initFn);
        watcher.onTriggerSituation(isinLinkPartyAsGuest			     , initFn);
        watcher.onTriggerSituation(isinTrainingArea				     , initFn);
        watcher.onTriggerSituation(isinJunctionArea				     , initFn);
        watcher.onTriggerSituation(isinSuja						     , initFn);
        watcher.onTriggerSituation(isinGrandHub					     , initFn);
        watcher.onTriggerSituation(DUPLICATE_isinTrainingArea		 , initFn);
        watcher.onTriggerSituation(isinBowlingGame					 , initFn);
        watcher.onTriggerSituation(isinArmWrestling				     , initFn);
        watcher.onTriggerSituation(isatTable						 , initFn);
        //watcher.onTriggerSituation(isAlwaysOn						 , initFn); // outside of loaded game loop

        watcher.onTriggerSituation(isInMainMenuScene    , initFn);
        watcher.onTriggerSituation(isInSaveSelectGUI    , initFn);
        watcher.onTriggerSituation(isInCharacterCreator , initFn);
        watcher.onTriggerSituation(isInHunterGuildCard  , initFn);
        watcher.onTriggerSituation(isInCutscene         , initFn);
        watcher.onTriggerSituation(isInGame             , initFn);
        //watcher.onTriggerSituation(isInTitleMenus       , initFn); // outside of loaded game loop
    }

    KBF::~KBF() {
		SymCleanup(GetCurrentProcess());
    }

    KBF& KBF::get() {
        static KBF kbf;
        return kbf;
    }

    int KBF::handleException(const char* line, EXCEPTION_POINTERS * ep) {
        KBF::get().condStackTrace(line, ep);
        return EXCEPTION_EXECUTE_HANDLER;
	}

    void KBF::onPreUpdateMotion() {
        if (pluginDisabled) return;

        __try {
            get().instance.onPreUpdateMotion();
        }
        __except (handleException("onPreUpdateMotion", GetExceptionInformation())) {
            pluginDisabled = true;
        }
    }

    void KBF::onPostUpdateMotion() { }

	void KBF::onPostLateUpdateBehavior() {
        if (pluginDisabled) return;

        __try {
		    get().instance.onPostLateUpdateBehavior();
        }
        __except (handleException("onPostLateUpdateBehavior", GetExceptionInformation())) {
            pluginDisabled = true;
        }
    }

    void KBF::drawUI() {
        if (pluginDisabled) {
            if (reframework::API::get()->reframework()->is_drawing_ui()) {
                instance.drawDisabled();
            }
        }
        else {
            __try {
                if (reframework::API::get()->reframework()->is_drawing_ui()) {
                    instance.draw();
                }
            }
            __except (handleException("drawUI", GetExceptionInformation())) {
                pluginDisabled = true;
		    }
        }

    }

    void KBF::condStackTrace(const char* line, EXCEPTION_POINTERS* ep) {
        if (!pluginDisabled) {
            reframework::API::get()->log_error(std::format("KBF Encountered a crash in function: {}. Stack Trace:", line).c_str());
            logStackTrace(ep);
			logKbfDebugLog();
		}
    }

    void KBF::logStackTrace(EXCEPTION_POINTERS* ep) {
        CONTEXT context = *ep->ContextRecord;
        STACKFRAME64 stackFrame = {};
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        DWORD machineType = IMAGE_FILE_MACHINE_AMD64;

        stackFrame.AddrPC.Offset = context.Rip; // x64
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Rbp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;

        while (StackWalk64(machineType, process, thread, &stackFrame, &context, nullptr,
            SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
            if (stackFrame.AddrPC.Offset == 0) break;

            HMODULE module = nullptr;
            DWORD64 moduleBase = 0;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCSTR>(stackFrame.AddrPC.Offset),
                &module)) {
                MODULEINFO info = {};
                if (GetModuleInformation(process, module, &info, sizeof(info))) {
                    moduleBase = reinterpret_cast<DWORD64>(info.lpBaseOfDll);
                }
            }

            const char* moduleName = module ? "" : "UnknownModule";
            char moduleFileName[MAX_PATH] = {};
            if (module) GetModuleFileNameA(module, moduleFileName, MAX_PATH);
            if (moduleFileName[0]) moduleName = moduleFileName;

            DWORD64 offset = stackFrame.AddrPC.Offset - moduleBase;

            // Log the current frame
            reframework::API::get()->log_error(
                std::format("Frame:    {} + 0x{:016x}", moduleName, offset).c_str()
            );
        }
    }

    void KBF::logKbfDebugLog() {
        reframework::API::get()->log_error(std::format("KBF Debug Log Start (VERSION={}):", KBF_VERSION).c_str());
		reframework::API::get()->log_error(DEBUG_STACK.string().c_str());
		reframework::API::get()->log_error("KBF Debug Log End");
    }
}