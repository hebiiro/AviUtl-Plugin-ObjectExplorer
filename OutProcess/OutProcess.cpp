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

void CALLBACK COutProcessApp::timerProc(HWND hwnd, UINT message, UINT_PTR timerId, DWORD time)
{
#ifdef OBJECT_EXPLORER_CHECK_MAIN_PROCESS
	if (!::IsWindow(theApp.m_mainProcessWindow))
	{
		MY_TRACE(_T("メインプロセスのウィンドウが無効になりました\n"));

		::PostQuitMessage(0);
	}
#endif
}

BOOL COutProcessApp::InitInstance()
{
	CWinApp::InitInstance();

	{
		LPWSTR commandLine = ::GetCommandLineW();
		MY_TRACE_WSTR(commandLine);

		int argc = 0;
		LPWSTR* argv = ::CommandLineToArgvW(commandLine, &argc);
		MY_TRACE_INT(argc);

		LPWSTR arg = commandLine;
		if (argc >=  2) // 引数が 2 個以上なら
			arg = argv[1]; // 2 番目の引数を使用する。
		MY_TRACE_WSTR(arg);

		// メインプロセスのウィンドウを取得する。
		m_mainProcessWindow = (HWND)wcstoul(arg, 0, 0);
		MY_TRACE_HWND(m_mainProcessWindow);

#ifdef OBJECT_EXPLORER_CHECK_MAIN_PROCESS
		// メインプロセスのウィンドウが無効ならプロセスを終了する。
		if (!::IsWindow(m_mainProcessWindow))
			return FALSE;
#endif
		DWORD pid = 0;
		::GetWindowThreadProcessId(m_mainProcessWindow, &pid);
		Handle process = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		::GetModuleFileNameEx(process, 0, m_mainProcessFileName, _countof(m_mainProcessFileName));
		MY_TRACE_TSTR(m_mainProcessFileName);

		::StringCbCopy(m_wavPlayerFileName, sizeof(m_wavPlayerFileName), m_mainProcessFileName);
		::PathRemoveFileSpec(m_wavPlayerFileName);
		::PathAppend(m_wavPlayerFileName, _T("WavPlayer.exe"));
		::PathQuoteSpaces(m_wavPlayerFileName);
		MY_TRACE_TSTR(m_wavPlayerFileName);
	}

	// OLE (COM) を初期化する。
	if (!AfxOleInit())
	{
		AfxMessageBox(_T("AfxOleInit() failed."));
		return FALSE;
	}

	// メインダイアログを作成する。
	CWnd* parent = CWnd::FromHandle(getFilterWindow());
	m_pMainWnd = &m_dialog;
	if (!m_dialog.Create(IDD_OUT_PROCESS, parent))
	{
		AfxMessageBox(_T("m_dialog.Create() failed."));
		return FALSE;
	}

	// メインプロセスを監視するタイマーを作成する。
	::SetTimer(0, 0, 1000, timerProc);

	MY_TRACE(_T("メッセージループを開始します\n"));

	// メッセージループを開始する。
	m_dialog.RunModalLoop(MLF_NOKICKIDLE);

	MY_TRACE(_T("プロセスが正常に終了しました\n"));

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif
	return FALSE;
}

//--------------------------------------------------------------------
