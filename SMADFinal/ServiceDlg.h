#if !defined(AFX_SERVICEDLG_H__472ADA57_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_SERVICEDLG_H__472ADA57_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ServiceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServiceDlg dialog

class CServiceDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CServiceDlg)

// Construction
public:
	struct SvcStartList
	{
		CString strServiceName;
		DWORD	dwServiceTimeOut;
	};

	CServiceDlg();
	~CServiceDlg();
	bool m_bTimeOutChanged;
	bool m_bDblClick2;
	bool m_bDblClick1;
	bool m_bMoveDown;
	bool m_bMoveUp;
	void GetServiceList();
	void InitServiceWnd();
	void UpdateList();
	BOOL SaveRegistrySettings();
	static SvcStartList *m_SvcStartList;
	SvcStartList *m_TempSvcList;
// Dialog Data
	//{{AFX_DATA(CServiceDlg)
	enum { IDD = IDD_SERVICEDIALOG };
	CSpinButtonCtrl	m_TimeOutSpin;
	CListBox	m_ServiceListOut;
	CListBox	m_ServiceListIn;
	DWORD	m_dwTimeOutVal;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CServiceDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CServiceDlg)
	afx_msg void OnDblclkServicelist1();
	afx_msg void OnDblclkServicelist2();
	afx_msg void OnSelchangeServicelist2();
	afx_msg void OnMovedownbutton();
	afx_msg void OnMoveupbutton();
	afx_msg void OnChangeTimeoutedit();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposTimeoutspin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOutofmemoryTimeoutspin(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVICEDLG_H__472ADA57_678E_11D3_80E1_0090271354E9__INCLUDED_)
