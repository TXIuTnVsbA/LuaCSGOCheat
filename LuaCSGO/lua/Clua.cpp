#include <fstream>
#include <ShlObj.h>
#include "CLua.h"
#include "../utils/console/console.h"
#include "../utils/KeyState.h"
#include "../config/config.h"
namespace lua {
	bool g_unload_flag = false;
	lua_State* g_lua_state = NULL;
	c_lua_hookManager* hooks = new c_lua_hookManager();
	std::vector<bool> loaded;
	std::vector<std::string> scripts;
	std::vector<std::filesystem::path> pathes;

	int extract_owner(sol::this_state st) {
		sol::state_view lua_state(st);
		sol::table rs = lua_state["debug"]["getinfo"](2, "S");
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();
		return get_script_id(filename);
	}

	namespace ns_lua {
		void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
			sol::state_view lua_state(s);
			sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
			std::string source = rs["source"];
			std::string filename = std::filesystem::path(source.substr(1)).filename().string();

			hooks->registerHook(eventname, get_script_id(filename), func);

			g_console.log("%s: subscribed to event %s", filename.c_str(), eventname.c_str());
		}

		void run_script(std::string scriptname) {
			load_script(get_script_id(scriptname));
		}

		void reload_active_scripts() {
			reload_all_scripts();
		}

		void refresh() {
			unload_all_scripts();
			refresh_scripts();
			load_script(get_script_id("autorun.lua"));
		}
	};

	namespace ns_config {
		/*
		config.get(key)
		Returns value of given key or nil if key not found.
		*/
		std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
			std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.c)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

			for (auto kv : g_config.f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);


			for (auto kv : g_config.i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			return retn;
		}

		/*
		config.set(key, value)
		Sets value for key
		*/
		void set_bool(std::string key, bool v) {
			g_config.b[key] = v;
		}

		void set_string(std::string key, std::string v) {
			g_config.s[key] = v;
		}

		void set_float(std::string key, float v) {
			if (ceilf(v) != v)
				g_config.f[key] = v;
			else
				g_config.i[key] = (int)v;
		}

		/*
		config.set(key, r,g,b,a)
		*/
		void set_color(std::string key, int r, int g, int b, int a) {
			g_config.c[key][0] = r / 255.f;
			g_config.c[key][1] = g / 255.f;
			g_config.c[key][2] = b / 255.f;
			g_config.c[key][3] = a / 255.f;
		}

		/*
		config.set(key, index, value)
		*/
		void set_select_tuple(std::string key, int pos, bool v) {
			g_config.i_b[key][pos] = v;
		}

		void set_string_tuple(std::string key, int pos, std::string v) {
			g_config.i_s[key][pos] = v;
		}

		void set_number_tuple(std::string key, int pos, float v) {
			if (ceilf(v) != v)
				g_config.i_f[key][pos] = v;
			else
				g_config.i_i[key][pos] = (int)v;

		}

		void set_select_map(std::string key, std::string pos, bool v) {
			g_config.s_b[key][pos] = v;
		}

		void set_string_map(std::string key, std::string pos, std::string v) {
			g_config.s_s[key][pos] = v;
		}

		void set_number_map(std::string key, std::string pos, float v) {
			if (ceilf(v) != v)
				g_config.s_f[key][pos] = v;
			else
				g_config.s_i[key][pos] = (int)v;

		}

		void load(size_t id) {
			g_config.load(id);
		}

		void save(size_t id) {
			g_config.save(id);
		}

		void add(std::string name) {
			g_config.add(name.c_str());
		}

		void remove(size_t id) {
			g_config.remove(id);
		}

		void rename(size_t id, std::string name) {
			g_config.rename(id, name.c_str());
		}

		void reset() {
			g_config.reset();
		}

		void init() {
			g_config.init();
		}

		sol::object configs(sol::this_state this_) {
			auto configs = g_config.getConfigs();
			if (!configs.empty())
			{
				return sol::make_object(this_, configs);
			}
			else {
				return sol::nil;
			}
		}

	};

	namespace ns_log {
		void enable_log_file(std::string filename) {
			g_console.enable_log_file(filename);
		}
	};

	void test() {
		g_console.log("RunTestFunc");
		for (auto hk : hooks->getHooks("on_test"))
		{
			try
			{
				auto result = hk.func();
				if (!result.valid()) {
					sol::error err = result;
					g_console.log(err.what());
				}
			}
			catch (const std::exception&)
			{

			}
		}
	}

	void init_state() {
		lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
		lua_writeline();
		lua::unload();
		g_lua_state = luaL_newstate();
		luaL_openlibs(g_lua_state);
	}

	void init_command() {
		sol::state_view lua_state(g_lua_state);
		lua_state["exit"] = []() { g_unload_flag = true; };
		lua_state["test"] = []() { test(); };
		auto config = lua_state.create_table();
		M(config, get);
		config["set"] = sol::overload(
			ns_config::set_bool,
			ns_config::set_color,
			ns_config::set_float,
			ns_config::set_string,
			ns_config::set_select_tuple,
			ns_config::set_number_tuple,
			ns_config::set_string_tuple,
			ns_config::set_select_map,
			ns_config::set_number_map,
			ns_config::set_string_map
		);
		M(config, init);
		M(config, save);
		M(config, load);
		M(config, reset);
		M(config, add);
		M(config, remove);
		M(config, rename);
		M(config, configs);
		N(lua_state, config);

		auto lua = lua_state.create_table();
		M(lua, set_event_callback);
		M(lua, run_script);
		M(lua, reload_active_scripts);
		M(lua, refresh);
		N(lua_state, lua);

		auto log = lua_state.create_table();
		M(log, enable_log_file);
		N(lua_state, log);

		refresh_scripts();
		load_script(get_script_id("autorun.lua"));
	}

	void init_console() {
		std::string str;
		//printf("lua_ptr: 0x%x \n", &g_console);
		g_console.log("LUA");
		std::printf(">");
		while (!g_unload_flag && std::getline(std::cin, str))
		{
			try
			{
				bool tmp = luaL_dostring(g_lua_state, str.c_str());
				if (tmp) {
					std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, -1));
				}
			}
			catch (const std::exception& Error)
			{
				std::printf("std::Error: %s\n", Error.what());
				//std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, 0));
				//g_console.log("std::Error: %s", Error.what());
				//g_console.log("lua::Error: %s", lua_tostring(g_lua_state, 0));
			}
			g_console.log("LUA");
			std::printf(">");
		}
	}

	void unload() {
		if (g_lua_state != NULL) {
			lua_close(g_lua_state);
			g_lua_state = NULL;
		}
	}

	void load_script(int id) {
		if (id == -1)
			return;

		if (loaded.at(id))
			return;

		auto path = get_script_path(id);
		if (path == (""))
			return;

		sol::state_view state(g_lua_state);
		state.script_file(path, [](lua_State*, sol::protected_function_result result) {
			if (!result.valid()) {
				sol::error err = result;
				//Utilities->Game_Msg(err.what());
				g_console.log(err.what());
			}

			return result;
			});

		loaded.at(id) = true;
	}

	void unload_script(int id) {
		if (id == -1)
			return;

		if (!loaded.at(id))
			return;

		hooks->unregisterHooks(id);
		loaded.at(id) = false;
	}

	void reload_all_scripts() {
		for (auto s : scripts) {
			if (loaded.at(get_script_id(s))) {
				unload_script(get_script_id(s));
				load_script(get_script_id(s));
			}
		}
	}

	void unload_all_scripts() {
		for (auto s : scripts)
			if (loaded.at(get_script_id(s)))
				unload_script(get_script_id(s));
	}

	void refresh_scripts() {
		std::filesystem::path path;
		path = g_config.get_path();
		path /= "Lua";

		if (!std::filesystem::is_directory(path)) {
			std::filesystem::remove(path);
			std::filesystem::create_directory(path);
		}

		auto oldLoaded = loaded;
		auto oldScripts = scripts;

		loaded.clear();
		pathes.clear();
		scripts.clear();

		for (auto& entry : std::filesystem::directory_iterator((path))) {
			if (entry.path().extension() == (".lua")) {
				auto path = entry.path();
				auto filename = path.filename().string();

				bool didPut = false;
				int oldScriptsSize = 0;
				oldScriptsSize = oldScripts.size();
				if (oldScriptsSize < 0)
					continue;

				for (int i = 0; i < oldScriptsSize; i++) {
					if (filename == oldScripts.at(i)) {
						loaded.push_back(oldLoaded.at(i));
						didPut = true;
					}
				}

				if (!didPut)
					loaded.push_back(false);

				pathes.push_back(path);
				scripts.push_back(filename);
			}
		}
	}

	int get_script_id(std::string name) {
		int scriptsSize = 0;
		scriptsSize = scripts.size();
		if (scriptsSize <= 0)
			return -1;

		for (int i = 0; i < scriptsSize; i++) {
			if (scripts.at(i) == name)
				return i;
		}

		return -1;
	}

	int get_script_id_by_path(std::string path) {
		int pathesSize = 0;
		pathesSize = pathes.size();
		if (pathesSize <= 0)
			return -1;

		for (int i = 0; i < pathesSize; i++) {
			if (pathes.at(i).string() == path)
				return i;
		}

		return -1;
	}

	std::string get_script_path(std::string name) {
		return get_script_path(get_script_id(name));
	}

	std::string get_script_path(int id) {
		if (id == -1)
			return  "";

		return pathes.at(id).string();
	}

	void c_lua_hookManager::registerHook(std::string eventName, int scriptId, sol::protected_function func) {
		c_lua_hook hk = { scriptId, func };

		this->hooks[eventName].push_back(hk);
	}

	void c_lua_hookManager::unregisterHooks(int scriptId) {
		for (auto& ev : this->hooks) {
			int pos = 0;

			for (auto& hk : ev.second) {
				if (hk.scriptId == scriptId)
					ev.second.erase(ev.second.begin() + pos);

				pos++;
			}
		}
	}

	std::vector<c_lua_hook> c_lua_hookManager::getHooks(std::string eventName) {
		return this->hooks[eventName];
	}
}
