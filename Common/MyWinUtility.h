#pragma once

//--------------------------------------------------------------------

inline int getWidth(const RECT& rc)
{
	return rc.right - rc.left;
}

inline int getHeight(const RECT& rc)
{
	return rc.bottom - rc.top;
}

inline void GetMonitorRect(POINT point, RECT& rect)
{
	// ウィンドウが表示されているディスプレイ情報を取得する
	HMONITOR monitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitor_info = {};
	monitor_info.cbSize = sizeof(monitor_info);
	::GetMonitorInfo(monitor, &monitor_info);
	rect = monitor_info.rcWork;
}

inline void GetMonitorRect(HWND hWnd, RECT& rect)
{
	// ウィンドウが表示されているディスプレイ情報を取得する
	HMONITOR monitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitor_info = {};
	monitor_info.cbSize = sizeof(monitor_info);
	::GetMonitorInfo(monitor, &monitor_info);
	rect = monitor_info.rcWork;
}

//--------------------------------------------------------------------

inline HANDLE createFileForRead(LPCTSTR filePath)
{
	return ::CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
}

inline HANDLE createFileForWrite(LPCTSTR filePath)
{
	return ::CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
}

inline HANDLE createFileForEdit(LPCTSTR filePath)
{
	return ::CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
}

inline HANDLE createFileForReadWrite(LPCTSTR filePath)
{
	return ::CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
}

inline DWORD readFile(HANDLE file, LPVOID buffer, DWORD numberOfBytesToRead, LPOVERLAPPED overlapped = 0)
{
	if (!::ReadFile(file, buffer, numberOfBytesToRead, &numberOfBytesToRead, overlapped))
		return 0;
	return numberOfBytesToRead;
}

inline DWORD writeFile(HANDLE file, LPCVOID buffer, DWORD numberOfBytesToWrite, LPOVERLAPPED overlapped = 0)
{
	if (!::WriteFile(file, buffer, numberOfBytesToWrite, &numberOfBytesToWrite, overlapped))
		return 0;
	return numberOfBytesToWrite;
}

inline DWORD writeFile(HANDLE file, LPCSTR text, LPOVERLAPPED overlapped = 0)
{
	DWORD numberOfBytesToWrite = 0;
	if (!::WriteFile(file, text, ::lstrlenA(text) * sizeof(text[0]), &numberOfBytesToWrite, overlapped))
		return 0;
	return numberOfBytesToWrite;
}

inline DWORD writeFile(HANDLE file, LPCWSTR text, LPOVERLAPPED overlapped = 0)
{
	DWORD numberOfBytesToWrite = 0;
	if (!::WriteFile(file, text, ::lstrlenW(text) * sizeof(text[0]), &numberOfBytesToWrite, overlapped))
		return 0;
	return numberOfBytesToWrite;
}

//--------------------------------------------------------------------

class Handle
{
private:

	HANDLE m_handle;

public:

	Handle()
	{
		m_handle = 0;
	}

	Handle(HANDLE handle)
	{
		m_handle = handle;
	}

	~Handle()
	{
		::CloseHandle(m_handle);
	}

private:

	Handle(const Handle& x)
	{
	}

public:

	operator HANDLE() const
	{
		return m_handle;
	}

	Handle& operator=(HANDLE handle)
	{
		if (m_handle)
			::CloseHandle(m_handle);

		m_handle = handle;

		return *this;
	}

	HANDLE* operator&()
	{
		return &m_handle;
	}
};

//--------------------------------------------------------------------

class FormatText
{
public:

	TCHAR m_buffer[MAX_PATH];

public:

	FormatText(int value)
	{
		::StringCbPrintf(m_buffer, sizeof(m_buffer), _T("%d"), value);
	}

	FormatText(double value)
	{
		::StringCbPrintf(m_buffer, sizeof(m_buffer), _T("%.10g"), value);
	}

	FormatText(LPCTSTR format, ...)
	{
		va_list va;
		va_start(va, format);
		::StringCbVPrintf(m_buffer, sizeof(m_buffer), format, va);
		va_end(va);
	}

	operator LPCTSTR() const
	{
		return m_buffer;
	}
};

//--------------------------------------------------------------------

class SimpleFileMapping
{
public:

	HANDLE m_handle;
	BYTE* m_buffer;

public:

	SimpleFileMapping(DWORD size, LPCTSTR name)
		: m_handle(0)
		, m_buffer(0)
	{
		MY_TRACE(_T("SimpleFileMapping::SimpleFileMapping(%d, %s)\n"), size, name);

		SEC_COMMIT;
		m_handle = ::CreateFileMapping(
			INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, name);

		if (!m_handle)
		{
			MY_TRACE(_T("::CreateSimpleFileMapping() failed.\n"));
			return;
		}

		m_buffer = (BYTE*)::MapViewOfFile(
			m_handle, FILE_MAP_WRITE, 0, 0, 0);

		if (!m_buffer)
		{
			MY_TRACE(_T("::MapViewOfFile() failed.\n"));
			return;
		}
	}

	~SimpleFileMapping()
	{
		::UnmapViewOfFile(m_buffer), m_buffer = 0;
		::CloseHandle(m_handle), m_handle = 0;
	}

	BYTE* getBuffer()
	{
		return m_buffer;
	}
};

typedef std::shared_ptr<SimpleFileMapping> SimpleFileMappingPtr;

//--------------------------------------------------------------------

class FileMapping
{
public:

	HANDLE m_handle;

public:

	FileMapping(DWORD size, LPCTSTR name)
		: m_handle(0)
	{
		MY_TRACE(_T("FileMapping::FileMapping(%d, %s)\n"), size, name);


		m_handle = ::CreateFileMapping(
			INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, name);

		if (!m_handle)
		{
			MY_TRACE(_T("::CreateFileMapping() failed.\n"));
			return;
		}
	}

	~FileMapping()
	{
		::CloseHandle(m_handle), m_handle = 0;
	}

	BYTE* allocBuffer()
	{
		return (BYTE*)::MapViewOfFile(
			m_handle, FILE_MAP_WRITE, 0, 0, 0);
	}

	void freeBuffer(BYTE* buffer)
	{
		::UnmapViewOfFile(buffer);
	}

public:

	class Buffer
	{
	public:

		FileMapping* m_fileMapping;
		BYTE* m_buffer;

		Buffer(FileMapping* fileMapping)
		{
			m_fileMapping = fileMapping;
			m_buffer = m_fileMapping->allocBuffer();
		}

		~Buffer()
		{
			m_fileMapping->freeBuffer(m_buffer), m_buffer = 0;
		}

		BYTE* get()
		{
			return m_buffer;
		}
	};
};

typedef std::shared_ptr<FileMapping> FileMappingPtr;

//--------------------------------------------------------------------

class Sync
{
public:

	virtual DWORD lock(DWORD timeOut = INFINITE) = 0;
	virtual BOOL unlock() = 0;
};

class Synchronizer
{
public:

	Synchronizer(Sync& sync, DWORD timeOut = INFINITE)
		: m_sync(sync)
	{
		m_sync.lock(timeOut);
	}

	virtual ~Synchronizer()
	{
		m_sync.unlock();
	}

protected:

	Sync& m_sync;
};

class Mutex : public Sync
{
public:

	Mutex()
	{
		m_mutex = 0;
	}

	Mutex(LPSECURITY_ATTRIBUTES sa, BOOL initialOwner, LPCTSTR name)
	{
		init(sa, initialOwner, name);
	}

	virtual ~Mutex()
	{
		::CloseHandle(m_mutex);
	}

protected:

	HANDLE m_mutex;

public:

	void init(LPSECURITY_ATTRIBUTES sa, BOOL initialOwner, LPCTSTR name)
	{
		m_mutex = ::CreateMutex(sa, initialOwner, name);
	}

	virtual DWORD lock(DWORD timeOut = INFINITE)
	{
		return ::WaitForSingleObject(m_mutex, timeOut);
	}

	virtual BOOL unlock()
	{
		::ReleaseMutex(m_mutex);

		return true;
	}
};

//--------------------------------------------------------------------

class CFileUpdateChecker
{
private:

	WCHAR m_filePath[MAX_PATH];
	FILETIME m_fileTime;

public:

	CFileUpdateChecker(LPCWSTR filePath)
	{
		MY_TRACE(_T("CFileUpdateChecker::CFileUpdateChecker(%ws)\n"), filePath);

		::StringCbCopyW(m_filePath, sizeof(m_filePath), filePath);
		getFileTime(filePath, &m_fileTime);
	}

	LPCWSTR getFilePath() const
	{
		return m_filePath;
	}

	BOOL isFileUpdated()
	{
//		MY_TRACE(_T("CFileUpdateChecker::isFileUpdated()\n"));

		FILETIME fileTime;

		if (!getFileTime(m_filePath, &fileTime))
			return FALSE;

		if (!::CompareFileTime(&m_fileTime, &fileTime))
			return FALSE;

		m_fileTime = fileTime;

		return TRUE;
	}

	static BOOL getFileTime(LPCWSTR filePath, FILETIME* fileTime)
	{
		HANDLE file = ::CreateFileW(filePath, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

		if (file == INVALID_HANDLE_VALUE)
			return FALSE;

		BOOL retValue = ::GetFileTime(file, 0, 0, fileTime);

		::CloseHandle(file);

		return retValue;
	}
};

typedef std::shared_ptr<CFileUpdateChecker> CFileUpdateCheckerPtr;

//--------------------------------------------------------------------
