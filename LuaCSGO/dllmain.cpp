#include <Windows.h>
#include <thread>
#include "lua/Clua.h"
#include "config/config.h"
#include "utils/console/console.h"


void init() {
	if (!g_console.allocate("Debug"))
		std::abort();

	g_console.log("INIT");

	// redirect warnings to a window similar to errors.
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_WNDW);

	lua::init_state();
	g_config.init();
	g_config.reset();

	//g_interfaces.init();
	//g_gui.init();
	//g_memory.init();
	//g_netvars.init();
	//g_hooks.init();

	for (auto hk : lua::hooks->getHooks("on_before_init_command"))
	{
		try
		{
			auto result = hk.func();
			if (!result.valid()) {
				sol::error err = result;
				g_console.log(err.what());
			}
		}
		catch (const std::exception& err)
		{
			g_console.log(err.what());
		}
	}

	lua::init_command();

	for (auto hk : lua::hooks->getHooks("on_after_init_command"))
	{
		try
		{
			auto result = hk.func();
			if (!result.valid()) {
				sol::error err = result;
				g_console.log(err.what());
			}
		}
		catch (const std::exception& err)
		{
			g_console.log(err.what());
		}
	}
}

void load() {
	for (auto hk : lua::hooks->getHooks("on_load"))
	{
		try
		{
			auto result = hk.func();
			if (!result.valid()) {
				sol::error err = result;
				g_console.log(err.what());
			}
		}
		catch (const std::exception& err)
		{
			g_console.log(err.what());
		}
	}
}

void wait() {
	g_console.log("WAIT");
	lua::init_console();
}

void unload() {
	for (auto hk : lua::hooks->getHooks("on_unload"))
	{
		try
		{
			auto result = hk.func();
			if (!result.valid()) {
				sol::error err = result;
				g_console.log(err.what());
			}
		}
		catch (const std::exception& err)
		{
			g_console.log(err.what());
		}
	}
	g_console.log("UNLOAD");
	g_console.detach();
}

void error() {
	for (auto hk : lua::hooks->getHooks("on_error"))
	{
		try
		{
			auto result = hk.func();
			if (!result.valid()) {
				sol::error err = result;
				g_console.log(err.what());
			}
		}
		catch (const std::exception& err)
		{
			g_console.log(err.what());
		}
	}
	g_console.log("ERROR");
	g_console.detach();
}

DWORD WINAPI DLL_INIT(LPVOID base) {
    while (!GetModuleHandleA("serverbrowser.dll"))
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	try
	{
		//INIT
		init();

		//LOAD
		load();

		//WAIT
		wait();

		//UNLOAD
		unload();

	}
	catch (const std::exception&)
	{
		error();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE init_thread = CreateThread(nullptr, NULL, DLL_INIT, hModule, NULL, nullptr);
        if (!init_thread)
            return 0;

        CloseHandle(init_thread);
    }
    return TRUE;
}

