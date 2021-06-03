#pragma once

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define STRSAFE_NO_DEPRECATE

#include <Windows.h>

#include <stdio.h>
#include <assert.h>
#include <time.h>

#define NOMINMAX
#include <windows.h>

#include <tchar.h>

#include <strsafe.h>

#ifdef UNICODE
#define _tcssprintf wsprintf
#define tcsplitpath _wsplitpath
#else
#define _tcssprintf sprintf
#define tcsplitpath _splitpath
#endif