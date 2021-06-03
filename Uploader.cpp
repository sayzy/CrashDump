#include "stdafx.h"
#include "Uploader.h"
#include "CombatClientVersion.h"

CUploader::CUploader()
{
	InitializeCriticalSection(&m_cs);
}

CUploader::~CUploader()
{
	DeleteCriticalSection(&m_cs);
}

bool CUploader::UploadFTP(TCHAR* szDay, TCHAR* szUploadRootName, TCHAR* szFileName, TCHAR* szSendFilePath)
{
	//long long fileSize = 0;
	HINTERNET hInternet;
	HINTERNET hFtp;

	setUpladePath(szDay, szUploadRootName, szFileName);

	hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	hFtp = InternetConnect(hInternet, FTPURL, DEFAULT_FTP_PORT, FTPID, FTPPASSWARD, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);

 	FtpCreateDirectory(hFtp, m_szMiniDumpPath);
 	FtpCreateDirectory(hFtp, m_szMiniDumpCountPath);
 
	//fileSize = GetFilesize(_szDumpFileToSend);

	EnterCriticalSection(&m_cs);
	if (FtpPutFile(hFtp, szSendFilePath, m_szMiniDumpUploadFilePath, FTP_TRANSFER_TYPE_BINARY, 0))
	{
#ifdef _DEBUG
		MessageBox(NULL, "Upload Successful.", "Title", NULL);
#endif
	}
	else
	{
#ifdef _DEBUG
		//This is the message I get...
		MessageBox(NULL, "Upload Failed.", "Title", NULL);
#endif
		LeaveCriticalSection(&m_cs);

		InternetCloseHandle(hFtp);
		InternetCloseHandle(hInternet);
		return false;
	}
	LeaveCriticalSection(&m_cs);

	InternetCloseHandle(hFtp);
	InternetCloseHandle(hInternet);

	return true;
}

void CUploader::setUpladePath(TCHAR* szDay, TCHAR* szUploadRootName, TCHAR* szFileName)
{
	StringCbPrintf(m_szMiniDumpPath, sizeof(m_szMiniDumpPath), ".\\%s\\%s\\%s", szUploadRootName, COMBAT_CLIENT_VERSION_NARROW, szDay);
	StringCbPrintf(m_szMiniDumpUploadFilePath, sizeof(m_szMiniDumpUploadFilePath), "%s\\%s.dmp", m_szMiniDumpPath, szFileName);
	StringCbPrintf(m_szMiniDumpCountPath, sizeof(m_szMiniDumpCountPath), "%s\\count\\%s", m_szMiniDumpPath, szFileName);
}

