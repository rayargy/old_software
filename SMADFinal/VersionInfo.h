// VersionInfo.h: interface for the CVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONINFO_H__876601C5_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
#define AFX_VERSIONINFO_H__876601C5_5EB6_11D3_80D5_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define NUM_VERSION_INFO_KEYS    12     // See FilerGetVersion()
#define VERSION_DLG_LB_HEIGHT    10     // height in text lines of version dlg LBs

#define VERSION_INFO_KEY_ROOT    TEXT("\\StringFileInfo\\")
typedef struct _VersionKeyInfo {
    TCHAR const *szKey;
    TCHAR       *szValue;
} VKINFO, *LPVKINFO;


class CVersionInfo  
{
typedef struct _VersionKeyInfo {
    TCHAR const *szKey;
    TCHAR       *szValue;
} VKINFO, *LPVKINFO;

public:
	bool GetFileInfo();
	CString m_sFileSize;
	COleDateTime m_FileDateTime;
	CString m_sMdbPath;
	void GetFileVersions();
	CStringList m_sVersionList;
	CString m_sFileToCheck;
	void GetFileVersion();
	CVersionInfo();
	virtual ~CVersionInfo();
	BOOL FilerGetVersion(LPTSTR, DWORD, LPTSTR);
	VKINFO  gVKArray[NUM_VERSION_INFO_KEYS];     // .EXE version info array.
	CSMADErrorLog* m_Log;
};

#endif // !defined(AFX_VERSIONINFO_H__876601C5_5EB6_11D3_80D5_0090271354E9__INCLUDED_)
