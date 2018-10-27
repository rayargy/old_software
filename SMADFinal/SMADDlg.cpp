// SMADDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "SMADDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSMADDlg

IMPLEMENT_DYNAMIC(CSMADDlg, CPropertySheet)

CSMADDlg::CSMADDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
}

CSMADDlg::CSMADDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
}

CSMADDlg::~CSMADDlg()
{
	m_Log->DestroyInstance();
}


BEGIN_MESSAGE_MAP(CSMADDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CSMADDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSMADDlg message handlers

BOOL CSMADDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	// TODO: Add your specialized code here
	m_Log->GetInstance();
	LoadMdbConfig();

	for(int i = GetPageCount() - 1; i >= 0; i--)
		SetActivePage(i);

	return bResult;
}

void CSMADDlg::LoadMdbConfig()
{
	long lRes = 0;
	HKEY hKey;
	CRegistry Reg;
	CString strMsg;
	LPVOID lpMsgBuf;
	CString sTempKey;
	LPTSTR lpSysDir;
	UINT iSize = 0;

	// adjustment of privileges should probably be moved into a global area
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;
   
	if(!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES,&hToken ))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSMADDlg::LoadMdbConfig(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	if(!LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &luid))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSMADDlg::LoadMdbConfig(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tp,
		sizeof(TOKEN_PRIVILEGES),NULL, NULL );
	if (DWORD dw = GetLastError() != ERROR_SUCCESS)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL);

		strMsg.Format("\tCSMADDlg::LoadMdbConfig(): %s",(LPTSTR)lpMsgBuf);
		strMsg.TrimRight();
		m_Log->WriteLogEntry(strMsg,0);
		LocalFree(lpMsgBuf);
	}

	lpSysDir = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,MAX_PATH + 1);
	iSize = GetSystemDirectory(lpSysDir,MAX_PATH);

	sTempKey = SMADKEY;
	sTempKey += "\\Event";
	hKey = Reg.OpenRegKey(&lRes,sTempKey.GetBuffer(sTempKey.GetLength()));
	if(lRes == ERROR_SUCCESS)
	{
		CString sTemp,sRegFile;

		sRegFile = lpSysDir;
		sRegFile += "\\EventMdb";
		lRes = RegRestoreKey(hKey,sRegFile,0);
		if(lRes != ERROR_SUCCESS)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);

			strMsg.Format("\tCSMADDlg::LoadMdbConfig(): %s",(LPTSTR)lpMsgBuf);
			strMsg.TrimRight();
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
			AfxMessageBox("An error occurred loading registry information\nplease review the SMAD log for details",MB_OK);
		}
		Reg.CloseRegKey(hKey);
	}

	sTempKey = SMADKEY;
	sTempKey += "\\Version";
	hKey = Reg.OpenRegKey(&lRes,sTempKey.GetBuffer(sTempKey.GetLength()));
	if(lRes == ERROR_SUCCESS)
	{
		CString sTemp,sRegFile;

		sRegFile = lpSysDir;
		sRegFile += "\\VersionMdb";
		lRes = RegRestoreKey(hKey,sRegFile,0);
		if(lRes != ERROR_SUCCESS)
		{
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,0,NULL);

			strMsg.Format("\tCSMADDlg::LoadMdbConfig(): %s",(LPTSTR)lpMsgBuf);
			strMsg.TrimRight();
			m_Log->WriteLogEntry(strMsg,0);
			LocalFree(lpMsgBuf);
			AfxMessageBox("An error occurred loading registry information\nplease review the SMAD log for details",MB_OK);
		}
		Reg.CloseRegKey(hKey);
	}

	LocalFree(lpSysDir);

	// Revoke all privileges this process holds (including backup)
	AdjustTokenPrivileges( hToken, TRUE, NULL, 0, NULL, NULL);

	// close handle to process token
	CloseHandle(hToken);
}
