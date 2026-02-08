#pragma once

#include <kbf/util/re_engine/reinvoke.hpp>

#include <reframework/API.hpp>

using REApi = reframework::API;

namespace kbf {

	inline REApi::ManagedObject* getComponent(
		REApi::ManagedObject* gameObject,
		const std::string& type
	) {
		REApi::ManagedObject* t = REApi::get()->typeof(type.c_str());
		if (t == nullptr) return nullptr;

		REApi::ManagedObject* c = REInvokePtr<REApi::ManagedObject>(gameObject, "getComponent(System.Type)", { (void*)t });
		return c;
	}

}