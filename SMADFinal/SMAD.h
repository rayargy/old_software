// SMAD.h : main header file for the SMAD application
//

#if !defined(AFX_SMAD_H__472ADA4C_678E_11D3_80E1_0090271354E9__INCLUDED_)
#define AFX_SMAD_H__472ADA4C_678E_11D3_80E1_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "MDBMaintenance.h"

#define SMADKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
#define END_SERVICE_LIST (-1)
/////////////////////////////////////////////////////////////////////////////
// CSMADApp:
// See SMAD.cpp for the implementation of this class
//

class CSMADApp : public CWinApp
{
public:
	CSMADErrorLog* m_Log;
	void ProcessVersionInfo();
	void ProcessEventLogs();
	void CreateCabArchive();
	bool CreateAccessDb(CMDBMaintenance*);
	void RunSilent();
	bool CheckSMADConfig();
	CSMADApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSMADApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSMADApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SMAD_H__472ADA4C_678E_11D3_80E1_0090271354E9__INCLUDED_)
