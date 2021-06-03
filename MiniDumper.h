#ifndef MINIDUMPER_H
#define MINIDUMPER_H

#pragma warning( disable: 4091)
#pragma warning( disable: 4101)
#include <dbghelp.h>

#include <new.h>
#include <signal.h>
#include <exception>
#include <crtdbg.h>

#include "Uploader.h"

class CMiniDumper
{
public:
    CMiniDumper(bool bPromptUserForMiniDump, MINIDUMP_TYPE dumbType = MiniDumpNormal);
    ~CMiniDumper(void);

	bool GetIsDeleteDumpFile() const { return m_bIsDeleteDumpFile; }
	void SetIsDeleteDumpFile(bool val) { m_bIsDeleteDumpFile = val; }
	bool GetFTPUpload() const { return m_isFTPUpload; }
	void SetFTPUpload(bool val) { m_isFTPUpload = val; }
	TCHAR* GetFTPUploadFolderName() { return m_szFTPUploadFolderName; }
	void SetFTPUploadFolderName(TCHAR* val) { if(_tcslen(val) > 0) _tcscpy_s(m_szFTPUploadFolderName, val); }
	MINIDUMP_TYPE GetMiniDumpType() const { return m_eMiniDumpType; }
	void SetMiniDumpType(MINIDUMP_TYPE value) { m_eMiniDumpType = value; }

	static LONG WINAPI unhandledExceptionHandler(struct _EXCEPTION_POINTERS *pExceptionInfo);

private:
	void SetCRTExceptionHandlerPerThread();
	void SetCRTExceptionHandlerPerProcess();
    
    void setMiniDumpFilePath(void);
    bool getImpersonationToken(HANDLE* phToken);
    BOOL enablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);
    BOOL restorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);
    LONG writeMiniDump(_EXCEPTION_POINTERS *pExceptionInfo );
	void LockUnhandledExceptionFilter();

	CUploader *m_pUploader;

	bool m_bPromptUserForMiniDump;

    _EXCEPTION_POINTERS *m_pExceptionInfo;
	TCHAR m_szDay[MAX_PATH];
	TCHAR m_szFileName[MAX_PATH];
    TCHAR m_szMiniDumpPath[MAX_PATH];
    TCHAR m_szAppPath[MAX_PATH];
    TCHAR m_szAppBaseName[MAX_PATH];

	bool m_bIsDeleteDumpFile = false;
	bool m_isFTPUpload = false;
	TCHAR m_szFTPUploadFolderName[MAX_PATH];
	MINIDUMP_TYPE m_eMiniDumpType;

	CRITICAL_SECTION m_cs;
};

#endif // MINIDUMPER_H
