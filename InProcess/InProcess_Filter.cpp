#include "pch.h"
#include "InProcess.h"
#include "InProcess_Hook.h"

//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable(void)
{
	static TCHAR name[MAX_PATH] = {};
	::StringCbCopy(name, sizeof(name), _T("オブジェクトエクスプローラ"));

	static TCHAR information[MAX_PATH] = {};
	::StringCbCopy(information, sizeof(information), _T("オブジェクトエクスプローラ 3.0.0 by 蛇色"));

	static FILTER_DLL filter =
	{
		FILTER_FLAG_ALWAYS_ACTIVE |
//		FILTER_FLAG_MAIN_MESSAGE |
		FILTER_FLAG_WINDOW_THICKFRAME |
		FILTER_FLAG_WINDOW_SIZE |
		FILTER_FLAG_DISP_FILTER |
		FILTER_FLAG_EX_INFORMATION,
		400, 400,
		name,
		NULL, NULL, NULL,
		NULL, NULL,
		NULL, NULL, NULL,
		NULL,//func_proc,
		func_init,
		func_exit,
		NULL,
		func_WndProc,
		NULL, NULL,
		NULL,
		NULL,
		information,
		NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL,
	};

	return &filter;
}

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

BOOL func_init(FILTER *fp)
{
	MY_TRACE(_T("func_init()\n"));

	initHook(fp);

	return theApp.init(fp);
}

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------
BOOL func_exit(FILTER *fp)
{
	MY_TRACE(_T("func_exit()\n"));

	termHook(fp);

	return theApp.exit(fp);
}

//---------------------------------------------------------------------
//		フィルタされた画像をバッファにコピー
//---------------------------------------------------------------------
BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	MY_TRACE(_T("func_proc() : %d\n"), ::GetTickCount());

	return TRUE;
}

//---------------------------------------------------------------------
//		WndProc
//---------------------------------------------------------------------
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp)
{
	//	TRUEを返すと全体が再描画される

	switch (message)
	{
	case WM_FILTER_INIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_INIT)\n"));

			modifyStyle(hwnd, 0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

			break;
		}
	case WM_FILTER_EXIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_EXIT)\n"));

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
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			theApp.dllInit(instance);

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			MY_TRACE(_T("DLL_PROCESS_DETACH\n"));

			break;
		}
	}

	return TRUE;
}

//---------------------------------------------------------------------
