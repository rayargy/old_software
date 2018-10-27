// SMADErrorLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "SMADErrorLogDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSMADErrorLogDlg property page

IMPLEMENT_DYNCREATE(CSMADErrorLogDlg, CPropertyPage)

CSMADErrorLogDlg::CSMADErrorLogDlg() : CPropertyPage(CSMADErrorLogDlg::IDD)
{
	//{{AFX_DATA_INIT(CSMADErrorLogDlg)
	m_sSMADLogName = _T("");
	m_dwDebug = 0;
	//}}AFX_DATA_INIT
}

CSMADErrorLogDlg::~CSMADErrorLogDlg()
{
}

void CSMADErrorLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSMADErrorLogDlg)
	DDX_Control(pDX, IDC_DEBUGSPIN, m_DebugS);
	DDX_Text(pDX, IDC_SMADLOGEDIT, m_sSMADLogName);
	DDX_Text(pDX, IDC_DEBUG, m_dwDebug);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSMADErrorLogDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CSMADErrorLogDlg)
	ON_BN_CLICKED(IDC_SMADLOGBUTTON, OnSmadlogbutton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DEBUGSPIN, OnDeltaposDebugspin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSMADErrorLogDlg message handlers

BOOL CSMADErrorLogDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_DebugS.SetRange(0,4);

	GetRegistryInfo();
	m_DebugS.SetPos(m_dwDebug);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSMADErrorLogDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();
	return CPropertyPage::OnApply();
}

void CSMADErrorLogDlg::OnSmadlogbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);
	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		m_sSMADLogName = pcBrowse->GetPathName();
		UpdateData(false);
	}
	delete pcBrowse;
}

void CSMADErrorLogDlg::GetRegistryInfo()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp;
	int iPos = -1;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		m_sSMADLogName = Reg.FetchStrValue(&lRes,hKey,"SMADLog");
		if(lRes != ERROR_SUCCESS)
			m_sSMADLogName = "C:\\SMADDbg.Log";
		m_dwDebug = Reg.FetchDWValue(&lRes,hKey,"DebugLevel");
		if(lRes != ERROR_SUCCESS)
			m_dwDebug = 4;
		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);

}

void CSMADErrorLogDlg::SaveRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
	if(lRes == ERROR_SUCCESS)
	{
		Reg.SetRegValue(hKey,"SMADLog",REG_EXPAND_SZ,
			(LPBYTE)m_sSMADLogName.GetBuffer(m_sSMADLogName.GetLength()),
			m_sSMADLogName.GetLength());

		Reg.SetRegValue(hKey,"DebugLevel",REG_DWORD,(LPBYTE)&m_dwDebug,sizeof(DWORD));

		Reg.CloseRegKey(hKey);
	}
}

void CSMADErrorLogDlg::OnDeltaposDebugspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_dwDebug = m_DebugS.GetPos() - 65536;
	UpdateData(false);

	*pResult = 0;
}
