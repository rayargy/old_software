#if !defined(AFX_EVENTLOGDLG_H__472ADA5B_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_EVENTLOGDLG_H__472ADA5B_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EventLogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventLogDlg dialog

class CEventLogDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CEventLogDlg)

// Construction
public:
	void SaveRegistrySettings();
	void GetEventLogList();
	CEventLogDlg();
	~CEventLogDlg();

// Dialog Data
	//{{AFX_DATA(CEventLogDlg)
	enum { IDD = IDD_EVENTLOGDIALOG };
	CListBox	m_ELogsSelected;
	CListBox	m_EventLogList;
	CString	m_sEventMdbName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEventLogDlg)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEventLogDlg)
	afx_msg void OnDblclkEventlist1();
	afx_msg void OnDblclkEventlist2();
	afx_msg void OnEventlogbutton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTLOGDLG_H__472ADA5B_678E_11D3_80E1_0090271354E9__INCLUDED_)
