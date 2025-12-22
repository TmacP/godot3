/**************************************************************************/
/*  poki_sdk.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "poki_sdk.h"

PokiSDK *PokiSDK::singleton = nullptr;

PokiSDK *PokiSDK::get_singleton() {
	return singleton;
}

PokiSDK::PokiSDK() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "PokiSDK singleton already exists.");
	singleton = this;
}

PokiSDK::~PokiSDK() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void PokiSDK::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_available"), &PokiSDK::is_available);
	ClassDB::bind_method(D_METHOD("is_ad_blocked"), &PokiSDK::is_ad_blocked);
	ClassDB::bind_method(D_METHOD("gameplay_start"), &PokiSDK::gameplay_start);
	ClassDB::bind_method(D_METHOD("gameplay_stop"), &PokiSDK::gameplay_stop);
	ClassDB::bind_method(D_METHOD("game_loading_finished"), &PokiSDK::game_loading_finished);
	ClassDB::bind_method(D_METHOD("commercial_break"), &PokiSDK::commercial_break);
	ClassDB::bind_method(D_METHOD("rewarded_break"), &PokiSDK::rewarded_break);
	ClassDB::bind_method(D_METHOD("enable_event_tracking"), &PokiSDK::enable_event_tracking);
	ClassDB::bind_method(D_METHOD("shareable_url", "params"), &PokiSDK::shareable_url);

	ADD_SIGNAL(MethodInfo("commercial_break_done", PropertyInfo(Variant::BOOL, "response")));
	ADD_SIGNAL(MethodInfo("rewarded_break_done", PropertyInfo(Variant::BOOL, "response")));
	ADD_SIGNAL(MethodInfo("shareable_url_ready", PropertyInfo(Variant::STRING, "url")));
}

bool PokiSDK::is_available() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	return sdk_handle.is_valid();
#else
	return false;
#endif
}

bool PokiSDK::is_ad_blocked() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return false;
	}
	Variant::CallError err;
	Variant ret = sdk_handle->call("isAdBlocked", nullptr, 0, err);
	if (err.error != Variant::CallError::CALL_OK) {
		return false;
	}
	return ret.operator bool();
#else
	return false;
#endif
}

void PokiSDK::gameplay_start() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	sdk_handle->call("gameplayStart", nullptr, 0, err);
#endif
}

void PokiSDK::gameplay_stop() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	sdk_handle->call("gameplayStop", nullptr, 0, err);
#endif
}

void PokiSDK::game_loading_finished() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	sdk_handle->call("gameLoadingFinished", nullptr, 0, err);
#endif
}

void PokiSDK::commercial_break() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	Variant promise = sdk_handle->call("commercialBreak", nullptr, 0, err);
	_chain_then(promise, cb_commercial_break);
#endif
}

void PokiSDK::rewarded_break() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	Variant promise = sdk_handle->call("rewardedBreak", nullptr, 0, err);
	_chain_then(promise, cb_reward_break);
#endif
}

void PokiSDK::enable_event_tracking() {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	Variant::CallError err;
	sdk_handle->call("enableEventTracking", nullptr, 0, err);
#endif
}

void PokiSDK::shareable_url(const Dictionary &p_params) {
#ifdef JAVASCRIPT_ENABLED
	_ensure_ready();
	if (!sdk_handle.is_valid()) {
		return;
	}
	if (!JavaScript::get_singleton()) {
		return;
	}

	Variant object_name = String("Object");
	const Variant *args[1] = { &object_name };
	Variant::CallError r_error;
	Variant obj_var = JavaScript::get_singleton()->_create_object_bind(args, 1, r_error);
	if (r_error.error != Variant::CallError::CALL_OK || obj_var.get_type() != Variant::OBJECT) {
		return;
	}
	Ref<JavaScriptObject> js_obj = obj_var;
	if (!js_obj.is_valid()) {
		return;
	}

	Array keys = p_params.keys();
	for (int i = 0; i < keys.size(); i++) {
		Variant k = keys[i];
		if (k.get_type() != Variant::STRING) {
			continue;
		}
		String key = k;
		js_obj->set(key, p_params[k]);
	}

	Variant::CallError err;
	const Variant *call_args[1] = { &obj_var };
	Variant promise = sdk_handle->call("shareableURL", call_args, 1, err);
	_chain_then(promise, cb_shareable_url);
#endif
}

#ifdef JAVASCRIPT_ENABLED
void PokiSDK::_ensure_ready() {
	if (sdk_handle.is_valid()) {
		return;
	}
	if (!JavaScript::get_singleton()) {
		return;
	}
	Ref<JavaScriptObject> iface = JavaScript::get_singleton()->get_interface("PokiSDK");
	if (!iface.is_valid()) {
		return;
	}
	sdk_handle = iface;
	cb_commercial_break = JavaScript::get_singleton()->create_callback(this, "_on_commercial_break");
	cb_reward_break = JavaScript::get_singleton()->create_callback(this, "_on_reward_break");
	cb_shareable_url = JavaScript::get_singleton()->create_callback(this, "_on_shareable_url");
}

void PokiSDK::_chain_then(const Variant &p_promise, const Ref<JavaScriptObject> &p_callback) {
	if (!p_callback.is_valid()) {
		return;
	}
	if (p_promise.get_type() != Variant::OBJECT) {
		return;
	}
	Ref<JavaScriptObject> promise_obj = p_promise;
	if (!promise_obj.is_valid()) {
		return;
	}
	Variant cb = p_callback;
	const Variant *args[1] = { &cb };
	Variant::CallError err;
	promise_obj->call("then", args, 1, err);
}

void PokiSDK::_on_commercial_break(const Array &p_args) {
	bool response = false;
	if (p_args.size() > 0) {
		response = (bool)p_args[0];
	}
	emit_signal("commercial_break_done", response);
}

void PokiSDK::_on_reward_break(const Array &p_args) {
	bool response = false;
	if (p_args.size() > 0) {
		response = (bool)p_args[0];
	}
	emit_signal("rewarded_break_done", response);
}

void PokiSDK::_on_shareable_url(const Array &p_args) {
	String url;
	if (p_args.size() > 0) {
		url = p_args[0];
	}
	emit_signal("shareable_url_ready", url);
}
#endif
