#pragma once

#ifdef _CRASH_DUMP_DLL
#define _CRASH_DUMP_API __declspec(dllexport)
#else 
#define _CRASH_DUMP_API __declspec(dllimport)
#endif

/**
* @param isDeleteDumpFile 덤프 파일 삭제 여부
* @param isFTPUpload 업로드 여부
* @param UploaderFolderName 업로드 디렉터리 이름
*/
EXTERN_C void DetectionCrash(bool isDeleteDumpFile = false, bool isFTPUpload = false, TCHAR* UploaderFolderName = "");