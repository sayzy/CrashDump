#pragma once

#ifdef _CRASH_DUMP_DLL
#define _CRASH_DUMP_API __declspec(dllexport)
#else 
#define _CRASH_DUMP_API __declspec(dllimport)
#endif

/**
* @param isDeleteDumpFile ���� ���� ���� ����
* @param isFTPUpload ���ε� ����
* @param UploaderFolderName ���ε� ���͸� �̸�
*/
EXTERN_C void DetectionCrash(bool isDeleteDumpFile = false, bool isFTPUpload = false, TCHAR* UploaderFolderName = "");