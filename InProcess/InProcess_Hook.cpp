#include "pch.h"
#include "InProcess.h"
#include "InProcess_Hook.h"

//--------------------------------------------------------------------

BOOL g_loadAlias = FALSE;
std::vector<std::string> g_aliasFileNames;

//--------------------------------------------------------------------

IMPLEMENT_INTERNAL_PROC(BOOL,  CDECL, Exedit_SaveFilterAlias, (int objectIndex, int filterIndex, LPCSTR fileName));

HWND* g_exeditWindow = 0;
HWND* g_settingDialog = 0;
auls::EXEDIT_OBJECT** g_objectData = 0; // オブジェクトデータへのポインタ。
BYTE** g_objectExdata = 0; // オブジェクト拡張データへのポインタ。
auls::EXEDIT_FILTER** g_filterTable = 0; // フィルタ配列。
int* g_objectIndex = 0; // カレントオブジェクトのインデックスへのポインタ。
int* g_filterIndex = 0; // カレントフィルタのインデックスへのポインタ。
int* g_nextObject = 0; // 次のオブジェクトの配列へのポインタ。
int* g_selectedObjects = 0;
int* g_selectedObjectsCount = 0;

//--------------------------------------------------------------------

HWND Exedit_GetWindow()
{
	return *g_exeditWindow;
}

HWND Exedit_SettingDialog()
{
	return *g_settingDialog;
}

int Exedit_GetCurrentObjectIndex()
{
	return *g_objectIndex;
}

int Exedit_GetCurrentFilterIndex()
{
	return *g_filterIndex;
}

auls::EXEDIT_OBJECT* Exedit_GetObject(int objectIndex)
{
	return *g_objectData + objectIndex;
}

auls::EXEDIT_FILTER* Exedit_GetFilter(int filterId)
{
	return g_filterTable[filterId];
}

auls::EXEDIT_FILTER* Exedit_GetFilter(auls::EXEDIT_OBJECT* object, int filterIndex)
{
	if (!object) return 0;
	int id = object->filter_param[filterIndex].id;
	if (id < 0) return 0;
	return Exedit_GetFilter(id);
}

int Exedit_GetNextObjectIndex(int objectIndex)
{
	return g_nextObject[objectIndex];
}

int Exedit_GetSelectedObjects(int i)
{
	return g_selectedObjects[i];
}

int Exedit_GetSelectedObjectsCount()
{
	return *g_selectedObjectsCount;
}

//--------------------------------------------------------------------

// 拡張編集のフックをセットする。
void initExeditHook()
{
	MY_TRACE(_T("initExeditHook()\n"));

	if (g_settingDialog)
		return;

	// 拡張編集の関数や変数を取得する。
	DWORD exedit = (DWORD)::GetModuleHandle(_T("exedit.auf"));
	g_exeditWindow = (HWND*)(exedit + 0x177A44);
	g_settingDialog = (HWND*)(exedit + 0x1539C8);
	g_objectData = (auls::EXEDIT_OBJECT**)(exedit + 0x1E0FA4);
	g_objectExdata = (BYTE**)(exedit + 0x1E0FA8);
	g_filterTable = (auls::EXEDIT_FILTER**)(exedit + 0x187C98);
	g_objectIndex = (int*)(exedit + 0x177A10);
	g_filterIndex = (int*)(exedit + 0x14965C);
	g_nextObject = (int*)(exedit + 0x1592d8);
	g_selectedObjects = (int*)(exedit + 0x179230);
	g_selectedObjectsCount = (int*)(exedit + 0x167D88);
	true_Exedit_WndProc = (Type_Exedit_WndProc)(exedit + 0x3B620);
	true_Exedit_SettingDialog_WndProc = (Type_Exedit_SettingDialog_WndProc)(exedit + 0x2CDE0);
	true_Exedit_GetAliasFileName = (Type_Exedit_GetAliasFileName)(exedit + 0x43FD0);
	true_Exedit_AddAlias = (Type_Exedit_AddAlias)(exedit + 0x29DC0);
	Exedit_SaveFilterAlias = (Type_Exedit_SaveFilterAlias)(exedit + 0x28CA0);

	// 拡張編集の関数をフックする。
	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

	ATTACH_HOOK_PROC(Exedit_WndProc);
	ATTACH_HOOK_PROC(Exedit_SettingDialog_WndProc);
	ATTACH_HOOK_PROC(Exedit_GetAliasFileName);
	ATTACH_HOOK_PROC(Exedit_AddAlias);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("拡張編集のフックに成功しました\n"));
	}
	else
	{
		MY_TRACE(_T("拡張編集のフックに失敗しました\n"));
	}
}

void getPath(LPTSTR buffer)
{
	int objectIndex = Exedit_GetCurrentObjectIndex();
	if (objectIndex < 0) return;
	auls::EXEDIT_OBJECT* object = Exedit_GetObject(objectIndex);
	if (!object) return;

	int filterIndex = 0;
	auls::EXEDIT_FILTER* filter = Exedit_GetFilter(object, filterIndex);
	if (!filter) return;

	int offset = -1, size = -1;
	int id = object->filter_param[filterIndex].id;
	switch (id)
	{
	case 0: offset = 0; size = 260; break; // 動画ファイル
	case 1: offset = 4; size = 256; break; // 画像ファイル
	case 2: offset = 0; size = 260; break; // 音声ファイル
	case 6: offset = 0; size = 260; break; // 音声波形表示
	case 82: offset = 0; size = 260; break; // 動画ファイル合成
	case 83: offset = 4; size = 256; break; // 画像ファイル合成
	}
	if (size <= 0) return;

	BYTE* exdataTable = *g_objectExdata;
	DWORD exoffset = object->ExdataOffset(filterIndex);
	BYTE* exdata = exdataTable + exoffset + 0x0004;
	LPCSTR fileName = (LPCSTR)(exdata + offset);

	::StringCchPrintf(buffer, MAX_PATH, _T("%hs"), fileName);
	::PathRemoveFileSpec(buffer);
}

void clearAliasFile()
{
	g_aliasFileNames.clear();
}

BOOL addAliasFile(LPCSTR fileName)
{
	BOOL retValue = FALSE;

	char tempPath[MAX_PATH] = {};
	::GetTempPathA(MAX_PATH, tempPath);
	MY_TRACE_STR(tempPath);

	DWORD pid = ::GetCurrentProcessId();
	MY_TRACE_INT(pid);

	std::vector<char> section;

	for (int i = 0; i < auls::EXEDIT_OBJECT::MAX_FILTER; i++)
	{
		char appName[MAX_PATH] = {};
		::StringCbPrintfA(appName, sizeof(appName), _T("vo.%d"), i);
		MY_TRACE_TSTR(appName);

		char name[MAX_PATH] = {};
		::GetPrivateProfileStringA(appName, "_name", "", name, MAX_PATH, fileName);
		MY_TRACE_TSTR(name);
		if (::lstrlenA(name) == 0) continue;
		else if (::lstrcmpA(name, "動画ファイル") == 0) continue;
		else if (::lstrcmpA(name, "画像ファイル") == 0) continue;
		else if (::lstrcmpA(name, "音声ファイル") == 0) continue;
		else if (::lstrcmpA(name, "テキスト") == 0) continue;
		else if (::lstrcmpA(name, "図形") == 0) continue;
		else if (::lstrcmpA(name, "フレームバッファ") == 0) continue;
		else if (::lstrcmpA(name, "音声波形") == 0) continue;
		else if (::lstrcmpA(name, "シーン") == 0) continue;
		else if (::lstrcmpA(name, "シーン(音声)") == 0) continue;
		else if (::lstrcmpA(name, "直前オブジェクト") == 0) continue;
		else if (::lstrcmpA(name, "標準描画") == 0) continue;
		else if (::lstrcmpA(name, "拡張描画") == 0) continue;
		else if (::lstrcmpA(name, "標準再生") == 0) continue;
		else if (::lstrcmpA(name, "パーティクル出力") == 0) continue;
		else if (::lstrcmpA(name, "カスタムオブジェクト") == 0) continue;
		else if (::lstrcmpA(name, "時間制御") == 0) continue;
		else if (::lstrcmpA(name, "グループ制御") == 0) continue;
		else if (::lstrcmpA(name, "カメラ制御") == 0) continue;

		section.clear();
		section.resize(32767, 0);
		DWORD size = ::GetPrivateProfileSectionA(appName, section.data(), section.size(), fileName);
		section.resize(size);
		std::replace(section.begin(), section.end(), '\0', '\n');

		char sectionHeader[MAX_PATH] = {};
		::StringCbPrintfA(sectionHeader, sizeof(sectionHeader), "[vo.0]\n");
		MY_TRACE_STR(sectionHeader);

		char tempFileName[MAX_PATH] = {};
		::StringCbPrintfA(tempFileName, sizeof(tempFileName), _T("%s\\AviUtl_ObjectExplorer_%d_%d.exa"), tempPath, pid, i);
		MY_TRACE_STR(tempFileName);

		Handle file = createFileForWrite(tempFileName);
		writeFile(file, sectionHeader);
		writeFile(file, section.data(), section.size());

		g_aliasFileNames.push_back(tempFileName);

		retValue = TRUE;
	}

	return retValue;
}

void loadAlias()
{
	MY_TRACE(_T("loadAlias()\n"));

	g_loadAlias = TRUE;
	::SendMessage(Exedit_GetWindow(), WM_COMMAND, 1036, 1);
	g_loadAlias = FALSE;
}

LPCSTR getName(auls::EXEDIT_OBJECT* object, int filterIndex)
{
	if (filterIndex < 0)
	{
		if (::lstrlenA(object->dispname))
			return object->dispname;

		filterIndex = 0;
	}

	auls::EXEDIT_FILTER* filter = Exedit_GetFilter(object, filterIndex);
	if (!filter) return "";

	int id = object->filter_param[filterIndex].id;
	if (id == 79) // アニメーション効果
	{
		BYTE* exdataTable = *g_objectExdata;
		DWORD offset = object->ExdataOffset(filterIndex);
		BYTE* exdata = exdataTable + offset + 0x0004;
		LPCSTR name = (LPCSTR)(exdata + 0x04);
		if (!name[0])
		{
			WORD type = *(WORD*)(exdata + 0);
			MY_TRACE_HEX(type);

			WORD filter = *(WORD*)(exdata + 2);
			MY_TRACE_HEX(filter);

			switch (type)
			{
			case 0x00: name = "震える"; break;
			case 0x01: name = "振り子"; break;
			case 0x02: name = "弾む"; break;
			case 0x03: name = "座標の拡大縮小(個別オブジェクト)"; break;
			case 0x04: name = "画面外から登場"; break;
			case 0x05: name = "ランダム方向から登場"; break;
			case 0x06: name = "拡大縮小して登場"; break;
			case 0x07: name = "ランダム間隔で落ちながら登場"; break;
			case 0x08: name = "弾んで登場"; break;
			case 0x09: name = "広がって登場"; break;
			case 0x0A: name = "起き上がって登場"; break;
			case 0x0B: name = "何処からともなく登場"; break;
			case 0x0C: name = "反復移動"; break;
			case 0x0D: name = "座標の回転(個別オブジェクト)"; break;
			case 0x0E: name = "立方体(カメラ制御)"; break;
			case 0x0F: name = "球体(カメラ制御)"; break;
			case 0x10: name = "砕け散る"; break;
			case 0x11: name = "点滅"; break;
			case 0x12: name = "点滅して登場"; break;
			case 0x13: name = "簡易変形"; break;
			case 0x14: name = "簡易変形(カメラ制御)"; break;
			case 0x15: name = "リール回転"; break;
			case 0x16: name = "万華鏡"; break;
			case 0x17: name = "円形配置"; break;
			case 0x18: name = "ランダム配置"; break;
			default: name = "アニメーション効果"; break;
			}
		}
		return name;
	}
	else if (id == 80) // カスタムオブジェクト
	{
		BYTE* exdataTable = *g_objectExdata;
		DWORD offset = object->ExdataOffset(filterIndex);
		BYTE* exdata = exdataTable + offset + 0x0004;
		LPCSTR name = (LPCSTR)(exdata + 0x04);
		if (!name[0]) name = "カスタムオブジェクト";
		return name;
	}
	else
	{
		return filter->name;
	}
}

BOOL saveAlias(HWND hwnd, int objectIndex, int filterIndex)
{
	MY_TRACE(_T("saveAlias(0x%08X, %d, %d)\n"), hwnd, objectIndex, filterIndex);

	TCHAR title[MAX_PATH] = "";
	TCHAR folderName[MAX_PATH] = "";
	TCHAR fileName[MAX_PATH] = "";
	TCHAR fileFilter[MAX_PATH] = "エイリアスファイル (*.exa)\0*.exa\0" "すべてのファイル (*.*)\0*.*\0";

	::GetWindowText(theApp.m_browser, folderName, MAX_PATH);

	auls::EXEDIT_OBJECT* object = Exedit_GetObject(objectIndex);
	if (!object) return FALSE;

	LPCSTR name = getName(object, filterIndex);

	switch (filterIndex)
	{
	case -2:
		{
			::StringCbCopy(title, sizeof(title), _T("オブジェクトの保存"));
			::StringCbPrintf(fileName, sizeof(fileName), _T("%hs.exa"), name);

			break;
		}
	case -1:
		{
			::StringCbCopy(title, sizeof(title), _T("全フィルタの保存"));
			::StringCbPrintf(fileName, sizeof(fileName), _T("%hsのフィルタ全体.exa"), object->dispname);

			break;
		}
	default:
		{
			::StringCbCopy(title, sizeof(title), _T("フィルタの保存"));
			::StringCbPrintf(fileName, sizeof(fileName), _T("%hs.exa"), name);

			break;
		}
	}

	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.hwndOwner = hwnd;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = folderName;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = fileFilter;
	ofn.lpstrDefExt = _T("exa");

	if (!::GetSaveFileName(&ofn))
		return FALSE;

	return Exedit_SaveFilterAlias(objectIndex, filterIndex, fileName);
}

//--------------------------------------------------------------------

IMPLEMENT_HOOK_PROC_NULL(LRESULT, CDECL, Exedit_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp))
{
	switch (message)
	{
	case WM_COMMAND:
		{
			switch (wParam)
			{
			case 1070: // エイリアスを作成
				{
					MY_TRACE(_T("エイリアスを作成します\n"));

					if (Exedit_GetSelectedObjectsCount() > 0)
					{
						int objectIndex = Exedit_GetSelectedObjects(0);

						saveAlias(hwnd, objectIndex, -2);
					}

					return 0;
				}
			}

			break;
		}
	}

	return true_Exedit_WndProc(hwnd, message, wParam, lParam, editp, fp);
}

IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
{
	switch (message)
	{
	case WM_COMMAND:
		{
			switch (wParam)
			{
			case 1109: // 『設定の保存』→『現在の設定でエイリアスを作成する』
				{
					MY_TRACE(_T("エイリアスを作成します\n"));

					int objectIndex = Exedit_GetCurrentObjectIndex();
					int filterIndex = Exedit_GetCurrentFilterIndex();

					if (filterIndex <= 0)
					{
						saveAlias(hwnd, objectIndex, -1);
					}
					else
					{
						saveAlias(hwnd, objectIndex, filterIndex);
					}

					return 0;
				}
			}

			break;
		}
	case WM_DROPFILES:
		{
			MY_TRACE(_T("WM_DROPFILES\n"));

			clearAliasFile();
			BOOL result = FALSE;

			HDROP drop = (HDROP)wParam;
			int c = ::DragQueryFile(drop, 0xFFFFFFFF, 0, 0);
			for (int i = 0; i < c; i++)
			{
				TCHAR fileName[MAX_PATH] = {};
				::DragQueryFile(drop, i, fileName, MAX_PATH);
				MY_TRACE_TSTR(fileName);

				LPCTSTR extension = ::PathFindExtension(fileName);
				if (::lstrcmpi(extension, _T(".exa")) == 0)
					result |= addAliasFile(fileName);
			}
			::DragFinish(drop);

			if (result)
			{
				loadAlias();
				return 0;
			}

			break;
		}
	}

	return true_Exedit_SettingDialog_WndProc(hwnd, message, wParam, lParam);
}

IMPLEMENT_HOOK_PROC_NULL(LPCSTR, CDECL, Exedit_GetAliasFileName, (int aliasId))
{
	MY_TRACE(_T("Exedit_GetAliasFileName(%d)\n"), aliasId);

	return true_Exedit_GetAliasFileName(aliasId);
}

IMPLEMENT_HOOK_PROC_NULL(BOOL, CDECL, Exedit_AddAlias, (LPCSTR fileName, BOOL flag1, BOOL flag2, int objectIndex))
{
	MY_TRACE(_T("Exedit_AddAlias(%s, %d, %d, %d)\n"), fileName, flag1, flag2, objectIndex);

	if (g_loadAlias)
	{
		BOOL retValue = FALSE;

		for (auto fileName : g_aliasFileNames)
		{
			MY_TRACE_STR(fileName.c_str());

			retValue |= true_Exedit_AddAlias(fileName.c_str(), flag1, flag2, objectIndex);

			::DeleteFileA(fileName.c_str());
		}

		return retValue;
	}

	return true_Exedit_AddAlias(fileName, flag1, flag2, objectIndex);
}

//--------------------------------------------------------------------
