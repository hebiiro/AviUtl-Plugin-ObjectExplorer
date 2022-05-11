#include "pch.h"
#include "InProcess.h"
#include "InProcess_Hook.h"

//--------------------------------------------------------------------

AviUtlInternal g_auin;

BOOL g_loadAlias = FALSE; // 独自のエイリアス読み込み処理を行うためのフラグ。
std::vector<std::string> g_aliasFileNames; // エイリアス読み込みに使用するファイル名の配列。

//--------------------------------------------------------------------

// フックをセットする。
void initHook(FILTER* fp)
{
	MY_TRACE(_T("initHook()\n"));

	g_auin.init();

	// 拡張編集の関数や変数を取得する。
	DWORD exedit = (DWORD)::GetModuleHandle(_T("exedit.auf"));
//	true_ExeditWindowProc = (Type_ExeditWindowProc)(exedit + 0x3B620);
//	true_SettingDialogProc = (Type_SettingDialogProc)(exedit + 0x2CDE0);
	true_ExeditWindowProc = g_auin.HookExeditWindowProc(fp, hook_ExeditWindowProc);
	true_SettingDialogProc = g_auin.HookSettingDialogProc(hook_SettingDialogProc);
	true_AddAlias = g_auin.GetAddAlias();

	// 拡張編集の関数をフックする。
	DetourTransactionBegin();
	DetourUpdateThread(::GetCurrentThread());

//	ATTACH_HOOK_PROC(ExeditWindowProc);
//	ATTACH_HOOK_PROC(SettingDialogProc);
	ATTACH_HOOK_PROC(AddAlias);

	if (DetourTransactionCommit() == NO_ERROR)
	{
		MY_TRACE(_T("拡張編集のフックに成功しました\n"));
	}
	else
	{
		MY_TRACE(_T("拡張編集のフックに失敗しました\n"));
	}
}

// フックを解除する。
void termHook(FILTER* fp)
{
	MY_TRACE(_T("termHook()\n"));
}

// カレントオブジェクトに関連するファイルパスを取得する。
void getPath(LPTSTR buffer)
{
	// カレントオブジェクトを取得する。
	int objectIndex = g_auin.GetCurrentObjectIndex();
	if (objectIndex < 0) return;
	ExEdit::Object* object = g_auin.GetObject(objectIndex);
	if (!object) return;

	// 先頭のフィルタを取得する。
	int filterIndex = 0;
	ExEdit::Filter* filter = g_auin.GetFilter(object, filterIndex);
	if (!filter) return;

	// 拡張データへのオフセットとサイズを取得する。
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
	if (size <= 0) return; // サイズを取得できなかった場合は何もしない。

	// 拡張データからファイル名を取得する。
	BYTE* exdata = g_auin.GetExdata(object, filterIndex);
	LPCSTR fileName = (LPCSTR)(exdata + offset);

	// 整形してバッファに格納する。
	::StringCchPrintf(buffer, MAX_PATH, _T("%hs"), fileName);
	::PathRemoveFileSpec(buffer);
}

// エイリアスファイル名の配列を消去する。
void clearAliasFile()
{
	g_aliasFileNames.clear();
}

// エイリアスファイルを読み込み、フィルタ毎に分解する。
// そして、それぞれのフィルタを一時ファイルに保存し、そのファイル名を配列に追加する。
BOOL addAliasFile(LPCSTR fileName)
{
	// 戻り値。
	BOOL retValue = FALSE;

	// テンポラリフォルダのパス。
	char tempPath[MAX_PATH] = {};
	::GetTempPathA(MAX_PATH, tempPath);
	MY_TRACE_STR(tempPath);

	// カレントプロセスの ID。
	DWORD pid = ::GetCurrentProcessId();
	MY_TRACE_INT(pid);

	// ini ファイル内のセクションを読み込むためのバッファ。
	std::vector<char> section;

	for (int i = 0; i < ExEdit::Object::MAX_FILTER; i++)
	{
		// セクションの appName を取得する。
		char appName[MAX_PATH] = {};
		::StringCbPrintfA(appName, sizeof(appName), "vo.%d", i);
		MY_TRACE_TSTR(appName);

		// セクション内の _name を取得し、読み込み可能なフィルタかチェックする。
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

		// セクションデータを取得する。
		section.clear();
		section.resize(32767, 0);
		DWORD size = ::GetPrivateProfileSectionA(appName, section.data(), section.size(), fileName);
		section.resize(size);
		std::replace(section.begin(), section.end(), '\0', '\n');

		// セクションのヘッダー。
		char sectionHeader[MAX_PATH] = {};
		::StringCbPrintfA(sectionHeader, sizeof(sectionHeader), "[vo.0]\n");
		MY_TRACE_STR(sectionHeader);

		// 一時ファイルのファイル名。
		char tempFileName[MAX_PATH] = {};
		::StringCbPrintfA(tempFileName, sizeof(tempFileName), "%s\\AviUtl_ObjectExplorer_%d_%d.exa", tempPath, pid, i);
		MY_TRACE_STR(tempFileName);

		// 一時ファイルにセクションヘッダーとセクションデータを書き込む。
		Handle file = createFileForWrite(tempFileName);
		writeFile(file, sectionHeader);
		writeFile(file, section.data(), section.size());

		// 配列に一時ファイルのファイル名を追加する。
		g_aliasFileNames.push_back(tempFileName);

		retValue = TRUE;
	}

	return retValue;
}

// アイテムにエイリアスを追加する。
// WM_DROPFILES のタイミングで呼ばれる。
void loadAlias()
{
	MY_TRACE(_T("loadAlias()\n"));

	g_loadAlias = TRUE;
	// この中で AddAlias() が呼ばれるのでフックする。
	// 1036 はエイリアスを追加するコマンド。1 はエイリアスのインデックス。
	::SendMessage(g_auin.GetExeditWindow(), WM_COMMAND, 1036, 1);
	g_loadAlias = FALSE;
}

// フィルタの名前を取得する。
LPCSTR getName(ExEdit::Object* object, int filterIndex)
{
	if (filterIndex < 0)
	{
		if (object->dispname[0])
			return object->dispname;

		filterIndex = 0;
	}

	ExEdit::Filter* filter = g_auin.GetFilter(object, filterIndex);
	if (!filter) return "";

	int id = object->filter_param[filterIndex].id;
	if (id == 79) // アニメーション効果
	{
		BYTE* exdata = g_auin.GetExdata(object, filterIndex);
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
		BYTE* exdata = g_auin.GetExdata(object, filterIndex);
		LPCSTR name = (LPCSTR)(exdata + 0x04);
		if (!name[0]) name = "カスタムオブジェクト";
		return name;
	}
	else
	{
		return filter->name;
	}
}

void replaceInvalidChar(LPTSTR fileName)
{
	int c = ::lstrlen(fileName);
	for (int i = 0; i < c; i++)
	{
		TCHAR ch = fileName[i];
#ifndef UNICODE
		if (::IsDBCSLeadByte(ch))
		{
			i++;
			continue;
		}
#endif
		UINT type = ::PathGetCharType(ch);
		switch (type)
		{
		case GCT_INVALID:
		case GCT_WILD:
			{
				fileName[i] = _T('_');
				break;
			}
		}
	}
}

// ファイル選択ダイアログを出してフィルタをエイリアスファイルに保存する。
BOOL saveAlias(HWND hwnd, int objectIndex, int filterIndex)
{
	MY_TRACE(_T("saveAlias(0x%08X, %d, %d)\n"), hwnd, objectIndex, filterIndex);

	TCHAR title[MAX_PATH] = "";
	TCHAR folderName[MAX_PATH] = "";
	TCHAR fileName[MAX_PATH] = "";
	TCHAR fileFilter[MAX_PATH] = "エイリアスファイル (*.exa)\0*.exa\0" "すべてのファイル (*.*)\0*.*\0";

	::GetWindowText(theApp.m_browser, folderName, MAX_PATH);

	ExEdit::Object* object = g_auin.GetObject(objectIndex);
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

	replaceInvalidChar(fileName);

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

	return g_auin.SaveFilterAlias(objectIndex, filterIndex, fileName);
}

//--------------------------------------------------------------------

IMPLEMENT_HOOK_PROC_NULL(LRESULT, CDECL, ExeditWindowProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp))
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

					if (g_auin.GetSelectedObjectsCount() > 0)
					{
						int objectIndex = g_auin.GetSelectedObjects(0);

						saveAlias(hwnd, objectIndex, -2);
					}

					return 0;
				}
			}

			break;
		}
	}

	return true_ExeditWindowProc(hwnd, message, wParam, lParam, editp, fp);
}

IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, SettingDialogProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam))
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

					int objectIndex = g_auin.GetCurrentObjectIndex();
					int filterIndex = g_auin.GetCurrentFilterIndex();

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
			//::DragFinish(drop); // これを呼ぶとデフォルトの処理が実行できなくなる。

			if (result)
			{
				loadAlias();
				return 0;
			}

			break;
		}
	}

	return true_SettingDialogProc(hwnd, message, wParam, lParam);
}

IMPLEMENT_HOOK_PROC_NULL(BOOL, CDECL, AddAlias, (LPCSTR fileName, BOOL flag1, BOOL flag2, int objectIndex))
{
	MY_TRACE(_T("AddAlias(%s, %d, %d, %d)\n"), fileName, flag1, flag2, objectIndex);

	if (g_loadAlias)
	{
		// フラグが立っている場合はエイリアスファイル名の配列を使用する。

		BOOL retValue = FALSE;

		for (auto fileName : g_aliasFileNames)
		{
			MY_TRACE_STR(fileName.c_str());

			retValue |= true_AddAlias(fileName.c_str(), flag1, flag2, objectIndex);

			::DeleteFileA(fileName.c_str());
		}

		return retValue;
	}

	return true_AddAlias(fileName, flag1, flag2, objectIndex);
}

//--------------------------------------------------------------------
