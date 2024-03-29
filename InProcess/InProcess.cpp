﻿#include "pch.h"
#include "InProcess.h"

//--------------------------------------------------------------------

CInProcessApp theApp;

//--------------------------------------------------------------------

void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

//--------------------------------------------------------------------

CInProcessApp::CInProcessApp()
{
}

CInProcessApp::~CInProcessApp()
{
}

BOOL CInProcessApp::dllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			// ロケールを設定する。
			// これをやらないと日本語テキストが文字化けするので最初に実行する。
			_tsetlocale(LC_CTYPE, _T(""));

			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			m_instance = instance;
			MY_TRACE_HEX(m_instance);

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

BOOL CInProcessApp::init(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("CInProcessApp::init()\n"));

	if (!createSubProcess(fp))
		return FALSE;

	return TRUE;
}

BOOL CInProcessApp::exit(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("CInProcessApp::exit()\n"));

	::CloseHandle(m_pi.hThread);
	::CloseHandle(m_pi.hProcess);

	return TRUE;
}

BOOL CInProcessApp::createSubProcess(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("CInProcessApp::createSubProcess()\n"));

	TCHAR path[MAX_PATH] = {};
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRemoveExtension(path);
	::PathAppend(path, _T("ObjectExplorer.exe"));
	MY_TRACE_TSTR(path);

	TCHAR args[MAX_PATH] = {};
	::StringCbPrintf(args, sizeof(args), _T("0x%08p"), fp->hwnd);
	MY_TRACE_TSTR(args);

	STARTUPINFO si = { sizeof(si) };
	if (!::CreateProcess(
		path,           // No module name (use command line)
		args,           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&m_pi))         // Pointer to PROCESS_INFORMATION structur
	{
		MY_TRACE(_T("::CreateProcess() failed.\n"));

		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------
