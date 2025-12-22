/**************************************************************************/
/*  poki_sdk.h                                                            */
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

#ifndef POKI_SDK_H
#define POKI_SDK_H

#include "core/object.h"

#ifdef JAVASCRIPT_ENABLED
#include "platform/javascript/api/javascript_singleton.h"
#endif

class PokiSDK : public Object {
	GDCLASS(PokiSDK, Object);

	static PokiSDK *singleton;

#ifdef JAVASCRIPT_ENABLED
	Ref<JavaScriptObject> sdk_handle;
	Ref<JavaScriptObject> cb_commercial_break;
	Ref<JavaScriptObject> cb_reward_break;
	Ref<JavaScriptObject> cb_shareable_url;

	void _ensure_ready();
	void _chain_then(const Variant &p_promise, const Ref<JavaScriptObject> &p_callback);

	void _on_commercial_break(const Array &p_args);
	void _on_reward_break(const Array &p_args);
	void _on_shareable_url(const Array &p_args);
#endif

protected:
	static void _bind_methods();

public:
	static PokiSDK *get_singleton();

	bool is_available();
	bool is_ad_blocked();

	void gameplay_start();
	void gameplay_stop();
	void game_loading_finished();
	void commercial_break();
	void rewarded_break();
	void enable_event_tracking();
	void shareable_url(const Dictionary &p_params);

	PokiSDK();
	~PokiSDK();
};

#endif // POKI_SDK_H
