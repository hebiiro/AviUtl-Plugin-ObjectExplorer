#include "pch.h"
#include "afxdialogex.h"
#include "OutProcess.h"
#include "OutProcessDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BEGIN_MESSAGE_MAP(COutProcessDialog, CDialogEx)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_APPCOMMAND()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_PREV, &COutProcessDialog::OnClickedPrev)
	ON_BN_CLICKED(IDC_NEXT, &COutProcessDialog::OnClickedNext)
	ON_BN_CLICKED(IDC_UP, &COutProcessDialog::OnClickedUp)
	ON_BN_CLICKED(IDC_GET, &COutProcessDialog::OnClickedGet)
	ON_COMMAND(ID_ADD_FAVORITE, &COutProcessDialog::OnAddFavorite)
	ON_COMMAND(ID_DELETE_FAVORITE, &COutProcessDialog::OnDeleteFavorite)
	ON_COMMAND(ID_SHOW_NAV_PANE, &COutProcessDialog::OnShowNavPane)
	ON_COMMAND(ID_PLAY_VOICE, &COutProcessDialog::OnPlayVoice)
	ON_COMMAND(ID_USE_COMMON_DIALOG, &COutProcessDialog::OnUseCommonDialog)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NAV_PANE, &COutProcessDialog::OnUpdateShowNavPane)
	ON_UPDATE_COMMAND_UI(ID_PLAY_VOICE, &COutProcessDialog::OnUpdatePlayVoice)
	ON_UPDATE_COMMAND_UI(ID_USE_COMMON_DIALOG, &COutProcessDialog::OnUpdateUseCommonDialog)
	ON_CBN_SELCHANGE(IDC_URL, &COutProcessDialog::OnSelChangeUrl)
	ON_REGISTERED_MESSAGE(WM_AVIUTL_OBJECT_EXPLORER_RESIZE, OnObjectExplorerResize)
	ON_REGISTERED_MESSAGE(WM_AVIUTL_OBJECT_EXPLORER_EXIT, OnObjectExplorerExit)
END_MESSAGE_MAP()

//--------------------------------------------------------------------

COutProcessDialog::COutProcessDialog(CWnd* parent)
	: CDialogEx(IDD_OUT_PROCESS, parent)
{
	MY_TRACE(_T("COutProcessDialog::COutProcessDialog()\n"));

	m_cookie = 0;
	m_shellView = 0;
	m_isSettingsLoaded = FALSE;
	m_isNavPaneVisible = TRUE;
	m_isVoiceEnabled = TRUE;
	m_usesCommonDialog = TRUE;
}

COutProcessDialog::~COutProcessDialog()
{
	MY_TRACE(_T("COutProcessDialog::~COutProcessDialog()\n"));
}

void COutProcessDialog::loadSystemSettings()
{
	MY_TRACE(_T("COutProcessDialog::loadSystemSettings()\n"));

	LPCWSTR filePath = m_fileUpdateChecker->getFilePath();
	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		if (document->load(filePath) == VARIANT_FALSE)
		{
			MY_TRACE(_T("%ws file loading failed\n"), filePath);

			return;
		}

		MSXML2::IXMLDOMElementPtr element = document->documentElement;

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"explorer");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];
			}
		}
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

void COutProcessDialog::loadSettings()
{
	MY_TRACE(_T("COutProcessDialog::loadSettings()\n"));

	m_url.ResetContent();

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"Settings.xml");
	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		if (document->load(filePath) == VARIANT_FALSE)
		{
			MY_TRACE(_T("%ws file loading failed\n"), filePath);

			return;
		}

		MSXML2::IXMLDOMElementPtr element = document->documentElement;

		getPrivateProfileBool(element, L"isNavPaneVisible", m_isNavPaneVisible);
		getPrivateProfileBool(element, L"isVoiceEnabled", m_isVoiceEnabled);
		getPrivateProfileBool(element, L"usesCommonDialog", m_usesCommonDialog);

		::SetProp(GetSafeHwnd(), _T("usesCommonDialog"), (HANDLE)m_usesCommonDialog);

		_bstr_t path;
		getPrivateProfileString(element, L"path", path);
		MY_TRACE_WSTR((BSTR)path);
		if ((BSTR)path)
			m_url.SetWindowText(path);

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"favorite");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				_bstr_t path;
				getPrivateProfileString(element, L"path", path);
				MY_TRACE_WSTR((BSTR)path);
				if ((BSTR)path)
				{
					m_url.AddString(path);
				}
			}
		}

		m_isSettingsLoaded = TRUE;
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

void COutProcessDialog::saveSettings()
{
	MY_TRACE(_T("COutProcessDialog::saveSettings()\n"));

	if (!m_isSettingsLoaded)
	{
		MY_TRACE(_T("初期化に失敗しているので保存処理をスキップします\n"));

		return;
	}

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"Settings.xml");
	MY_TRACE_WSTR(filePath);

	try
	{
		MSXML2::IXMLDOMDocumentPtr document(__uuidof(MSXML2::DOMDocument));

		MSXML2::IXMLDOMElementPtr parentElement = appendElement(document, document, L"ObjectExplorerSettings");

		setPrivateProfileString(parentElement, L"path", (LPCTSTR)m_currentFolderPath);
		setPrivateProfileBool(parentElement, L"isNavPaneVisible", m_isNavPaneVisible);
		setPrivateProfileBool(parentElement, L"isVoiceEnabled", m_isVoiceEnabled);
		setPrivateProfileBool(parentElement, L"usesCommonDialog", m_usesCommonDialog);

		int c = m_url.GetCount();
		for (int i = 0; i < c; i++)
		{
			MSXML2::IXMLDOMElementPtr element = appendElement(document, parentElement, L"favorite");

			CString path; m_url.GetLBText(i, path);
			setPrivateProfileString(element, L"path", (LPCTSTR)path);
		}

		saveXMLDocument(document, filePath);
	}
	catch (_com_error& e)
	{
		MY_TRACE(_T("%s\n"), e.ErrorMessage());
	}
}

void COutProcessDialog::createExplorer()
{
	MY_TRACE(_T("COutProcessDialog::createExplorer()\n"));

	// エクスプローラを作成する。
	HRESULT hr = m_explorer.CreateInstance(CLSID_ExplorerBrowser);

	{
		// エクスプローラを初期化する。

		CWnd* placeHolder = GetDlgItem(IDC_PLACE_HOLDER);
		CRect rc; placeHolder->GetWindowRect(&rc);
		ScreenToClient(&rc);

		FOLDERSETTINGS fs = {};
		fs.ViewMode = FVM_AUTO;
		fs.fFlags = 0;//FWF_NOBROWSERVIEWSTATE;

		hr = m_explorer->Initialize(GetSafeHwnd(), &rc, &fs);
		hr = m_explorer->SetPropertyBag(L"AviUtl.ObjectExplorer");
	}

	// ハンドラを追加する。
	hr = ::IUnknown_SetSite(m_explorer, static_cast<IServiceProvider*>(this));
	hr = m_explorer->Advise(static_cast<IExplorerBrowserEvents*>(this), &m_cookie);
	IFolderFilterSitePtr folderFilterSite = m_explorer;
	MY_TRACE_HEX(folderFilterSite.GetInterfacePtr());
	hr = folderFilterSite->SetFilter(static_cast<IFolderFilter*>(this));
	MY_TRACE_COM_ERROR(hr);

	if (m_isNavPaneVisible)
	{
		// フレームを表示する。
		hr = m_explorer->SetOptions(EBO_SHOWFRAMES);
	}

	CString url; m_url.GetWindowText(url);

	if (url.IsEmpty())
	{
		PIDLIST_ABSOLUTE pidlDesktop;
		::SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, &pidlDesktop);
		m_explorer->BrowseToIDList(pidlDesktop, SBSP_ABSOLUTE);
		::ILFree(pidlDesktop);
	}
	else
	{
		browseToPath(url);
	}

	// エクスプローラの初期化が完了したことを通知する。
	IShellBrowserPtr browser = m_explorer;
	HWND hwnd = 0;
	browser->GetWindow(&hwnd);
	::PostMessage(theApp.getFilterWindow(), WM_AVIUTL_OBJECT_EXPLORER_INITED, (WPARAM)GetSafeHwnd(), (LPARAM)hwnd);
}

void COutProcessDialog::destroyExplorer()
{
	MY_TRACE(_T("COutProcessDialog::destroyExplorer()\n"));

	m_explorer->Unadvise(m_cookie);
	m_explorer->Destroy();
	m_explorer = 0;
}

void COutProcessDialog::browseToPath(LPCTSTR path)
{
	MY_TRACE(_T("COutProcessDialog::browseToPath(%s)\n"), path);

	if (::lstrcmpi(path, m_currentFolderPath) == 0)
	{
		// 現在のフォルダを再読み込みする。

#if 1
		if (m_shellView)
		{
			HRESULT hr = m_shellView->Refresh();
			MY_TRACE_COM_ERROR(hr);
		}
#else
		IShellViewPtr currentView;
		HRESULT hr = m_explorer->GetCurrentView(IID_PPV_ARGS(&currentView));
		if (currentView)
		{
			HRESULT hr = currentView->Refresh();
			MY_TRACE_COM_ERROR(hr);
		}
#endif
	}
	else
	{
		// 指定されたフォルダを表示する。

		PIDLIST_ABSOLUTE pidl = ::ILCreateFromPath(path);
		if (pidl)
		{
			m_explorer->BrowseToIDList(pidl, SBSP_ABSOLUTE | SBSP_SAMEBROWSER);
			::ILFree(pidl);
		}
	}
}

void COutProcessDialog::playVoice(LPCTSTR voice)
{
	MY_TRACE(_T("COutProcessDialog::playVoice(%s)\n"), voice);

	if (!m_isVoiceEnabled)
		return;

	// wav ファイルのパスを取得する。
	TCHAR wavFileName[MAX_PATH] = {};
	::GetModuleFileName(AfxGetInstanceHandle(), wavFileName, MAX_PATH);
	::PathRemoveFileSpec(wavFileName);
	::PathAppend(wavFileName, voice);
	MY_TRACE_TSTR(wavFileName);

	// ファイルが存在するなら
	if (::GetFileAttributes(wavFileName) != INVALID_FILE_ATTRIBUTES)
	{
		// wav ファイルを再生する。

		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpFile = theApp.m_wavPlayerFileName;
		sei.lpParameters = wavFileName;
		BOOL result = ::ShellExecuteEx(&sei);
		MY_TRACE_HEX(result);
	}
}

//--------------------------------------------------------------------

void COutProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	MY_TRACE(_T("COutProcessDialog::DoDataExchange()\n"));

	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_url);
	DDX_Control(pDX, IDC_SEARCH, m_search);
}

BOOL COutProcessDialog::PreTranslateMessage(MSG* pMsg)
{
#if 1
	if (m_shellView && m_shellView->TranslateAccelerator(pMsg) == S_OK)
#else
	IShellViewPtr currentView;
	HRESULT hr = m_explorer->GetCurrentView(IID_PPV_ARGS(&currentView));
	if (currentView && currentView->TranslateAccelerator(pMsg) == S_OK)
#endif
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL COutProcessDialog::OnInitDialog()
{
	MY_TRACE(_T("COutProcessDialog::OnInitDialog()\n"));

	CDialogEx::OnInitDialog();

	// 設定をファイルから読み込む。
	loadSystemSettings();
	loadSettings();

	// エクスプローラを作成する。
	createExplorer();

	// リサイズしてから表示する。
	OnObjectExplorerResize(0, 0);
	ShowWindow(SW_SHOW);

	return TRUE;
}

int COutProcessDialog::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("COutProcessDialog::OnCreate()\n"));

	if (CDialogEx::OnCreate(cs) == -1)
		return -1;

	{
		// DarkenWindow が存在する場合は読み込む。

		TCHAR fileName[MAX_PATH] = {};
		::GetModuleFileName(AfxGetInstanceHandle(), fileName, MAX_PATH);
		::PathRemoveFileSpec(fileName);
		::PathAppend(fileName, _T("..\\DarkenWindow.aul"));
		MY_TRACE_TSTR(fileName);

		HMODULE DarkenWindow = ::LoadLibrary(fileName);
		MY_TRACE_HEX(DarkenWindow);

		if (DarkenWindow)
		{
			typedef void (WINAPI* Type_DarkenWindow_init)(HWND hwnd);
			Type_DarkenWindow_init DarkenWindow_init =
				(Type_DarkenWindow_init)::GetProcAddress(DarkenWindow, "DarkenWindow_init");
			MY_TRACE_HEX(DarkenWindow_init);

			if (DarkenWindow_init)
				DarkenWindow_init(GetSafeHwnd());
		}
	}

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"SystemSettings.xml");
	MY_TRACE_WSTR(filePath);

	m_fileUpdateChecker.reset(new FileUpdateChecker(filePath));

	SetTimer(TIMER_ID_CHECK_SETTING_FILE, 1000, 0);
#ifdef OBJECT_EXPLORER_CHECK_MAIN_PROCESS
	SetTimer(TIMER_ID_CHECK_MAIN_PROCESS, 1000, 0);
#endif
	return 0;
}

void COutProcessDialog::OnDestroy()
{
	MY_TRACE(_T("COutProcessDialog::OnDestroy()\n"));

	// 監視タイマーを終了させる。
	KillTimer(TIMER_ID_CHECK_MAIN_PROCESS);

	// 設定をファイルに保存する。
	saveSettings();

	// エクスプローラを閉じる。
	destroyExplorer();

	CDialogEx::OnDestroy();
}

void COutProcessDialog::OnTimer(UINT_PTR timerId)
{
	switch (timerId)
	{
	case TIMER_ID_CHECK_MAIN_PROCESS:
		{
			if (!::IsWindow(theApp.m_mainProcessWindow))
			{
				KillTimer(TIMER_ID_CHECK_MAIN_PROCESS);
				PostQuitMessage(0);
			}

			break;
		}
	case TIMER_ID_CHECK_SETTING_FILE:
		{
			if (m_fileUpdateChecker->isFileUpdated())
			{
				loadSystemSettings();
			}

			break;
		}
	}

	CDialogEx::OnTimer(timerId);
}

void COutProcessDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CWnd* placeHolder = GetDlgItem(IDC_PLACE_HOLDER);
	if (placeHolder)
	{
		CRect rc; placeHolder->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_explorer->SetRect(0, rc);
	}
}

void COutProcessDialog::OnAppCommand(CWnd* pWnd, UINT nCmd, UINT nDevice, UINT nKey)
{
	switch (nCmd)
	{
	case APPCOMMAND_BROWSER_BACKWARD:
		{
			MY_TRACE(_T("APPCOMMAND_BROWSER_BACKWARD\n"));

			OnClickedPrev();
			break;
		}
	case APPCOMMAND_BROWSER_FORWARD:
		{
			MY_TRACE(_T("APPCOMMAND_BROWSER_FORWARD\n"));

			OnClickedNext();
			break;
		}
	case APPCOMMAND_BROWSER_REFRESH:
		{
			MY_TRACE(_T("APPCOMMAND_BROWSER_REFRESH\n"));

			break;
		}
	}

	__super::OnAppCommand(pWnd, nCmd, nDevice, nKey);
}

void COutProcessDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
	MY_TRACE(_T("COutProcessDialog::OnContextMenu()\n"));

	CMenu menu; menu.LoadMenu(IDR_POPUP_MENU);
	CMenu* subMenu = menu.GetSubMenu(0);
	subMenu->CheckMenuItem(ID_SHOW_NAV_PANE, m_isNavPaneVisible ? MF_CHECKED : MF_UNCHECKED);
	subMenu->CheckMenuItem(ID_PLAY_VOICE, m_isVoiceEnabled ? MF_CHECKED : MF_UNCHECKED);
	subMenu->CheckMenuItem(ID_USE_COMMON_DIALOG, m_usesCommonDialog ? MF_CHECKED : MF_UNCHECKED);
	subMenu->TrackPopupMenu(0, point.x, point.y, this);
}

//--------------------------------------------------------------------

void COutProcessDialog::OnOK()
{
	MY_TRACE(_T("COutProcessDialog::OnOK()\n"));

	CString path; m_url.GetWindowText(path);
	MY_TRACE_TSTR(path);

	browseToPath(path);

//	__super::OnOK();
}

void COutProcessDialog::OnCancel()
{
	MY_TRACE(_T("COutProcessDialog::OnCancel()\n"));

//	ShowWindow(SW_HIDE);

//	__super::OnCancel();
}

void COutProcessDialog::OnClickedPrev()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedPrev()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_NAVIGATEBACK);

	playVoice(_T("Prev.wav"));
}

void COutProcessDialog::OnClickedNext()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedNext()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_NAVIGATEFORWARD);

	playVoice(_T("Next.wav"));
}

void COutProcessDialog::OnClickedUp()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedUp()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_PARENT);

	playVoice(_T("Up.wav"));
}

void COutProcessDialog::OnClickedGet()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedGet()\n"));

	::PostMessage(theApp.getFilterWindow(), WM_AVIUTL_OBJECT_EXPLORER_GET, (WPARAM)GetSafeHwnd(), (LPARAM)m_url.GetSafeHwnd());

	playVoice(_T("Get.wav"));
}

void COutProcessDialog::OnAddFavorite()
{
	MY_TRACE(_T("COutProcessDialog::OnAddFavorite()\n"));

	int index = m_url.FindString(0, m_currentFolderPath);
	if (index == CB_ERR)
	{
		m_url.AddString(m_currentFolderPath);

		playVoice(_T("AddFavorite.wav"));
	}
}

void COutProcessDialog::OnDeleteFavorite()
{
	MY_TRACE(_T("COutProcessDialog::OnDeleteFavorite()\n"));

	int index = m_url.FindString(0, m_currentFolderPath);
	if (index != CB_ERR)
	{
		m_url.DeleteString(index);

		playVoice(_T("DeleteFavorite.wav"));
	}
}

void COutProcessDialog::OnShowNavPane()
{
	MY_TRACE(_T("COutProcessDialog::OnShowNavPane()\n"));

	if (m_isNavPaneVisible)
	{
		m_isNavPaneVisible = FALSE;
		destroyExplorer();
		createExplorer();
	}
	else
	{
		m_isNavPaneVisible = TRUE;
		destroyExplorer();
		createExplorer();
	}
}

void COutProcessDialog::OnPlayVoice()
{
	MY_TRACE(_T("COutProcessDialog::OnPlayVoice()\n"));

	if (m_isVoiceEnabled)
	{
		m_isVoiceEnabled = FALSE;
	}
	else
	{
		m_isVoiceEnabled = TRUE;
	}
}

void COutProcessDialog::OnUseCommonDialog()
{
	MY_TRACE(_T("COutProcessDialog::OnUseCommonDialog()\n"));

	if (m_usesCommonDialog)
	{
		m_usesCommonDialog = FALSE;
	}
	else
	{
		m_usesCommonDialog = TRUE;
	}

	::SetProp(GetSafeHwnd(), _T("usesCommonDialog"), (HANDLE)m_usesCommonDialog);
}

void COutProcessDialog::OnUpdateShowNavPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_isNavPaneVisible);
}

void COutProcessDialog::OnUpdatePlayVoice(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_isVoiceEnabled);
}

void COutProcessDialog::OnUpdateUseCommonDialog(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_usesCommonDialog);
}

void COutProcessDialog::OnSelChangeUrl()
{
	MY_TRACE(_T("COutProcessDialog::OnSelChangeUrl()\n"));

	int index = m_url.GetCurSel();
	MY_TRACE_INT(index);
	if (index != CB_ERR)
	{
		CString path; m_url.GetLBText(index, path);
		MY_TRACE_TSTR((LPCTSTR)path);

		browseToPath(path);
	}
}

//--------------------------------------------------------------------

LRESULT COutProcessDialog::OnObjectExplorerResize(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessDialog::OnObjectExplorerResize(0x%08X, 0x%08X)\n"), wParam, lParam);

	CWnd* parent = GetParent();
	if (parent)
	{
		CRect rc; parent->GetClientRect(&rc);
		SetWindowPos(0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	}

	return 0;
}

LRESULT COutProcessDialog::OnObjectExplorerExit(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("COutProcessDialog::OnObjectExplorerExit(0x%08X, 0x%08X)\n"), wParam, lParam);

	DestroyWindow();

	return 0;
}

//--------------------------------------------------------------------
