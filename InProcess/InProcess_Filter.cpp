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
	::StringCbCopy(information, sizeof(information), _T("オブジェクトエクスプローラ 2.1.0 by 蛇色"));

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
		func_proc,
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
	case WM_FILTER_UPDATE:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_UPDATE)\n"));

			if (fp->exfunc->is_editing(editp) != TRUE) break; // 編集中でなければ終了

			break;
		}
	case WM_FILTER_CHANGE_EDIT:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_CHANGE_EDIT)\n"));

			if (fp->exfunc->is_editing(editp) != TRUE) break; // 編集中でなければ終了

			break;
		}
	case WM_FILTER_CHANGE_WINDOW:
		{
			MY_TRACE(_T("func_WndProc(WM_FILTER_CHANGE_WINDOW)\n"));

			if (fp->exfunc->is_filter_window_disp(fp))
			{
				theApp.postMessage(WM_AVIUTL_OBJECT_EXPLORER_SHOW, SW_SHOW, 0);
				::ShowWindow(fp->hwnd, SW_HIDE);
			}
			else
			{
				theApp.postMessage(WM_AVIUTL_OBJECT_EXPLORER_SHOW, SW_HIDE, 0);
				::ShowWindow(fp->hwnd, SW_HIDE);
			}

			break;
		}
	case WM_FILTER_COMMAND:
		{
			switch (wParam)
			{
			case ID_SHOW:
				{
					MY_TRACE(_T("func_WndProc(WM_COMMAND, ID_SHOW)\n"));

					theApp.postMessage(WM_AVIUTL_OBJECT_EXPLORER_SHOW, -1, 0);

					break;
				}
			}

			break;
		}
	}

	if (message == WM_AVIUTL_OBJECT_EXPLORER_INITED)
	{
		MY_TRACE(_T("func_WndProc(WM_AVIUTL_OBJECT_EXPLORER_INITED)\n"));

		theApp.m_browser = (HWND)wParam;
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
