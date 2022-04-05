#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")
#include <shellapi.h>

#include <tchar.h>
#include <strsafe.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
