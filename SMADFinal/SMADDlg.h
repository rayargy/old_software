#if !defined(AFX_SMADDLG_H__472ADA56_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_SMADDLG_H__472ADA56_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SMADDlg.h : header file
//
#include "BackupDlg.h"
#include "CabFileDlg.h"
#include "EventLogDlg.h"
#include "RegistryDlg.h"
#include "ServiceDlg.h"
#include "VersionInfoDlg.h"
#include "SMADErrorLogDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CSMADDlg

class CSMADDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSMADDlg)

// Construction
public:
	CSMADDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSMADDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
// Attributes
public:
	CServiceDlg m_Page1;
	CBackupDlg m_Page2;
	CEventLogDlg m_Page3;
	CRegistryDlg m_Page4;
	CVersionInfoDlg m_Page5;
	CCabFileDlg m_Page6;
	CSMADErrorLogDlg m_Page7;
	CSMADErrorLog* m_Log;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSMADDlg)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	void LoadMdbConfig();
	virtual ~CSMADDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSMADDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMADDLG_H__472ADA56_678E_11D3_80E1_0090271354E9__INCLUDED_)
