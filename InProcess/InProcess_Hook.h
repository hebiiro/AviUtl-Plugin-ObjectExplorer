﻿#pragma once

//---------------------------------------------------------------------
// Api Hook

DECLARE_HOOK_PROC(LRESULT, CDECL, ExeditWindowProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp));
DECLARE_HOOK_PROC(LRESULT, WINAPI, SettingDialogProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
DECLARE_HOOK_PROC(BOOL, CDECL, AddAlias, (LPCSTR fileName, BOOL flag1, BOOL flag2, int objectIndex));

//---------------------------------------------------------------------

void initHook(FILTER* fp);
void termHook(FILTER* fp);
void getPath(LPTSTR buffer);
void clearAliasFile();
BOOL addAliasFile(LPCSTR fileName);
void loadAlias();
BOOL saveAlias(HWND hwnd, int objectIndex, int filterIndex);

//---------------------------------------------------------------------
