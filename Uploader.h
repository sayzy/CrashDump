#ifndef UPLOADER_H
#define UPLOADER_H

#include <fstream>

#include <wininet.h>
#pragma comment(lib, "wininet")

#define FTPURL _T("0.0.0.0")
#define FTPID _T("id")
#define FTPPASSWARD _T("passward")
#define DEFAULT_FTP_PORT 2122
//#define CLIENT_VERSION _T("VER_EU_1902.01")

class CUploader
{
public:
	CUploader();
	~CUploader();

	bool UploadFTP(TCHAR* szDay, TCHAR* szProjectName, TCHAR* szFileName, TCHAR* szSendFilePath);

private:
	void setUpladePath(TCHAR* szDay, TCHAR* szProjectName, TCHAR* szFileName);

	TCHAR m_szMiniDumpPath[MAX_PATH];
	TCHAR m_szMiniDumpUploadFilePath[MAX_PATH];
	TCHAR m_szMiniDumpCountPath[MAX_PATH];

	CRITICAL_SECTION m_cs;
};

inline int GetFilesize(const TCHAR* path)
{
	std::ifstream ifs(path, std::ios::in | std::ios::binary);
	int size = 0;

	if (ifs.is_open() == 0)
		printf("\n\n파일이 존재하지 않습니다.n\n");

	ifs.seekg(0, std::ios::end);
	size = (int)ifs.tellg();
	ifs.close();
	return size;
}

#endif // UPLOADER_H