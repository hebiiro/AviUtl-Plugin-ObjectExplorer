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
	ON_CBN_SELCHANGE(IDC_URL, &COutProcessDialog::OnSelChangeUrl)
END_MESSAGE_MAP()

//--------------------------------------------------------------------

COutProcessDialog::COutProcessDialog(CWnd* parent)
	: CDialogEx(IDD_OUT_PROCESS, parent)
{
	MY_TRACE(_T("COutProcessDialog::COutProcessDialog()\n"));

	m_cookie = 0;
	m_isSettingsLoaded = FALSE;
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

		_bstr_t path;
		getPrivateProfileString(element, L"path", path);
		MY_TRACE_WSTR((BSTR)path);
		if ((BSTR)path)
		{
			browseToPath(path);
		}

		{
			MSXML2::IXMLDOMNodeListPtr nodeList = element->selectNodes(L"window");
			int c = nodeList->length;
			for (int i = 0; i < c; i++)
			{
				MSXML2::IXMLDOMElementPtr element = nodeList->item[i];

				int x = 0, y = 0, w = 0, h = 0;
				getPrivateProfileInt(element, L"x", x);
				getPrivateProfileInt(element, L"y", y);
				getPrivateProfileInt(element, L"w", w);
				getPrivateProfileInt(element, L"h", h);

				if (w > 0 && h > 0)
					MoveWindow(x, y, w, h);

				BOOL show = FALSE;
				getPrivateProfileInt(element, L"show", show);
				ShowWindow(show ? SW_SHOW : SW_HIDE);
			}
		}

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

		{
			MSXML2::IXMLDOMElementPtr element = appendElement(document, parentElement, L"window");

			setPrivateProfileInt(element, L"show", IsWindowVisible());

			CRect rc; GetWindowRect(&rc);
			int x = rc.left;
			int y = rc.top;
			int w = rc.Width();
			int h = rc.Height();
			setPrivateProfileInt(element, L"x", x);
			setPrivateProfileInt(element, L"y", y);
			setPrivateProfileInt(element, L"w", w);
			setPrivateProfileInt(element, L"h", h);
		}

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

void COutProcessDialog::browseToPath(LPCTSTR path)
{
	MY_TRACE(_T("COutProcessDialog::browseToPath(%s)\n"), path);

	PIDLIST_ABSOLUTE pidl = ::ILCreateFromPath(path);
	if (pidl)
	{
		m_explorer->BrowseToIDList(pidl, SBSP_ABSOLUTE);
		::ILFree(pidl);
	}
}

//--------------------------------------------------------------------

void COutProcessDialog::DoDataExchange(CDataExchange* pDX)
{
	MY_TRACE(_T("COutProcessDialog::DoDataExchange()\n"));

	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_url);
}

BOOL COutProcessDialog::PreTranslateMessage(MSG* pMsg)
{
	IShellViewPtr currentView;
	HRESULT hr = m_explorer->GetCurrentView(IID_PPV_ARGS(&currentView));
	if (currentView && currentView->TranslateAccelerator(pMsg) == S_OK)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL COutProcessDialog::OnInitDialog()
{
	MY_TRACE(_T("COutProcessDialog::OnInitDialog()\n"));

	CDialogEx::OnInitDialog();

	// エクスプローラを作成する。
	HRESULT hr = m_explorer.CreateInstance(CLSID_ExplorerBrowser);

	{
		// エクスプローラを初期化する。

		CWnd* placeHolder = GetDlgItem(IDC_PLACE_HOLDER);
		CRect rc; placeHolder->GetWindowRect(&rc);
		ScreenToClient(&rc);

		FOLDERSETTINGS fs;
		fs.ViewMode = FVM_DETAILS;
		fs.fFlags = 0;

		hr = m_explorer->Initialize(GetSafeHwnd(), &rc, &fs);
	}

	// ハンドラを追加する。
	hr = ::IUnknown_SetSite(m_explorer, static_cast<IServiceProvider*>(this));
	hr = m_explorer->Advise(static_cast<IExplorerBrowserEvents*>(this), &m_cookie);

	// フレームを表示する。
	hr = m_explorer->SetOptions(EBO_SHOWFRAMES | EBO_NOPERSISTVIEWSTATE);

	{
		PIDLIST_ABSOLUTE pidlDesktop;
		::SHGetKnownFolderIDList(FOLDERID_Desktop, 0, NULL, &pidlDesktop);
		m_explorer->BrowseToIDList(pidlDesktop, SBSP_ABSOLUTE);
		::ILFree(pidlDesktop);
	}

	IShellBrowserPtr browser = m_explorer;
	HWND hwnd = 0;
	browser->GetWindow(&hwnd);
	::PostMessage(theApp.getFilterWindow(), WM_AVIUTL_OBJECT_EXPLORER_INITED, (WPARAM)hwnd, 0);

	return TRUE;
}

int COutProcessDialog::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("COutProcessDialog::OnCreate()\n"));

	if (CDialogEx::OnCreate(cs) == -1)
		return -1;

	WCHAR filePath[MAX_PATH] = {};
	::GetModuleFileNameW(AfxGetInstanceHandle(), filePath, MAX_PATH);
	::PathRemoveExtensionW(filePath);
	::StringCbCatW(filePath, sizeof(filePath), L"SystemSettings.xml");
	MY_TRACE_WSTR(filePath);

	m_fileUpdateChecker = CFileUpdateCheckerPtr(new CFileUpdateChecker(filePath));

	SetTimer(TIMER_ID_CHECK_SETTING_FILE, 1000, 0);
#ifdef OBJECT_EXPLORER_CHECK_MAIN_PROCESS
	SetTimer(TIMER_ID_CHECK_MAIN_PROCESS, 1000, 0);
#endif
	return 0;
}

void COutProcessDialog::OnDestroy()
{
	MY_TRACE(_T("COutProcessDialog::OnDestroy()\n"));

	KillTimer(TIMER_ID_CHECK_MAIN_PROCESS);

	m_explorer->Unadvise(m_cookie);
	m_explorer->Destroy();
	m_explorer = 0;

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
			OnClickedPrev();
			break;
		}
	case APPCOMMAND_BROWSER_FORWARD:
		{
			OnClickedNext();
			break;
		}
	}

	__super::OnAppCommand(pWnd, nCmd, nDevice, nKey);
}

void COutProcessDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
	MY_TRACE(_T("COutProcessDialog::OnContextMenu()\n"));

	CMenu menu; menu.LoadMenuA(IDR_POPUP_MENU);
	CMenu* subMenu = menu.GetSubMenu(0);
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

	ShowWindow(SW_HIDE);

//	__super::OnCancel();
}

void COutProcessDialog::OnClickedPrev()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedPrev()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_NAVIGATEBACK);
}

void COutProcessDialog::OnClickedNext()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedNext()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_NAVIGATEFORWARD);
}

void COutProcessDialog::OnClickedUp()
{
	MY_TRACE(_T("COutProcessDialog::OnClickedUp()\n"));

	m_explorer->BrowseToIDList(NULL, SBSP_PARENT);
}

void COutProcessDialog::OnClickedGet()
{
	::PostMessage(theApp.getFilterWindow(), WM_AVIUTL_OBJECT_EXPLORER_GET, (WPARAM)GetSafeHwnd(), (LPARAM)m_url.GetSafeHwnd());
}

void COutProcessDialog::OnAddFavorite()
{
	MY_TRACE(_T("COutProcessDialog::OnAddFavorite()\n"));

	int index = m_url.FindString(0, m_currentFolderPath);
	if (index == CB_ERR)
		m_url.AddString(m_currentFolderPath);
}

void COutProcessDialog::OnDeleteFavorite()
{
	MY_TRACE(_T("COutProcessDialog::OnDeleteFavorite()\n"));

	int index = m_url.FindString(0, m_currentFolderPath);
	if (index != CB_ERR)
		m_url.DeleteString(index);
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
