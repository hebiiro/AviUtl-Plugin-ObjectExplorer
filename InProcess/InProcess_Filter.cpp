#include "pch.h"
#include "InProcess.h"
#include "InProcess_Hook.h"

//--------------------------------------------------------------------

BOOL func_init(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_init()\n"));

	initHook(fp);

	return theApp.init(fp);
}

BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_exit()\n"));

	termHook(fp);

	return theApp.exit(fp);
}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	switch (message)
	{
	case AviUtl::FilterPlugin::WindowMessage::Init:
		{
			MY_TRACE(_T("func_WndProc(Init, 0x%08X, 0x%08X)\n"), wParam, lParam);

			modifyStyle(hwnd, 0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::Exit:
		{
			MY_TRACE(_T("func_WndProc(Exit, 0x%08X, 0x%08X)\n"), wParam, lParam);

			theApp.sendMessage(WM_AVIUTL_OBJECT_EXPLORER_EXIT, 0, 0);

			break;
		}
	case WM_SIZE:
		{
			MY_TRACE(_T("func_WndProc(WM_SIZE)\n"));

			theApp.postMessage(WM_AVIUTL_OBJECT_EXPLORER_RESIZE, 0, 0);

			break;
		}
	}

	if (message == WM_AVIUTL_OBJECT_EXPLORER_INITED)
	{
		MY_TRACE(_T("func_WndProc(WM_AVIUTL_OBJECT_EXPLORER_INITED)\n"));

		theApp.m_dialog = (HWND)wParam;
		theApp.m_browser = (HWND)lParam;

		MY_TRACE_HWND(theApp.m_dialog);
		MY_TRACE_HWND(theApp.m_browser);
	}
	else if (message == WM_AVIUTL_OBJECT_EXPLORER_GET)
	{
		MY_TRACE(_T("func_WndProc(WM_AVIUTL_OBJECT_EXPLORER_GET)\n"));

		TCHAR path[MAX_PATH] = {};
		getPath(path);
		if (::lstrlen(path))
		{
			HWND dialog = (HWND)wParam;
			HWND url = (HWND)lParam;
			::SendMessage(url, WM_SETTEXT, 0, (LPARAM)path);
			::PostMessage(dialog, WM_COMMAND, IDOK, 0);
		}
	}

	return FALSE;
}

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	return theApp.dllMain(instance, reason, reserved);
}

//--------------------------------------------------------------------

EXTERN_C AviUtl::FilterPluginDLL* WINAPI GetFilterTable()
{
	static char name[MAX_PATH] = "オブジェクトエクスプローラ";
	static char information[MAX_PATH] = "オブジェクトエクスプローラ 3.6.1 by 蛇色";

	char fileName[MAX_PATH] = {};
	::GetModuleFileNameA(theApp.m_instance, fileName, MAX_PATH);
	::PathStripPathA(fileName);
	::PathRemoveExtensionA(fileName);
	MY_TRACE_STR(fileName);

	if (::lstrcmpiA(fileName, "ObjectExplorer") != 0)
		::StringCbCopyA(name, sizeof(name), fileName);

	static AviUtl::FilterPluginDLL filter =
	{
		.flag =
			AviUtl::FilterPlugin::Flag::AlwaysActive |
			AviUtl::FilterPlugin::Flag::DispFilter |
			AviUtl::FilterPlugin::Flag::WindowThickFrame |
			AviUtl::FilterPlugin::Flag::WindowSize |
			AviUtl::FilterPlugin::Flag::ExInformation,
		.x = 400,
		.y = 400,
		.name = name,
		.func_init = func_init,
		.func_exit = func_exit,
		.func_WndProc = func_WndProc,
		.information = information,
	};

	return &filter;
}

//--------------------------------------------------------------------
