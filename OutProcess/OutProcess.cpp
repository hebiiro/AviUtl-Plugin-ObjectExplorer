#include "pch.h"
#include "OutProcess.h"
#include "OutProcessDialog.h"
#include "Common/Tracer2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

COutProcessApp theApp;

//--------------------------------------------------------------------

BEGIN_MESSAGE_MAP(COutProcessApp, CWinApp)
	ON_REGISTERED_THREAD_MESSAGE(WM_AVIUTL_OBJECT_EXPLORER_SHOW, OnObjectExplorerShow)
END_MESSAGE_MAP()

//--------------------------------------------------------------------

COutProcessApp::COutProcessApp()
{
	_tsetlocale(LC_ALL, _T(""));

	trace_init(0, 0);

	m_mainProcessWindow = 0;
}

COutProcessApp::~COutProcessApp()
{
	trace_term();
}

HWND COutProcessApp::getAviUtlWindow()
{
	return ::GetWindow(getFilterWindow(), GW_OWNER);
}

HWND COutProcessApp::getFilterWindow()
{
	return m_mainProcessWindow;
}

BOOL COutProcessApp::InitInstance()
{
	CWinApp::InitInstance();

	{
		// メインプロセスのウィンドウを取得する。
		m_mainProcessWindow = (HWND)_tcstoul(::GetCommandLine(), 0, 0);
		MY_TRACE_HWND(m_mainProcessWindow);
#ifdef OBJECT_EXPLORER_CHECK_MAIN_PROCESS
		// メインプロセスのウィンドウが無効ならプロセスを終了する。
		if (!::IsWindow(m_mainProcessWindow))
			return FALSE;
#endif
	}

	// OLE (COM) を初期化する。
	if (!AfxOleInit())
	{
		AfxMessageBox(_T("AfxOleInit() failed."));

		return FALSE;
	}

	// メインダイアログを作成する。
	CWnd* parent = CWnd::FromHandle(getAviUtlWindow());
	m_pMainWnd = &m_dialog;
	if (!m_dialog.Create(IDD_OUT_PROCESS, parent))
	{
		AfxMessageBox(_T("dialog.Create(IDD_OUT_PROCESS) failed."));
	}
	else
	{
		// 設定をファイルから読み込む。
		m_dialog.loadSystemSettings();
		m_dialog.loadSettings();

		// メッセージループを開始する。
		m_dialog.RunModalLoop(MLF_NOKICKIDLE);

		// 設定をファイルに保存する。
		m_dialog.saveSettings();

		// メインダイアログを削除する。
		m_dialog.DestroyWindow();
	}
#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif
	return FALSE;
}

void COutProcessApp::OnObjectExplorerShow(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessApp::OnObjectExplorerShow(0x%08X, 0x%08X)\n"), wParam, lParam);

	if (wParam == -1)
	{
		if (m_dialog.IsWindowVisible())
			m_dialog.ShowWindow(SW_HIDE);
		else
			m_dialog.ShowWindow(SW_SHOW);
	}
	else
	{
		m_dialog.ShowWindow(wParam);
	}
}

//--------------------------------------------------------------------
