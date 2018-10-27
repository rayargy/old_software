// CabFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "CabFileDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCabFileDlg property page

IMPLEMENT_DYNCREATE(CCabFileDlg, CPropertyPage)

CCabFileDlg::CCabFileDlg() : CPropertyPage(CCabFileDlg::IDD)
{
	//{{AFX_DATA_INIT(CCabFileDlg)
	m_sCabFileName = _T("");
	//}}AFX_DATA_INIT
}

CCabFileDlg::~CCabFileDlg()
{
}

void CCabFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCabFileDlg)
	DDX_Text(pDX, IDC_CABEDIT, m_sCabFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCabFileDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CCabFileDlg)
	ON_BN_CLICKED(IDC_CABBUTTON, OnCabbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCabFileDlg message handlers

BOOL CCabFileDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetRegistryInfo();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CCabFileDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();
	return CPropertyPage::OnApply();
}

void CCabFileDlg::OnCabbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);
	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		m_sCabFileName = pcBrowse->GetPathName();
		UpdateData(false);
	}
	delete pcBrowse;

}

void CCabFileDlg::GetRegistryInfo()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp;
	int iPos = -1;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sCabFileName = Reg.FetchStrValue(&lRes,hKey,"CtrCabFile");
		if(lRes != ERROR_SUCCESS)
			m_sCabFileName = "C:\\CTRFiles.CAB";
		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);
}

void CCabFileDlg::SaveRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
	if(lRes == ERROR_SUCCESS)
	{
		Reg.SetRegValue(hKey,"CtrCabFile",REG_EXPAND_SZ,
			(LPBYTE)m_sCabFileName.GetBuffer(m_sCabFileName.GetLength()),
			m_sCabFileName.GetLength());

		Reg.CloseRegKey(hKey);
	}
}
