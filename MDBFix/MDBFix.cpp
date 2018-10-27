// MDBFix.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MDBFix.h"
#include "MDBMaintenance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMDBFixApp

BEGIN_MESSAGE_MAP(CMDBFixApp, CWinApp)
	//{{AFX_MSG_MAP(CMDBFixApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDBFixApp construction

CMDBFixApp::CMDBFixApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMDBFixApp object

CMDBFixApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMDBFixApp initialization

BOOL CMDBFixApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	CMDBMaintenance Mdb("SOFTWARE\\CTRSystems\\PARCS-CS\\Access\\TransBuff");
//	CMDBMaintenance Mdb("");
	long lDaoErr = 0;

	lDaoErr = 3343;

	Mdb.InitMdb(lDaoErr);
	Mdb.DestroyDaoPtr();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
