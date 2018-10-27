// EventLogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "EventLogDlg.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventLogDlg property page

IMPLEMENT_DYNCREATE(CEventLogDlg, CPropertyPage)

CEventLogDlg::CEventLogDlg() : CPropertyPage(CEventLogDlg::IDD)
{
	//{{AFX_DATA_INIT(CEventLogDlg)
	m_sEventMdbName = _T("");
	//}}AFX_DATA_INIT
}

CEventLogDlg::~CEventLogDlg()
{
}

void CEventLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEventLogDlg)
	DDX_Control(pDX, IDC_EVENTLIST2, m_ELogsSelected);
	DDX_Control(pDX, IDC_EVENTLIST1, m_EventLogList);
	DDX_Text(pDX, IDC_EVENTLOGEDIT, m_sEventMdbName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEventLogDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CEventLogDlg)
	ON_LBN_DBLCLK(IDC_EVENTLIST1, OnDblclkEventlist1)
	ON_LBN_DBLCLK(IDC_EVENTLIST2, OnDblclkEventlist2)
	ON_BN_CLICKED(IDC_EVENTLOGBUTTON, OnEventlogbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventLogDlg message handlers

BOOL CEventLogDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetEventLogList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CEventLogDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();
	return CPropertyPage::OnApply();
}

void CEventLogDlg::OnDblclkEventlist1() 
{
	// TODO: Add your control notification handler code here
	CString sItem;
	int iListItem = m_EventLogList.GetCurSel();
	m_EventLogList.GetText(iListItem,sItem);
	m_ELogsSelected.AddString(sItem);
}

void CEventLogDlg::OnDblclkEventlist2() 
{
	// TODO: Add your control notification handler code here
	int iListItem = m_ELogsSelected.GetCurSel();
	m_ELogsSelected.DeleteString(iListItem);
}

void CEventLogDlg::OnEventlogbutton() 
{
	// TODO: Add your control notification handler code here
	CFileDialog* pcBrowse;
	
	//Need a file open dialog  
	pcBrowse = (CFileDialog*)new CFileDialog(TRUE,_T(""),_T("*.*"),
		OFN_SHAREAWARE,_T("All Files | *.* ||"), NULL);

	int nResponse = pcBrowse->DoModal();
	if(nResponse != IDCANCEL)
	{
		m_sEventMdbName = pcBrowse->GetPathName();
		UpdateData(false);
	}
	delete pcBrowse;
}

void CEventLogDlg::GetEventLogList()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	DWORD dwSubKeys = 0,dwMaxSubKeyLen = 0,dwValues = 0,
		dwMaxValueNameLen = 0,dwMaxValueLen = 0,
		dwName = 0,dwReserved = 0,dwClass = 128;
	FILETIME ftLastWriteTime;
	CString sTemp;


	hKey = Reg.OpenRegKey(&lRes,"SYSTEM\\CurrentControlSet\\Services\\EventLog");
	if(lRes == ERROR_SUCCESS)
	{
		LPTSTR sKeyName;

		lRes = Reg.QueryRegKey(hKey,
			&dwSubKeys,&dwMaxSubKeyLen,&dwValues,&dwMaxValueNameLen,&dwMaxValueLen);

		sKeyName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);

		for(int i = 0; (DWORD)i < dwSubKeys; i++)
		{
			dwName = dwMaxSubKeyLen + 1;
			lRes = RegEnumKeyEx(hKey,(DWORD)i,sKeyName,&dwName,NULL,NULL,NULL,&ftLastWriteTime );

			if(lRes == ERROR_SUCCESS)
				m_EventLogList.AddString(sKeyName);
		}
		LocalFree(sKeyName);
		Reg.CloseRegKey(hKey);
	}

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		sTemp = Reg.FetchStrValue(&lRes,hKey,"LogsToSave");
		for(int iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			m_ELogsSelected.AddString(sTemp.Left(iPos));
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
			m_ELogsSelected.AddString(sTemp);

		m_sEventMdbName = Reg.FetchStrValue(&lRes,hKey,"EventMdb");
		if(lRes != ERROR_SUCCESS)
			m_sEventMdbName = "C:\\Events.Mdb";
		Reg.CloseRegKey(hKey);
	}
	UpdateData(false);
}

void CEventLogDlg::SaveRegistrySettings()
{
	bool bRetVal = true;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CString sTemp,sItem;

	hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
	if(lRes == ERROR_SUCCESS)
	{

		for(int i = 0; i < m_ELogsSelected.GetCount(); i++)
		{
			m_ELogsSelected.GetText(i,sItem);
			sTemp += sItem;
			if(i < m_ELogsSelected.GetCount() - 1)
				sTemp += ";";
		}
		Reg.SetRegValue(hKey,"LogsToSave",REG_EXPAND_SZ,
			(LPBYTE)sTemp.GetBuffer(sTemp.GetLength()),
			sTemp.GetLength());
		sTemp.Empty();

		Reg.SetRegValue(hKey,"EventMdb",REG_EXPAND_SZ,
			(LPBYTE)m_sEventMdbName.GetBuffer(m_sEventMdbName.GetLength()),
			m_sEventMdbName.GetLength());

		Reg.CloseRegKey(hKey);
	}
}
