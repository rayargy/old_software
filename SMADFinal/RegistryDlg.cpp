// RegistryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "RegistryDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegistryDlg property page

IMPLEMENT_DYNCREATE(CRegistryDlg, CPropertyPage)

CRegistryDlg::CRegistryDlg() : CPropertyPage(CRegistryDlg::IDD)
{
	//{{AFX_DATA_INIT(CRegistryDlg)
	m_sRegFileName = _T("");
	//}}AFX_DATA_INIT
}

CRegistryDlg::~CRegistryDlg()
{
}

void CRegistryDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegistryDlg)
	DDX_Text(pDX, IDC_REGEDIT, m_sRegFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegistryDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CRegistryDlg)
	ON_BN_CLICKED(IDC_REGBUTTON, OnRegbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegistryDlg message handlers

BOOL CRegistryDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetRegistryInfo();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CRegistryDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();
	return CPropertyPage::OnApply();
}

void CRegistryDlg::OnRegbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);
	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		m_sRegFileName = pcBrowse->GetPathName();
		UpdateData(false);
	}
	delete pcBrowse;
}

void CRegistryDlg::GetRegistryInfo()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp;
	int iPos = -1;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sRegFileName = Reg.FetchStrValue(&lRes,hKey,"RegFile");
		if(lRes != ERROR_SUCCESS)
			m_sRegFileName = "C:\\CTRsys.reg";
		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);
}

void CRegistryDlg::SaveRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
	if(lRes == ERROR_SUCCESS)
	{
		Reg.SetRegValue(hKey,"RegFile",REG_EXPAND_SZ,
			(LPBYTE)m_sRegFileName.GetBuffer(m_sRegFileName.GetLength()),
			m_sRegFileName.GetLength());

		Reg.CloseRegKey(hKey);
	}
}
