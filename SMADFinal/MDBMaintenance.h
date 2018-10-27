// MDBMaintenance.h: interface for the CMDBMaintenance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDBMAINTENANCE_H__66F2913F_2A0F_11D3_804B_0090271354E9__INCLUDED_)
#define AFX_MDBMAINTENANCE_H__66F2913F_2A0F_11D3_804B_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMDBMaintenance  
{
protected:
	void vCloseDb();
	void vDbOpen();
	void ProcessTransDump();
	void BackupMDB();
	BOOL EnablePolling();
	BOOL DisablePolling();
	BOOL GetIndexInfo(HKEY,CDaoIndexInfo*,CDaoIndexFieldInfo*);
	void GetColumnInfo(HKEY,CDaoFieldInfo&);
	BOOL CreateQuery(HKEY,LPTSTR);
	BOOL CreateColumns(HKEY,LPTSTR);
	BOOL CreateQueryDef();
	BOOL CreateTableDef();
	CString m_strDbKey;
	CString m_strBackupPath;
	CString m_strTempDbPath;
	BOOL GetRegValue(HKEY,CString&,CString);
	BOOL RepairMdb();
	static void GetDaoPtr();
	static void DestroyDaoPtr();
	char m_szMessage[1024];
public:
	CMDBMaintenance(CString);
	virtual ~CMDBMaintenance();
	static CDaoDatabase* m_daodb;
	static CDaoTableDef* m_DaoTableDef;
	static CDaoQueryDef* m_DaoQueryDef;
	CString m_strDestinationDb;
	CString m_strTempDb;
	BOOL m_bRetVal;
	int InitMdb();
	BOOL CreateMdb();
	void MoveMDB();
	CSMADErrorLog* m_Log;
};

#endif // !defined(AFX_MDBMAINTENANCE_H__66F2913F_2A0F_11D3_804B_0090271354E9__INCLUDED_)
