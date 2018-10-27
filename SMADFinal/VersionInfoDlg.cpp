// VersionInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "VersionInfoDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVersionInfoDlg property page

IMPLEMENT_DYNCREATE(CVersionInfoDlg, CPropertyPage)

CVersionInfoDlg::CVersionInfoDlg() : CPropertyPage(CVersionInfoDlg::IDD)
{
	//{{AFX_DATA_INIT(CVersionInfoDlg)
	m_sVersionMdbFile = _T("");
	//}}AFX_DATA_INIT
}

CVersionInfoDlg::~CVersionInfoDlg()
{
}

void CVersionInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVersionInfoDlg)
	DDX_Control(pDX, IDC_VERSIONLIST, m_VersionList);
	DDX_Text(pDX, IDC_VERSIONMDB, m_sVersionMdbFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVersionInfoDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CVersionInfoDlg)
	ON_LBN_DBLCLK(IDC_VERSIONLIST, OnDblclkVersionlist)
	ON_BN_CLICKED(IDC_VSLISTBUTTON, OnVslistbutton)
	ON_BN_CLICKED(IDC_VERSIONBUTTON, OnVersionbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVersionInfoDlg message handlers

BOOL CVersionInfoDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetRegistryInfo();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CVersionInfoDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();
	return CPropertyPage::OnApply();
}

void CVersionInfoDlg::OnDblclkVersionlist() 
{
	// TODO: Add your control notification handler code here
	int iListItem = m_VersionList.GetCurSel();
	m_VersionList.DeleteString(iListItem);
}

void CVersionInfoDlg::OnVslistbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;

	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_ALLOWMULTISELECT | OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);

	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		POSITION POS;
		POS = pcBrowse->GetStartPosition();
		while(POS)
			m_VersionList.AddString(pcBrowse->GetNextPathName(POS));
	}
	delete pcBrowse;
	
}

void CVersionInfoDlg::OnVersionbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);

	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		m_sVersionMdbFile = pcBrowse->GetPathName();
		UpdateData(false);
	}
	delete pcBrowse;
}

void CVersionInfoDlg::GetRegistryInfo()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp;
	int iPos = -1;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{

		sTemp = Reg.FetchStrValue(&lRes,hKey,"FileVersion");
		for(iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_VersionList.AddString(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_VersionList.AddString(sTemp);

		m_sVersionMdbFile = Reg.FetchStrValue(&lRes,hKey,"VersionMdb");
		if(lRes != ERROR_SUCCESS)
			m_sVersionMdbFile = "C:\\Version.Mdb";

		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);

}

void CVersionInfoDlg::SaveRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
	if(lRes == ERROR_SUCCESS)
	{
		for(int i = 0; i < m_VersionList.GetCount(); i++)
		{
			m_VersionList.GetText(i,sItem);
			sTemp += sItem;
			if(i < m_VersionList.GetCount() - 1)
				sTemp += ";";
		}
		Reg.SetRegValue(hKey,"FileVersion",REG_EXPAND_SZ,
			(LPBYTE)sTemp.GetBuffer(sTemp.GetLength()),
			sTemp.GetLength());
		sTemp.Empty();

		Reg.SetRegValue(hKey,"VersionMdb",REG_EXPAND_SZ,
			(LPBYTE)m_sVersionMdbFile.GetBuffer(m_sVersionMdbFile.GetLength()),
			m_sVersionMdbFile.GetLength());

		Reg.CloseRegKey(hKey);
	}

}
