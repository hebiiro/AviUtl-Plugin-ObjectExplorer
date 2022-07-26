#pragma once

//---------------------------------------------------------------------
// Api Hook

DECLARE_HOOK_PROC(LRESULT, CDECL, ExEditWindowProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp));
DECLARE_HOOK_PROC(LRESULT, WINAPI, SettingDialogProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
DECLARE_HOOK_PROC(BOOL, CDECL, AddAlias, (LPCSTR fileName, BOOL flag1, BOOL flag2, int objectIndex));

//---------------------------------------------------------------------

void initHook(AviUtl::FilterPlugin* fp);
void termHook(AviUtl::FilterPlugin* fp);
void getPath(LPTSTR buffer);
void clearAliasFile();
BOOL addAliasFile(LPCSTR fileName);
void loadAlias();
BOOL saveAlias(HWND hwnd, int objectIndex, int filterIndex);

//---------------------------------------------------------------------
