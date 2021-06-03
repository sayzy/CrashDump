#include "stdafx.h"
#include "MiniDumper.h"
#include "CrashDump.h"

CMiniDumper* g_miniDumperObject = NULL;

void TryExcept()
{
	CHAR* p = NULL;
	__try
	{
		*p = 'A';
	}
	__except (g_miniDumperObject->unhandledExceptionHandler(GetExceptionInformation()))
	{
		exit(1);
	}
}

void DetectionCrash(bool isDeleteDumpFile, bool isFTPUpload, TCHAR* UploaderFolderName)
{
	if (g_miniDumperObject == NULL)
	{
		g_miniDumperObject = new CMiniDumper(true);
		g_miniDumperObject->SetIsDeleteDumpFile(isDeleteDumpFile);
		g_miniDumperObject->SetFTPUpload(isFTPUpload);
		g_miniDumperObject->SetFTPUploadFolderName(UploaderFolderName);
	}

#ifdef _DEBUG
	// 디버그 모드에서 테스트를 하기 위한 코드
	//TryExcept();
#endif
}
