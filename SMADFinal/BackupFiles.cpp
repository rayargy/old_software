// BackupFiles.cpp: implementation of the CBackupFiles class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BackupFiles.h"
#include "registry.h"
#include "SMADErrorLog.h"

#define FILELISTLOC "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBackupFiles::CBackupFiles()
{
	CRegistry Reg;
	HKEY hKey;
	DWORD dwValType = REG_MULTI_SZ,dwValSize = 0;
	LPBYTE lpBuf;
	CString sTemp;
	long lRes = 0;

	m_log->GetInstance();
	m_log->WriteLogEntry("CBackupFiles::CBackupFiles(): Constructor for CBackupFiles",0);

	hKey = Reg.OpenRegKey(&lRes,FILELISTLOC);
	if(lRes == ERROR_SUCCESS)
	{

		lRes = Reg.QueryRegValue(hKey,"MoveFileList",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"MoveFileList",&dwValType,lpBuf,&dwValSize);
		sTemp = lpBuf;

		for(int iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_sMoveList.AddHead(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_sMoveList.AddHead(sTemp);

		LocalFree(lpBuf);

		dwValSize = 0;
		lRes = Reg.QueryRegValue(hKey,"CopyFileList",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"CopyFileList",&dwValType,lpBuf,&dwValSize);
		sTemp = lpBuf;

		for(iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_sCopyList.AddHead(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_sCopyList.AddHead(sTemp);

		LocalFree(lpBuf);

		m_strBackupPath = Reg.FetchStrValue(&lRes,hKey,"BackupPath");
		if(lRes != ERROR_SUCCESS)
			m_strBackupPath = "C:\\";
		Reg.CloseRegKey(hKey);
	}
}

CBackupFiles::~CBackupFiles()
{

}

void CBackupFiles::FileCopy()
{
	HANDLE hFile;
	DWORD dwDbFilSiz = 0;
	ULARGE_INTEGER iUserSpace,iTotalSpace,iDiskFree;
	LPVOID lpMsgBuf;
	CString strMsg;
	BOOL bFileCopied = FALSE;

	hFile = CreateFile(
		m_strFile,							// File to open
		GENERIC_READ,						// We are reading
		FILE_SHARE_READ,					// Others can read too
		NULL,
		OPEN_EXISTING,						// Open only if exists
		FILE_FLAG_SEQUENTIAL_SCAN,			// We are reading sequentially
		0);									// No template needed

	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwDbFilSiz = GetFileSize(hFile,NULL); // This file should never be larger than 4 gig!
		CloseHandle(hFile);
		if(dwDbFilSiz > 0)
		{
			if(GetDiskFreeSpaceEx(m_strBackupPath,&iUserSpace,&iTotalSpace,&iDiskFree))
			{
				if(iUserSpace.QuadPart/dwDbFilSiz >= 4)
				{
					// Backup the corrupt database if it exists!
					// if it doesn't exist it doesn't matter because it will be recreated!
					int iCharPos = m_strFile.ReverseFind('\\');
					bFileCopied = CopyFile(m_strFile,
						m_strBackupPath + m_strFile.Mid(iCharPos),true);
					if(!bFileCopied)
					{
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,0,NULL);
						
						strMsg.Format("\tCBackupFiles::FileCopy() %s",(LPTSTR)lpMsgBuf);
						strMsg.TrimRight();
						m_log->WriteLogEntry(strMsg,0);
						LocalFree(lpMsgBuf);
					}
				}
				else // not enough free space
				{
					strMsg.Format("\tCBackupFiles::FileCopy() Disk free space to low: %ul to backup %s",
							iUserSpace.QuadPart,m_strFile);
					strMsg.TrimRight();
					m_log->WriteLogEntry(strMsg,0);
				}
			}
			else
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);
				
				strMsg.Format("\tCBackupFiles::FileCopy() %s",(LPTSTR)lpMsgBuf);
				strMsg.TrimRight();
				m_log->WriteLogEntry(strMsg,0);
				LocalFree(lpMsgBuf);
			}
		}
		else
		{
			strMsg.Format("\tCBackupFiles::FileCopy() failed %s is zero bytes",m_strFile);
			strMsg.TrimRight();
			m_log->WriteLogEntry(strMsg,0);
		}
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);
		
		strMsg.Format("\tCBackupFiles::FileCopy() %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}
}

void CBackupFiles::FileMove()
{
	LPVOID lpMsgBuf;
	CString strMsg;
	BOOL bFileMoved = FALSE;

	int iCharPos = m_strFile.ReverseFind('\\');

	bFileMoved = MoveFileEx(m_strFile,
		m_strBackupPath + m_strFile.Mid(iCharPos),MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	if(!bFileMoved)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCBackupFiles::FileMove(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}
}

void CBackupFiles::CopyFiles()
{
	POSITION iPos;
	CString strMsg;

	iPos = m_sCopyList.GetHeadPosition();
	while(iPos != NULL)
	{
		m_strFile = m_sCopyList.GetNext(iPos);
		strMsg.Format("\tCBackupFiles::CopyFiles(): Processing file %s",m_strFile);
		m_log->WriteLogEntry(strMsg,0);
		FileCopy();
	}
}

void CBackupFiles::MoveFiles()
{
	POSITION iPos;
	CString strMsg;

	iPos = m_sMoveList.GetHeadPosition();
	while(iPos != NULL)
	{
		m_strFile = m_sMoveList.GetNext(iPos);
		strMsg.Format("\tCBackupFiles::MoveFiles(): Processing file %s",m_strFile);
		m_log->WriteLogEntry(strMsg,0);
		FileMove();
	}
}
