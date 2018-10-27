// BackupFiles.h: interface for the CBackupFiles class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BACKUPFILES_H__B7572170_48DD_11D3_8080_0090271354E9__INCLUDED_)
#define AFX_BACKUPFILES_H__B7572170_48DD_11D3_8080_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CBackupFiles  
{
public:
	void MoveFiles();
	void CopyFiles();
	CStringList m_sCopyList;
	CStringList m_sMoveList;
	CString m_strBackupPath;
	CString m_strFile;
	CBackupFiles();
	virtual ~CBackupFiles();
	void FileMove();
	void FileCopy();
	CSMADErrorLog* m_log;

private:
};

#endif // !defined(AFX_BACKUPFILES_H__B7572170_48DD_11D3_8080_0090271354E9__INCLUDED_)
