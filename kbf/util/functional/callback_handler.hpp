#pragma once

#include <unordered_map>
#include <functional>

namespace kbf {

	class CallbackHandler {
	public:
		size_t addCallback(std::function<void()> callback) { 
			size_t id = nextId++; 
			callbacks[id] = callback; 
			return id; 
		} 
		void triggerAllCallbacks() { 
			for (const auto& [id, callback] : callbacks) {
				callback();
			} 
		}
		void triggerCallback(size_t id) {
			if (callbacks.contains(id)) {
				callbacks[id]();
			}
		}
		void removeCallback(size_t id) { callbacks.erase(id); }
		void clearCallbacks() { callbacks.clear(); }

	private:
		size_t nextId = 0;
		std::unordered_map<size_t, std::function<void()>> callbacks;
	};

}