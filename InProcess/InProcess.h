#pragma once

#include "resource.h"

class CInProcessApp
{
public:

	HINSTANCE m_instance;
	PROCESS_INFORMATION m_pi;
	HWND m_dialog;
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
		::PostMessage(m_dialog, message, wParam, lParam);
	}

	void sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::SendMessage(m_dialog, message, wParam, lParam);
	}
};

extern CInProcessApp theApp;
