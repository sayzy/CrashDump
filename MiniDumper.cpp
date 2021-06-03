#include "stdafx.h"
#include "miniDumper.h"

const int USER_DATA_BUFFER_SIZE = 4096;

CMiniDumper *g_miniDumper = NULL;

typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess,
	DWORD dwPid,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

#pragma region Handler
static void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	UNREFERENCED_PARAMETER(expression);
	UNREFERENCED_PARAMETER(function);
	UNREFERENCED_PARAMETER(file);
	UNREFERENCED_PARAMETER(line);
	UNREFERENCED_PARAMETER(pReserved);

	RaiseException(STATUS_INVALID_PARAMETER, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static void PurecallHandler(void)
{
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static int NewHandler(size_t size)
{
	UNREFERENCED_PARAMETER(size);
	RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
	return 0;
};

static void SigabrtHandler(int code)
{
	UNREFERENCED_PARAMETER(code);
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static void SigintHandler(int code)
{
	UNREFERENCED_PARAMETER(code);
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static void SigtermHandler(int code)
{
	UNREFERENCED_PARAMETER(code);
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static void TerminateHandler()
{
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};

static void UnexpectedHandler()
{
	RaiseException(STATUS_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
};
#pragma endregion Handler

CMiniDumper::CMiniDumper(bool bPromptUserForMiniDump, MINIDUMP_TYPE dumbType)
	: m_szFTPUploadFolderName("")
{
	LPTOP_LEVEL_EXCEPTION_FILTER pRet = ::SetUnhandledExceptionFilter(unhandledExceptionHandler);
	if (pRet == NULL)
	{
		printf("SetUnhandledExceptionFilter failed... This program can't get dmp file at crash time\n");
	}
	LockUnhandledExceptionFilter();
	SetCRTExceptionHandlerPerThread();
	SetCRTExceptionHandlerPerProcess();
	InitializeCriticalSection(&m_cs);
	g_miniDumper = this;
	m_bPromptUserForMiniDump = bPromptUserForMiniDump;
	m_eMiniDumpType = dumbType;
	m_pUploader = new CUploader();
}

CMiniDumper::~CMiniDumper(void)
{
	delete m_pUploader;
	DeleteCriticalSection(&m_cs);
}

void CMiniDumper::SetCRTExceptionHandlerPerThread()
{
	set_terminate(TerminateHandler);
	set_unexpected(UnexpectedHandler);
}

void CMiniDumper::SetCRTExceptionHandlerPerProcess()
{
	_set_invalid_parameter_handler(InvalidParameterHandler);
	_set_purecall_handler(PurecallHandler);
	_set_new_mode(1);
	_set_new_handler(NewHandler);
	_CrtSetReportMode(_CRT_ASSERT, 0);

	_set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
	signal(SIGABRT, SigabrtHandler);
	signal(SIGINT, SigintHandler);
	signal(SIGTERM, SigtermHandler);
}

LONG CMiniDumper::unhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo)
{
	//int nRet = MessageBox(NULL, "Sorry！软件崩溃了，为了提高软件质量，请点击\"确定\"按钮，生成转储文件，并把转储文件发送给开发人员。感谢您的配合！", "软件一不小心崩溃了...", MB_OKCANCEL);
	//if (nRet == IDCANCEL)
	//{
	//	TerminateProcess(GetCurrentProcess(), -1);
	//}

	if (!g_miniDumper)
		return EXCEPTION_CONTINUE_SEARCH;

	g_miniDumper->writeMiniDump(pExceptionInfo);
	TerminateProcess(GetCurrentProcess(), -1);
	return 0;
}

void CMiniDumper::setMiniDumpFilePath(void)
{
	time_t now = time(0);
	struct tm  tstruct;
	localtime_s(&tstruct, &now);
	strftime(m_szDay, sizeof(m_szDay), "%Y%m%d", &tstruct);

	sprintf_s(m_szFileName,
		_T("%s-%u-%u-%u"), m_szDay,
		tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec);

	sprintf_s(m_szMiniDumpPath,
		_T(".\\%s_%s.dmp"),
		m_szAppBaseName,
		m_szFileName);
}

bool CMiniDumper::getImpersonationToken(HANDLE* phToken)
{
	*phToken = NULL;

	if (!OpenThreadToken(GetCurrentThread(),
		TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
		TRUE,
		phToken))
	{
		if (GetLastError() == ERROR_NO_TOKEN)
		{
			if (!OpenProcessToken(GetCurrentProcess(),
				TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
				phToken))
				return false;
		}
		else
			return false;
	}

	return true;
}

BOOL CMiniDumper::enablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
{
	BOOL bOk = FALSE;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	bOk = LookupPrivilegeValue(0, pszPriv, &tp.Privileges[0].Luid);

	if (bOk)
	{
		DWORD cbOld = sizeof(*ptpOld);
		bOk = AdjustTokenPrivileges(hToken, FALSE, &tp, cbOld, ptpOld, &cbOld);
	}

	return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
}

BOOL CMiniDumper::restorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld)
{
	BOOL bOk = AdjustTokenPrivileges(hToken, FALSE, ptpOld, 0, NULL, NULL);
	return (bOk && (ERROR_NOT_ALL_ASSIGNED != GetLastError()));
}


LONG CMiniDumper::writeMiniDump(_EXCEPTION_POINTERS *pExceptionInfo)
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	m_pExceptionInfo = pExceptionInfo;

	HANDLE hImpersonationToken = NULL;
	if (!getImpersonationToken(&hImpersonationToken))
		return FALSE;

	HMODULE hDll = NULL;
	TCHAR szDbgHelpPath[MAX_PATH];

	if (GetModuleFileName(NULL, m_szAppPath, MAX_PATH))
	{
		TCHAR *pSlashFirst = _tcsrchr(m_szAppPath, '\\');
		TCHAR *pSlashSecond = _tcstok_s(pSlashFirst, ".", &pSlashFirst);

		if (pSlashSecond)
		{
			_tcscpy_s(m_szAppBaseName, pSlashSecond + 1);
			*(pSlashSecond + 1) = 0;
		}

		_tcscpy_s(szDbgHelpPath, m_szAppPath);
		_tcscat_s(szDbgHelpPath, _T("DBGHELP.DLL"));
		hDll = ::LoadLibrary(szDbgHelpPath);
	}

	if (hDll == NULL)
	{
		// If we haven't found it yet - try one more time.
		hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP MiniDumpWriteDump =
			(MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");

		if (MiniDumpWriteDump != NULL)
		{
			TCHAR szScratch[USER_DATA_BUFFER_SIZE];

			setMiniDumpFilePath();

			// Create the mini-dump file...
			HANDLE hFile = ::CreateFile(m_szMiniDumpPath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = NULL;

				// We need the SeDebugPrivilege to be able to run MiniDumpWriteDump
				TOKEN_PRIVILEGES tp;
				BOOL bPrivilegeEnabled = enablePrivilege(SE_DEBUG_NAME, hImpersonationToken, &tp);

				BOOL bOk;

				// DBGHELP.dll is not thread-safe, so we need to restrict access...
				EnterCriticalSection(&m_cs);
				{
					try
					{
						// Write out the mini-dump data to the file...
						bOk = MiniDumpWriteDump(GetCurrentProcess(),
							GetCurrentProcessId(),
							hFile,
							m_eMiniDumpType,
							&ExInfo,
							NULL,
							NULL);
					}
					catch (...)
					{
						LeaveCriticalSection(&m_cs);
						throw;
					}

				}
				LeaveCriticalSection(&m_cs);

				// Restore the privileges when done
				if (bPrivilegeEnabled)
					restorePrivilege(hImpersonationToken, &tp);

				if (bOk)
				{
					szResult = NULL;
					retval = EXCEPTION_EXECUTE_HANDLER;
				}

				::CloseHandle(hFile);
			}
		}
		else
		{
			szResult = _T("Call to GetProcAddress failed to find MiniDumpWriteDump. ")
				_T("The DBGHELP.DLL is possibly outdated.");
		}
	}
	else
	{
		szResult = _T("Call to LoadLibrary failed to find DBGHELP.DLL.");
	}

	if (szResult && m_bPromptUserForMiniDump)
		::MessageBox(NULL, szResult, NULL, MB_OK);

	if (m_isFTPUpload)
	{
		if (_tcslen(m_szFTPUploadFolderName) == 0)
			m_pUploader->UploadFTP(m_szDay, m_szAppBaseName, m_szFileName, m_szMiniDumpPath);
		else
			m_pUploader->UploadFTP(m_szDay, m_szFTPUploadFolderName, m_szFileName, m_szMiniDumpPath);
	}

	if (m_bIsDeleteDumpFile)
		::DeleteFile(m_szMiniDumpPath);

	TerminateProcess(GetCurrentProcess(), 0);

	return retval;
}

void CMiniDumper::LockUnhandledExceptionFilter()
{
	HMODULE kernel32 = LoadLibraryA("kernel32.dll");

	if (FARPROC gpaSetUnhandledExceptionFilter = GetProcAddress(kernel32, "SetUnhandledExceptionFilter")) {
		unsigned char expected_code[] = {
			0x8B, 0xFF, // mov edi,edi
			0x55,       // push ebp
			0x8B, 0xEC, // mov ebp,esp
		};

		// only replace code we expect
		if (memcmp(expected_code, gpaSetUnhandledExceptionFilter, sizeof(expected_code)) == 0) {
			unsigned char new_code[] = {
				0x33, 0xC0,       // xor eax,eax
				0xC2, 0x04, 0x00, // ret 4
			};

			DWORD old_protect;
			if (VirtualProtect(gpaSetUnhandledExceptionFilter, sizeof(new_code), PAGE_EXECUTE_READWRITE, &old_protect)) {
				CopyMemory(gpaSetUnhandledExceptionFilter, new_code, sizeof(new_code));

				DWORD dummy;
				VirtualProtect(gpaSetUnhandledExceptionFilter, sizeof(new_code), old_protect, &dummy);

				FlushInstructionCache(GetCurrentProcess(), gpaSetUnhandledExceptionFilter, sizeof(new_code));
			}
		}
	}
	FreeLibrary(kernel32);
}