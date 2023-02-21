#pragma once

#include "resource.h"
#include "OutProcessDialog.h"

//--------------------------------------------------------------------

class COutProcessApp : public CWinApp
{
public:

	HWND m_mainProcessWindow;
	TCHAR m_mainProcessFileName[MAX_PATH];
	TCHAR m_wavPlayerFileName[MAX_PATH];
	COutProcessDialog m_dialog;

public:

	COutProcessApp();
	virtual ~COutProcessApp();

	HWND getAviUtlWindow();
	HWND getFilterWindow();

	static void CALLBACK timerProc(HWND hwnd, UINT message, UINT_PTR timerId, DWORD time);

	void postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::PostMessage(m_mainProcessWindow, message, wParam, lParam);
	}

	void sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		::SendMessage(m_mainProcessWindow, message, wParam, lParam);
	}

	LONG getWindowLong(int index)
	{
		return ::GetWindowLong(m_mainProcessWindow, index);
	}

public:

	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------

extern COutProcessApp theApp;

//--------------------------------------------------------------------
