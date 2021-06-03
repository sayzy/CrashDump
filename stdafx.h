#pragma once

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
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