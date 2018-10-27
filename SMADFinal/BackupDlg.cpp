// BackupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "BackupDlg.h"
#include <shlobj.h>
#include <objidl.h>
#include "Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBackupDlg property page

IMPLEMENT_DYNCREATE(CBackupDlg, CPropertyPage)

CBackupDlg::CBackupDlg() : CPropertyPage(CBackupDlg::IDD)
{
	//{{AFX_DATA_INIT(CBackupDlg)
	m_sBackupPath = _T("");
	//}}AFX_DATA_INIT
}

CBackupDlg::~CBackupDlg()
{
}

void CBackupDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackupDlg)
	DDX_Control(pDX, IDC_MOVELIST, m_MoveList);
	DDX_Control(pDX, IDC_COPYLIST, m_CopyList);
	DDX_Text(pDX, IDC_BACKUPLOC, m_sBackupPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBackupDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CBackupDlg)
	ON_BN_CLICKED(IDC_ADDMOVEBUTTON, OnAddmovebutton)
	ON_BN_CLICKED(IDC_ADDCOPYBUTTON, OnAddcopybutton)
	ON_LBN_DBLCLK(IDC_COPYLIST, OnDblclkCopylist)
	ON_LBN_DBLCLK(IDC_MOVELIST, OnDblclkMovelist)
	ON_BN_CLICKED(IDC_CHANGEBUTTON, OnChangebutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBackupDlg message handlers

BOOL CBackupDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetRegistrySettings();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CBackupDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateRegistrySettings();
	return CPropertyPage::OnApply();
}



void CBackupDlg::OnAddmovebutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_ALLOWMULTISELECT | OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);

	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		POSITION POS;
		POS = pcBrowse->GetStartPosition();
		while(POS)
			m_MoveList.AddString(pcBrowse->GetNextPathName(POS));
	}

	delete pcBrowse;
}

void CBackupDlg::OnAddcopybutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	CString sTemp;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_ALLOWMULTISELECT | OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);

	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		POSITION POS;
		POS = pcBrowse->GetStartPosition();
		while(POS)
			m_CopyList.AddString(pcBrowse->GetNextPathName(POS));
	}

	delete pcBrowse;
}

void CBackupDlg::OnDblclkCopylist() 
{
	// TODO: Add your control notification handler code here
	int iListItem = m_CopyList.GetCurSel();
	m_CopyList.DeleteString(iListItem);
}

void CBackupDlg::OnDblclkMovelist() 
{
	// TODO: Add your control notification handler code here
	int iListItem = m_MoveList.GetCurSel();
	m_MoveList.DeleteString(iListItem);
}

void CBackupDlg::OnChangebutton() 
{
	// TODO: Add your control notification handler code here
	BROWSEINFO pB;
	ITEMIDLIST* ID;
	ITEMIDLIST* pidlLocalMachine;

	char ach[MAX_PATH];

	SHGetSpecialFolderLocation(NULL,CSIDL_DRIVES,&pidlLocalMachine);
	pB.hwndOwner = this->m_hWnd;
	pB.pidlRoot = pidlLocalMachine;
	pB.pszDisplayName = ach;
	pB.lpszTitle = NULL;
	pB.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN;
	pB.lpfn = NULL;
	ID = SHBrowseForFolder(&pB);
    if (ID)
	{
	    SHGetPathFromIDList(ID, ach);
		m_sBackupPath = ach;
	}
	UpdateData(false);
}

void CBackupDlg::GetRegistrySettings()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp;
	int iPos = -1;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sBackupPath = Reg.FetchStrValue(&lRes,hKey,"BackupPath");
		if(lRes == ERROR_SUCCESS)
		{
			if(m_sBackupPath.GetAt(m_sBackupPath.GetLength() - 2) != ':')
				m_sBackupPath = m_sBackupPath.Left(m_sBackupPath.GetLength() - 1);
		}
		else
			m_sBackupPath = "C:\\";

		sTemp = Reg.FetchStrValue(&lRes,hKey,"CopyFileList");
		for(iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_CopyList.AddString(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_CopyList.AddString(sTemp);

		sTemp = Reg.FetchStrValue(&lRes,hKey,"MoveFileList");
		for(iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_MoveList.AddString(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_MoveList.AddString(sTemp);

		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);
}

void CBackupDlg::UpdateRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		if(m_sBackupPath.ReverseFind('\\') < m_sBackupPath.GetLength() - 1)
			m_sBackupPath += "\\";

		Reg.SetRegValue(hKey,"BackupPath",REG_EXPAND_SZ,
			(LPBYTE)m_sBackupPath.GetBuffer(m_sBackupPath.GetLength()),
			m_sBackupPath.GetLength());
		Reg.SetRegValue(hKey,"BackupEvtPath",REG_EXPAND_SZ,
			(LPBYTE)m_sBackupPath.GetBuffer(m_sBackupPath.GetLength()),
			m_sBackupPath.GetLength());


		HKEY hDbKey;

		hDbKey = Reg.OpenRegKey(&lRes,"Event",hKey);
		if(lRes == ERROR_SUCCESS)
		{
			Reg.SetRegValue(hDbKey,"TempDbPath",REG_SZ,
				(LPBYTE)m_sBackupPath.GetBuffer(m_sBackupPath.GetLength()),
				m_sBackupPath.GetLength());

			Reg.CloseRegKey(hDbKey);
		}

		hDbKey = Reg.OpenRegKey(&lRes,"Version",hKey);
		if(lRes == ERROR_SUCCESS)
		{
			Reg.SetRegValue(hDbKey,"TempDbPath",REG_SZ,
				(LPBYTE)m_sBackupPath.GetBuffer(m_sBackupPath.GetLength()),
				m_sBackupPath.GetLength());

			Reg.CloseRegKey(hDbKey);
		}


		for(int i = 0; i < m_CopyList.GetCount(); i++)
		{
			m_CopyList.GetText(i,sItem);
			sTemp += sItem;
			if(i < m_CopyList.GetCount() - 1)
				sTemp += ";";
		}
		Reg.SetRegValue(hKey,"CopyFileList",REG_EXPAND_SZ,
			(LPBYTE)sTemp.GetBuffer(sTemp.GetLength()),
			sTemp.GetLength());
		sTemp.Empty();

		for(i = 0; i < m_MoveList.GetCount(); i++)
		{
			m_MoveList.GetText(i,sItem);
			sTemp += sItem;
			if(i < m_MoveList.GetCount() - 1)
				sTemp += ";";
		}
		Reg.SetRegValue(hKey,"MoveFileList",REG_EXPAND_SZ,
			(LPBYTE)sTemp.GetBuffer(sTemp.GetLength()),
			sTemp.GetLength());
		sTemp.Empty();

		Reg.CloseRegKey(hKey);
	}
}