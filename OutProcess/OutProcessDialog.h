#pragma once

//--------------------------------------------------------------------

class COutProcessDialog
	: public CDialogEx
	, public IServiceProvider
	, public IExplorerBrowserEvents
	, public IExplorerPaneVisibility
	, public IFolderFilter
{
public:

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppv)
	{
		if (::IsEqualIID(iid, IID_IUnknown))
		{
			*ppv = static_cast<IUnknown*>(static_cast<IServiceProvider*>(this));
		}
		else if (::IsEqualIID(iid, IID_IServiceProvider))
		{
			*ppv = static_cast<IServiceProvider*>(this);
		}
		else if (::IsEqualIID(iid, IID_IExplorerBrowserEvents))
		{
			*ppv = static_cast<IExplorerBrowserEvents*>(this);
		}
		else if (::IsEqualIID(iid, IID_IExplorerPaneVisibility))
		{
			*ppv = static_cast<IExplorerPaneVisibility*>(this);
		}
		else if (::IsEqualIID(iid, IID_IFolderFilter))
		{
			*ppv = static_cast<IFolderFilter*>(this);
		}
		else
		{
			*ppv = 0;
			return E_NOINTERFACE;
		}

		this->AddRef();
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}

	virtual ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID service, REFIID iid, void** ppv)
	{
		if (::IsEqualIID(service, IID_IExplorerPaneVisibility))
		{
			return this->QueryInterface(iid, ppv);
		}

		*ppv = 0;
		return E_NOINTERFACE;
	}

	virtual HRESULT STDMETHODCALLTYPE OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder)
	{
		MY_TRACE(_T("COutProcessDialog::OnNavigationPending()\n"));

		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnViewCreated(IShellView* shellView)
	{
		MY_TRACE(_T("COutProcessDialog::OnViewCreated()\n"));

		m_shellView = shellView;

		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder)
	{
		MY_TRACE(_T("COutProcessDialog::OnNavigationComplete()\n"));

		TCHAR path[MAX_PATH] = {};
		::SHGetPathFromIDList(pidlFolder, path);
		m_currentFolderPath = path;
		m_url.SetWindowText(path);
		m_url.SetEditSel(0, -1);

		IShellBrowserPtr browser = m_explorer;
		HWND hwnd = 0;
		browser->GetWindow(&hwnd);
		::SetWindowText(hwnd, path);

		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE OnNavigationFailed(PCIDLIST_ABSOLUTE pidlFolder)
	{
		MY_TRACE(_T("COutProcessDialog::OnNavigationFailed()\n"));

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps)
	{
		MY_TRACE(_T("COutProcessDialog::GetPaneState()\n"));

		if (ep == EP_NavPane)
			*peps = EPS_DEFAULT_ON;
		else
			*peps = EPS_DEFAULT_OFF | EPS_FORCE;

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ShouldShow( 
		/* [in] */ __RPC__in_opt IShellFolder *psf,
		/* [unique][in] */ __RPC__in_opt PCIDLIST_ABSOLUTE pidlFolder,
		/* [in] */ __RPC__in PCUITEMID_CHILD pidlItem)
	{
		IShellItemPtr si;
		HRESULT hr = ::SHCreateItemWithParent(0, psf, pidlItem, IID_PPV_ARGS(&si));

		LPWSTR displayName = 0;
		si->GetDisplayName(SIGDN_NORMALDISPLAY, &displayName);

		SFGAOF attributes = 0;
		si->GetAttributes(SFGAO_FOLDER, &attributes);

		MY_TRACE(_T("COutProcessDialog::ShouldShow(%ws, 0x%08X)\n"), displayName, attributes);

		WCHAR searchString[MAX_PATH] = {};
		::GetWindowTextW(m_search, searchString, MAX_PATH);

		HRESULT retValue = S_OK;

		if (attributes & SFGAO_FOLDER)
		{
			// フォルダの場合
		}
		else
		{
			// ファイルの場合

			if (::lstrlenW(searchString) != 0)
				retValue = ::StrStrIW(displayName, searchString) ? S_OK : S_FALSE;
		}

		::CoTaskMemFree(displayName);

		return retValue;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetEnumFlags( 
		/* [in] */ __RPC__in_opt IShellFolder *psf,
		/* [in] */ __RPC__in PCIDLIST_ABSOLUTE pidlFolder,
		/* [out] */ __RPC__deref_out_opt HWND *phwnd,
		/* [out][in] */ __RPC__inout DWORD *pgrfFlags)
	{
		MY_TRACE(_T("COutProcessDialog::GetEnumFlags()\n"));

		return S_OK;
	}

public:

	static const int TIMER_ID_CHECK_MAIN_PROCESS = 2021;
	static const int TIMER_ID_CHECK_SETTING_FILE = 2022;

public:

	FileUpdateCheckerPtr m_fileUpdateChecker;
	IExplorerBrowserPtr m_explorer;
	DWORD m_cookie;
	IShellView* m_shellView;
	CString m_currentFolderPath;
	CComboBox m_url;
	CEdit m_search;
	BOOL m_isSettingsLoaded;
	BOOL m_isNavPaneVisible;
	BOOL m_isVoiceEnabled;
	BOOL m_usesCommonDialog;

public:

	COutProcessDialog(CWnd* pParent = nullptr);
	virtual ~COutProcessDialog();

	void loadSystemSettings();

	void loadSettings();
	void saveSettings();

	void createExplorer();
	void destroyExplorer();
	void browseToPath(LPCTSTR path);
	void playVoice(LPCTSTR voice);

protected:

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OUT_PROCESS };
#endif
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAppCommand(CWnd* pWnd, UINT nCmd, UINT nDevice, UINT nKey);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClickedPrev();
	afx_msg void OnClickedNext();
	afx_msg void OnClickedUp();
	afx_msg void OnClickedGet();
	afx_msg void OnAddFavorite();
	afx_msg void OnDeleteFavorite();
	afx_msg void OnShowNavPane();
	afx_msg void OnPlayVoice();
	afx_msg void OnUseCommonDialog();
	afx_msg void OnUpdateShowNavPane(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlayVoice(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUseCommonDialog(CCmdUI* pCmdUI);
	afx_msg void OnSelChangeUrl();
	afx_msg LRESULT OnObjectExplorerResize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnObjectExplorerExit(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
