#pragma once
#include "sol/sol.hpp"
#include <map>
#include <filesystem>
#define M(x,y) {##x[#y] = ns_##x::##y;}
#define N(x,y) {##x[#y] = ##y;}
namespace lua {
	struct c_lua_hook {
		int scriptId;
		sol::protected_function func;
	};

	class c_lua_hookManager {
	public:
		void registerHook(std::string eventName, int scriptId, sol::protected_function func);
		void unregisterHooks(int scriptId);

		std::vector<c_lua_hook> getHooks(std::string eventName);

	private:
		std::map<std::string, std::vector<c_lua_hook>> hooks;
	};
	void init_state();
	void init_command();
	void init_console();
	void unload();

	void load_script(int id);
	void unload_script(int id);
	void reload_all_scripts();
	void unload_all_scripts();
	void refresh_scripts();
	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);

	extern std::vector<std::filesystem::path> pathes;
	extern std::vector<bool> loaded;
	extern std::vector<std::string> scripts;
	extern c_lua_hookManager* hooks;
	extern lua_State* g_lua_state;
	extern bool g_unload_flag;
}