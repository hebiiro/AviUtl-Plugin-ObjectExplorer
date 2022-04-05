#pragma once

#include "resource.h"

const UINT ID_SHOW = 1000;

class CInProcessApp
{
public:

	TCHAR m_name[MAX_PATH];
	TCHAR m_information[MAX_PATH];

	HINSTANCE m_instance;
	PROCESS_INFORMATION m_pi;
	HWND m_browser;

public:

	CInProcessApp();
	~CInProcessApp();

	void dllInit(HINSTANCE instance);
	BOOL init(FILTER *fp);
	BOOL exit(FILTER *fp);
	BOOL createSubProcess(FILTER *fp);

	void postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::PostThreadMessage(m_pi.dwThreadId, message, wParam, lParam);
	}
};

extern CInProcessApp theApp;
