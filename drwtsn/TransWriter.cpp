// TransWriter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ErrorHandlers.h"
#include "TransWriter.h"
#include "ServiceMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTransWriterApp

BEGIN_MESSAGE_MAP(CTransWriterApp, CWinApp)
	//{{AFX_MSG_MAP(CTransWriterApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTransWriterApp construction

CTransWriterApp::CTransWriterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTransWriterApp object

CTransWriterApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTransWriterApp initialization

BOOL CTransWriterApp::InitInstance()
{
	// Socket setup: if not using sockets, can be removed.
	if (!AfxSocketInit())
	{
		ErrorHandler ( "Socket Initialization Failure!", 
			IDP_SOCKETS_INIT_FAILED );
		return FALSE;
	}

	// Start the service threads.
	CServiceMain svc;

	// Since the service has terminated, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
