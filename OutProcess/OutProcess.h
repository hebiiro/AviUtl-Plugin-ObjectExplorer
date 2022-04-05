#pragma once

#include "resource.h"
#include "OutProcessDialog.h"

//--------------------------------------------------------------------

class COutProcessApp : public CWinApp
{
public:

	HWND m_mainProcessWindow;
	COutProcessDialog m_dialog;

public:

	COutProcessApp();
	virtual ~COutProcessApp();

	HWND getAviUtlWindow();
	HWND getFilterWindow();

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
	afx_msg void OnObjectExplorerShow(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------

extern COutProcessApp theApp;

//--------------------------------------------------------------------
