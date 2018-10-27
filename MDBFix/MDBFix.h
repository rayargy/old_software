// MDBFix.h : main header file for the MDBFIX application
//

#if !defined(AFX_MDBFIX_H__66F29135_2A0F_11D3_804B_0090271354E9__INCLUDED_)
#define AFX_MDBFIX_H__66F29135_2A0F_11D3_804B_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMDBFixApp:
// See MDBFix.cpp for the implementation of this class
//

class CMDBFixApp : public CWinApp
{
public:
	CMDBFixApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDBFixApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMDBFixApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDBFIX_H__66F29135_2A0F_11D3_804B_0090271354E9__INCLUDED_)
