#pragma once

//---------------------------------------------------------------------
// Define and Const

#define DECLARE_INTERNAL_PROC(resultType, callType, procName, args) \
	typedef resultType (callType *Type_##procName) args; \
	extern Type_##procName procName

#define IMPLEMENT_INTERNAL_PROC(resultType, callType, procName, args) \
	Type_##procName procName = 0

#define DECLARE_HOOK_PROC(resultType, callType, procName, args) \
	typedef resultType (callType *Type_##procName) args; \
	extern Type_##procName true_##procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC(resultType, callType, procName, args) \
	Type_##procName true_##procName = procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC_NULL(resultType, callType, procName, args) \
	Type_##procName true_##procName = NULL; \
	resultType callType hook_##procName args

#define GET_INTERNAL_PROC(module, procName) \
	procName = (Type_##procName)::GetProcAddress(module, #procName)

#define GET_HOOK_PROC(module, procName) \
	true_##procName = (Type_##procName)::GetProcAddress(module, #procName)

#define ATTACH_HOOK_PROC(name) DetourAttach((PVOID*)&true_##name, hook_##name)

//---------------------------------------------------------------------
// Api Hook

DECLARE_HOOK_PROC(LRESULT, CDECL, Exedit_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, void *editp, FILTER *fp));
DECLARE_HOOK_PROC(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));
DECLARE_HOOK_PROC(LPCSTR, CDECL, Exedit_GetAliasFileName, (int aliasId));
DECLARE_HOOK_PROC(BOOL, CDECL, Exedit_AddAlias, (LPCSTR fileName, BOOL flag1, BOOL flag2, int objectIndex));

//---------------------------------------------------------------------
// Internal Function and Variable

DECLARE_INTERNAL_PROC(BOOL,  CDECL, Exedit_SaveFilterAlias, (int objectIndex, int filterIndex, LPCSTR fileName));

int Exedit_GetCurrentObjectIndex();
int Exedit_GetCurrentFilterIndex();
auls::EXEDIT_OBJECT* Exedit_GetObject(int objectIndex);
auls::EXEDIT_FILTER* Exedit_GetFilter(int filterId);
auls::EXEDIT_FILTER* Exedit_GetFilter(auls::EXEDIT_OBJECT* object, int filterIndex);
int Exedit_GetNextObjectIndex(int objectIndex);
int Exedit_GetSelectedObjects(int i);
int Exedit_GetSelectedObjectsCount();

//---------------------------------------------------------------------

void initExeditHook();
void getPath(LPTSTR buffer);
void clearAliasFile();
BOOL addAliasFile(LPCSTR fileName);
void loadAlias();
BOOL saveAlias(HWND hwnd, int objectIndex, int filterIndex);

//---------------------------------------------------------------------
