#include "pch.h"
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
	::ZeroMemory(m_name, sizeof(m_name));
	::ZeroMemory(m_information, sizeof(m_information));
	m_instance = 0;
	::ZeroMemory(&m_pi, sizeof(m_pi));
	m_browser = 0;
}

CInProcessApp::~CInProcessApp()
{
}

void CInProcessApp::dllInit(HINSTANCE instance)
{
	MY_TRACE(_T("CInProcessApp::dllInit(0x%08p)\n"), instance);

	m_instance = instance;
	MY_TRACE_HEX(m_instance);

	::GetModuleFileName(m_instance, m_name, MAX_PATH);
	::PathStripPath(m_name);
	::PathRemoveExtension(m_name);
	MY_TRACE_TSTR(m_name);

	::StringCbCopy(m_information, sizeof(m_information), _T("オブジェクトエクスプローラ version 1.0.0 by 蛇色"));
	MY_TRACE_TSTR(m_information);
}

BOOL CInProcessApp::init(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::init()\n"));

	if (!createSubProcess(fp))
		return FALSE;

	fp->exfunc->add_menu_item(fp, const_cast<LPSTR>("オブジェクトエクスプローラを表示"), fp->hwnd, ID_SHOW, 0, 0);

	return TRUE;
}

BOOL CInProcessApp::exit(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::exit()\n"));

	::CloseHandle(m_pi.hThread);
	::CloseHandle(m_pi.hProcess);

	return TRUE;
}

BOOL CInProcessApp::createSubProcess(FILTER *fp)
{
	MY_TRACE(_T("CInProcessApp::createSubProcess()\n"));

	TCHAR path[MAX_PATH] = {};
	::GetModuleFileName(m_instance, path, MAX_PATH);
	::PathRemoveExtension(path);
	::PathAppend(path, m_name);
	::PathAddExtension(path, _T(".exe"));
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
