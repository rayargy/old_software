// ServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
#include "ServiceDlg.h"
#include "Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CServiceDlg::SvcStartList *CServiceDlg::m_SvcStartList = NULL;
/////////////////////////////////////////////////////////////////////////////
// CServiceDlg property page

IMPLEMENT_DYNCREATE(CServiceDlg, CPropertyPage)

CServiceDlg::CServiceDlg() : CPropertyPage(CServiceDlg::IDD)
{
	//{{AFX_DATA_INIT(CServiceDlg)
	m_dwTimeOutVal = 0;
	//}}AFX_DATA_INIT
}

CServiceDlg::~CServiceDlg()
{
}

void CServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServiceDlg)
	DDX_Control(pDX, IDC_TIMEOUTSPIN, m_TimeOutSpin);
	DDX_Control(pDX, IDC_SERVICELIST2, m_ServiceListOut);
	DDX_Control(pDX, IDC_SERVICELIST1, m_ServiceListIn);
	DDX_Text(pDX, IDC_TIMEOUTEDIT, m_dwTimeOutVal);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServiceDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CServiceDlg)
	ON_LBN_DBLCLK(IDC_SERVICELIST1, OnDblclkServicelist1)
	ON_LBN_DBLCLK(IDC_SERVICELIST2, OnDblclkServicelist2)
	ON_LBN_SELCHANGE(IDC_SERVICELIST2, OnSelchangeServicelist2)
	ON_BN_CLICKED(IDC_MOVEDOWNBUTTON, OnMovedownbutton)
	ON_BN_CLICKED(IDC_MOVEUPBUTTON, OnMoveupbutton)
	ON_EN_CHANGE(IDC_TIMEOUTEDIT, OnChangeTimeoutedit)
	ON_WM_CLOSE()
	ON_NOTIFY(UDN_DELTAPOS, IDC_TIMEOUTSPIN, OnDeltaposTimeoutspin)
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_TIMEOUTSPIN, OnOutofmemoryTimeoutspin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServiceDlg message handlers

BOOL CServiceDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	InitServiceWnd();
	GetServiceList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CServiceDlg::OnApply() 
{
	// TODO: Add your specialized code here and/or call the base class
	SaveRegistrySettings();	
	return CPropertyPage::OnApply();
}

void CServiceDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete[] m_SvcStartList;
	CPropertyPage::PostNcDestroy();
}

void CServiceDlg::OnDblclkServicelist1() 
{
	// TODO: Add your control notification handler code here
	CString sItem;

	m_bDblClick1 = true;

	int iListItem = m_ServiceListIn.GetCurSel();
	m_ServiceListIn.GetText(iListItem,sItem);

	if(m_ServiceListOut.FindString(-1,sItem) == LB_ERR)
	{
		int iItemPos = m_ServiceListOut.FindString(-1,sItem);
		if(iItemPos != -1)
			m_ServiceListOut.SetCurSel(iItemPos);
		m_ServiceListOut.AddString(sItem);
		UpdateList();
	}
}

void CServiceDlg::OnDblclkServicelist2() 
{
	// TODO: Add your control notification handler code here

	m_bDblClick2 = true;

	int iListItem = m_ServiceListOut.GetCurSel();
	m_ServiceListOut.DeleteString(iListItem);
	UpdateList();
}

void CServiceDlg::OnSelchangeServicelist2() 
{
	// TODO: Add your control notification handler code here
	CString sItem;

	int iListItem = m_ServiceListOut.GetCurSel();
	m_ServiceListOut.GetText(iListItem,sItem);

	m_dwTimeOutVal = 0;
	// TODO: Add your control notification handler code here
	for(int i = 0;m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST; i++)
	{
		if(m_SvcStartList[i].strServiceName.Compare(sItem) == 0)
			m_dwTimeOutVal = m_SvcStartList[i].dwServiceTimeOut/1000;
	}
	UpdateData(false);
}

void CServiceDlg::OnMovedownbutton() 
{
	// TODO: Add your control notification handler code here
	CString sItem1,sItem2;

	m_bMoveDown = true;

	int iListItem = m_ServiceListOut.GetCurSel();
	if(iListItem < m_ServiceListOut.GetCount() - 1)
	{
		m_ServiceListOut.GetText(iListItem,sItem1);
		m_ServiceListOut.GetText(iListItem + 1,sItem2);
		m_ServiceListOut.InsertString(iListItem + 1,sItem1);
		m_ServiceListOut.DeleteString(iListItem);
		m_ServiceListOut.InsertString(iListItem,sItem2);
		m_ServiceListOut.DeleteString(iListItem + 2);
		m_ServiceListOut.SetCurSel(iListItem + 1);
	}
	else
		AfxMessageBox("This is the bottom of the list.",MB_OK);
}

void CServiceDlg::OnMoveupbutton() 
{
	// TODO: Add your control notification handler code here
	CString sItem1,sItem2;

	m_bMoveUp = true;

	int iListItem = m_ServiceListOut.GetCurSel();
	if(iListItem != 0)
	{
		m_ServiceListOut.GetText(iListItem,sItem1);
		m_ServiceListOut.GetText(iListItem - 1,sItem2);
		m_ServiceListOut.InsertString(iListItem - 1,sItem1);
		m_ServiceListOut.DeleteString(iListItem);
		m_ServiceListOut.InsertString(iListItem,sItem2);
		m_ServiceListOut.DeleteString(iListItem + 1);
		m_ServiceListOut.SetCurSel(iListItem - 1);
	}
	else
		AfxMessageBox("This is the top of the list",MB_OK);
}

void CServiceDlg::OnChangeTimeoutedit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_CHANGE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	CString sItem;

	m_bTimeOutChanged = true;

	int iListItem = m_ServiceListOut.GetCurSel();
	if(iListItem > -1)
	{
		m_ServiceListOut.GetText(iListItem,sItem);

		UpdateData(true);
		for(int i = 0;m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST; i++)
		{
			if(m_SvcStartList[i].strServiceName.Compare(sItem) == 0)
				m_SvcStartList[i].dwServiceTimeOut = m_dwTimeOutVal * 1000;
		}
	}
}

void CServiceDlg::InitServiceWnd()
{
	HKEY hKey;
	long lRes = 0;
	CRegistry Reg;
	DWORD dwSubKeys = 0,dwMaxSubKeyLen = 0,dwValues = 0,
		dwMaxValueNameLen = 0,dwMaxValueLen = 0,
		dwName = 0,dwReserved = 0,dwClass = 128;
	FILETIME ftLastWriteTime;
	
	hKey = Reg.OpenRegKey(&lRes,"SYSTEM\\CurrentControlSet\\Services");
	if(lRes == ERROR_SUCCESS)
	{
		HKEY hServiceKey;
		LPTSTR sKeyName;

		lRes = Reg.QueryRegKey(hKey,
			&dwSubKeys,&dwMaxSubKeyLen,&dwValues,&dwMaxValueNameLen,&dwMaxValueLen);

		sKeyName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,dwMaxSubKeyLen + 1);

		for(int i = 0; (DWORD)i < dwSubKeys; i++)
		{
			dwName = dwMaxSubKeyLen;
			lRes = RegEnumKeyEx(hKey,(DWORD)i,sKeyName,&dwName,NULL,NULL,NULL,&ftLastWriteTime );

			hServiceKey = Reg.OpenRegKey(&lRes,sKeyName,hKey);
			if(lRes == ERROR_SUCCESS)
			{
				CString sTemp;
				DWORD dwType = Reg.FetchDWValue(&lRes,hServiceKey,"Type");
				if(lRes == ERROR_SUCCESS && dwType != 1)
					m_ServiceListIn.AddString(sKeyName);
				Reg.CloseRegKey(hServiceKey);
			}
		}
		LocalFree(sKeyName);
		Reg.CloseRegKey(hKey);
	}
}

void CServiceDlg::GetServiceList()
{
	CRegistry CReg;
	long lRes = 0;
	HKEY hKey;
	DWORD dwType = 0,dwcbData = 0,dwTemp = 0,dwCount = 0;
	LPBYTE bVal,lpTemp;

	hKey = CReg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD",HKEY_LOCAL_MACHINE);

	if(lRes == ERROR_SUCCESS)
	{
		lRes = CReg.QueryRegValue(hKey,"StartServiceList",&dwType,NULL,&dwcbData);
		bVal = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwcbData + 1);
		lRes = CReg.QueryRegValue(hKey,"StartServiceList",&dwType,bVal,&dwcbData);

		dwCount = dwcbData/72;
		m_SvcStartList = new SvcStartList[dwCount + 1];

		if((lRes == ERROR_SUCCESS || dwCount == 0) && m_SvcStartList != NULL)
		{
			lpTemp = bVal;
			for(int i = 0; (DWORD)i < dwCount; i++)
			{
				m_SvcStartList[i].strServiceName = lpTemp;
				m_ServiceListOut.AddString((LPTSTR)lpTemp);

				memcpy(&m_SvcStartList[i].dwServiceTimeOut,lpTemp + 68,sizeof(DWORD));
				lpTemp += 72;
			}
			m_SvcStartList[i].dwServiceTimeOut = END_SERVICE_LIST;
		}

		LocalFree(bVal);
		CReg.CloseRegKey(hKey);
	}
}

void CServiceDlg::UpdateList()
{

	int j = m_ServiceListOut.GetCount() + 1;
	m_TempSvcList = new SvcStartList[m_ServiceListOut.GetCount() + 1];
	for(int iBoxPos = 0; iBoxPos < m_ServiceListOut.GetCount(); iBoxPos++)
	{
		CString sItem;
		m_ServiceListOut.GetText(iBoxPos,sItem);
		for(int i = 0; m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST; i++)
		{
			if(sItem == m_SvcStartList[i].strServiceName)
			{
				m_TempSvcList[iBoxPos].strServiceName = m_SvcStartList[i].strServiceName;
				m_TempSvcList[iBoxPos].dwServiceTimeOut = m_SvcStartList[i].dwServiceTimeOut;
			}
		}
		if(iBoxPos == i)
		{
			int iListItem = m_ServiceListOut.GetCurSel();
			m_ServiceListOut.GetText(iBoxPos,m_TempSvcList[iBoxPos].strServiceName);
			m_TempSvcList[iBoxPos].dwServiceTimeOut = 30000;
		}			
	}

	m_TempSvcList[iBoxPos].strServiceName.Empty();
	m_TempSvcList[iBoxPos].dwServiceTimeOut = END_SERVICE_LIST;
	
	delete[] m_SvcStartList;
	m_SvcStartList = NULL;
	m_SvcStartList = m_TempSvcList;
}

BOOL CServiceDlg::SaveRegistrySettings() 
{
	// TODO: Add your specialized code here and/or call the base class
	LPBYTE lpStart = NULL,lpStop = NULL;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	int iRevPos = m_ServiceListOut.GetCount() - 1;

	// TODO: Add your control notification handler code here
	if(m_ServiceListOut.GetCount() > 0)
	{
		lpStart = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,m_ServiceListOut.GetCount() * 72);
		lpStop = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,m_ServiceListOut.GetCount() * 72);
		for(int iPos = 0; iPos < m_ServiceListOut.GetCount(); iPos++)
		{
			for(int i = 0; m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST; i++)
			{
				CString sItem;
				m_ServiceListOut.GetText(iPos,sItem);
				if(m_SvcStartList[i].strServiceName.Compare(sItem) == 0)
				{
					memcpy(lpStart + (iPos * 72),
						m_SvcStartList[i].strServiceName,m_SvcStartList[i].strServiceName.GetLength());
					memcpy(lpStop + (iRevPos * 72),
						m_SvcStartList[i].strServiceName,m_SvcStartList[i].strServiceName.GetLength());

					memcpy(lpStart + (iPos * 72) + 68,&m_SvcStartList[i].dwServiceTimeOut,sizeof(DWORD));
					memcpy(lpStop + (iRevPos * 72) + 68,&m_SvcStartList[i].dwServiceTimeOut,sizeof(DWORD));
				}
			}

			if(iPos == m_ServiceListOut.GetCount() - 1)
			{
				hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
				if(lRes == ERROR_SUCCESS && m_ServiceListOut.GetCount != 0)
				{
					lRes = Reg.SetRegValue(hKey,"StartServiceList",REG_BINARY,lpStart,(DWORD)(i * 72));
					lRes = Reg.SetRegValue(hKey,"StopServiceList",REG_BINARY,lpStop,(DWORD)(i * 72));
					Reg.CloseRegKey(hKey);
				}
			}
			iRevPos--;
		}
	}
	else
	{
		hKey = Reg.OpenRegKey(&lRes,"SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD");
		if(lRes == ERROR_SUCCESS && m_ServiceListOut.GetCount != 0)
		{
			lRes = Reg.SetRegValue(hKey,"StartServiceList",REG_BINARY,NULL,0);
			lRes = Reg.SetRegValue(hKey,"StopServiceList",REG_BINARY,NULL,0);
			Reg.CloseRegKey(hKey);
		}
	}

	if(lpStart != NULL)
		LocalFree(lpStart);
	if(lpStop != NULL)
		LocalFree(lpStop);
	
	return CPropertyPage::OnApply();
}

void CServiceDlg::OnDeltaposTimeoutspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	CString sItem;
	
	m_dwTimeOutVal = m_TimeOutSpin.GetPos() - 65506;

	int iListItem = m_ServiceListOut.GetCurSel();
	if(iListItem != -1)
	{
		m_ServiceListOut.GetText(iListItem,sItem);

		// TODO: Add your control notification handler code here
		for(int i = 0;m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST; i++)
		{
			if(m_SvcStartList[i].strServiceName.Compare(sItem) == 0)
				m_SvcStartList[i].dwServiceTimeOut = m_dwTimeOutVal;
		}
		UpdateData(false);
	}
	else
		AfxMessageBox("Please highlight a service under the 'Currently selected services'\nbefore attempting to adjust the time out value",MB_OK);


	*pResult = 0;
}

void CServiceDlg::OnOutofmemoryTimeoutspin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}
