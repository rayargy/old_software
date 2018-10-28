// ServiceMain.cpp: implementation of the CServiceMain class.
//
// This implementation for service handling involves the use of several
// handler threads.  The first thread, starts in the App and comes 
// to the CServiceMain constructor.
//
// The CServiceMain constructor initializes its member variables and
// then tells the Service Control Manager what function to call to
// initiate the service.
//
// The ServiceMain global function is executed by the SCM to start the
// service.  The function starts the primary service thread through
// the member function InitService.  The function also identifies to
// the SCM the ServiceCtrlHandler.
//
// When invoked, the ServiceCtrlHandler runs as its own thread to 
// control service activities and respond to requests from the SCM.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <winsvc.h>
#include "ErrorHandlers.h"
#include "TransWriter.h"
#include "ServiceMain.h"

#include "EventLog.h"
#include "PARCSMSGS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// int DumpTrans ( ); EVS no longer needed with TransBuff.mdb
int TransWriter_main ( CServiceMain *);
void ServiceMain( DWORD argc, LPTSTR *argv );
DWORD ServiceThread( LPDWORD param );
void ServiceCtrlHandler( DWORD controlCode );

CServiceMain *m_pInstanceRef;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceMain::CServiceMain()
{
	m_pInstanceRef = this;
	m_hTerminateEvent = NULL;
	m_iSvcDelay = 5000;
	m_bPauseService = FALSE;
	m_bRunningService = FALSE;
	m_hThreadHandle = 0;

	SERVICE_NAME = "TransWriter";			// The name of the service

	SERVICE_TABLE_ENTRY serviceTable[] = 
	{ 
		{
			SERVICE_NAME,
			(LPSERVICE_MAIN_FUNCTION) ServiceMain
		},
		{
			NULL,
			NULL
		}
	};
	BOOL success;

	// Register with the SCM
	success = StartServiceCtrlDispatcher(serviceTable);
	if (!success)
		ErrorHandler("In StartServiceCtrlDispatcher", GetLastError());
}

CServiceMain::~CServiceMain()
{

}

// Handle an error from ServiceMain by cleaning up
// and telling SCM that the service didn't start.
void CServiceMain::Terminate(DWORD error)
{
	// if terminateEvent has been created, close it.
	if (m_hTerminateEvent)
		CloseHandle(m_hTerminateEvent);

	// Send a message to the scm to tell about stopage
	if (m_dwServiceStatusHandle)
		SendStatusToSCM(SERVICE_STOPPED, error, 0, 0, 0);

	// If the thread has started kill it off
	if (m_hThreadHandle)
		CloseHandle(m_hThreadHandle);

	// Do not need to close serviceStatusHandle
}

// This function consolidates the activities of 
// updating the service status with
// SetServiceStatus
BOOL CServiceMain::SendStatusToSCM(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint, DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS serviceStatus;

	// Fill in all of the SERVICE_STATUS fields
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = dwCurrentState;

	// If in the process of something, then accept
	// no control events, else accept anything
	if (dwCurrentState == SERVICE_START_PENDING)
		serviceStatus.dwControlsAccepted = 0;
	else
		serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | 
			SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;

	// if a specific exit code is defines, set up
	// the win32 exit code properly
	if (dwServiceSpecificExitCode == 0)
		serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
	else
		serviceStatus.dwWin32ExitCode =  ERROR_SERVICE_SPECIFIC_ERROR;
	serviceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;

	serviceStatus.dwCheckPoint = dwCheckPoint;
	serviceStatus.dwWaitHint = dwWaitHint;

	// Pass the status record to the SCM
	success = SetServiceStatus (m_dwServiceStatusHandle, &serviceStatus);
	if (!success)
	{
		CEventLog CLog( "TransWriter" );
		success = CLog.PostError ( MSG_TWRT_STOP, NULL, 0 );
		StopService();
	}

	return success;
}

// Stops the service by allowing ServiceMain to
// complete
void CServiceMain::StopService()
{
	m_bRunningService=FALSE;
	// Set the event that is holding ServiceMain
	// so that ServiceMain can return
	SetEvent(m_hTerminateEvent);

	Sleep(100);
}

// Pauses the service
void CServiceMain::PauseService()
{
	m_bPauseService = TRUE;
	SuspendThread(m_hThreadHandle);
}

// Resumes a paused service
void CServiceMain::ResumeService()
{
	m_bPauseService=FALSE;
	ResumeThread(m_hThreadHandle);
}

// Initializes the service by starting its thread
BOOL CServiceMain::InitService()
{
	DWORD id;

	// TODO: add service global initialization code here,

	// Start the service's primary thread
	m_hThreadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE) ServiceThread, 0, 0, &id);

	m_bRunningService = ( m_hThreadHandle != 0 );
	return m_bRunningService;
}

// ServiceMain is called when the SCM wants to
// start the service. When it returns, the service
// has stopped. It therefore waits on an event
// just before the end of the function, and
// that event gets set when it is time to stop. 
// It also returns on any error because the
// service cannot start if there is an eror.
void ServiceMain(DWORD argc, LPTSTR * argv)
{
	BOOL success;
	CEventLog CLog( "TransWriter" );

	// immediately call Registration function
	m_pInstanceRef->m_dwServiceStatusHandle =
		RegisterServiceCtrlHandler(	m_pInstanceRef->SERVICE_NAME,
			(LPHANDLER_FUNCTION) ServiceCtrlHandler);
	if ( !m_pInstanceRef->m_dwServiceStatusHandle )
	{
		m_pInstanceRef->Terminate(GetLastError());
		return;
	}

	// Notify SCM of progress
	success = m_pInstanceRef->SendStatusToSCM( 
		SERVICE_START_PENDING, NO_ERROR, 0, 1, 5000);
	if (!success)
	{
		m_pInstanceRef->Terminate(GetLastError()); 
		return;
	}

	// create the termination event
	m_pInstanceRef->m_hTerminateEvent = CreateEvent (0, TRUE, FALSE, 0);
	if (!m_pInstanceRef->m_hTerminateEvent)
	{
		m_pInstanceRef->Terminate(GetLastError());
		return;
	}

	// Notify SCM of progress
	success = m_pInstanceRef->SendStatusToSCM(
		SERVICE_START_PENDING, NO_ERROR, 0, 2, 1000);
	if (!success)
	{
		m_pInstanceRef->Terminate(GetLastError()); 
		return;
	}

	// Check for startup params
	if (argc == 2)
	{
		int temp = atoi(argv[1]);
		m_pInstanceRef->m_iSvcDelay = ( temp < 1000 ? 2000 : temp );
	}

	// Notify SCM of progress
	success = m_pInstanceRef->SendStatusToSCM(
		SERVICE_START_PENDING, NO_ERROR, 0, 3, 5000);
	if (!success)
	{
		m_pInstanceRef->Terminate(GetLastError()); 
		return;
	}

	success = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	// Start the service itself
	success = m_pInstanceRef->InitService();
	if (!success)
	{
		// Post Initialization Failed Event to the Event Log
		success = CLog.PostError ( MSG_TWRT_INITFAIL, NULL, 0 );
		m_pInstanceRef->Terminate(GetLastError());
		return;
	}

	// The service is now running. 
	// Notify SCM of progress
	success = m_pInstanceRef->SendStatusToSCM(
		SERVICE_RUNNING, NO_ERROR, 0, 0, 0);
	if (!success)
	{
		m_pInstanceRef->Terminate(GetLastError()); 
		return;
	}

	// Post Start Event to the Event Log
	success = CLog.PostInfo ( MSG_TWRT_START, NULL, 0 );

	success = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

	// Wait for stop signal, and then terminate
	WaitForSingleObject (m_pInstanceRef->m_hTerminateEvent, INFINITE);

	m_pInstanceRef->Terminate(0);
}

//////////////////////////////////////////////////////////////////////
// This is where the primary thread for the service actually does its
// work.  This work usually involves launching other 'worker' threads.
//////////////////////////////////////////////////////////////////////
DWORD ServiceThread(LPDWORD param)
{
	// TODO: add final thread specific initialization code here,
	int iretVal = 1;
//	BOOL success = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
	BOOL success = SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );

//	while (iretVal)
	while(true)
	{
		// TODO: add primary thread execution code here,
		
		// Heartbeat: useful for testing, remove for production.
		iretVal = TransWriter_main (m_pInstanceRef );

		// Sleep is used instead of Timer.  If m_iSvcDelay is too
		// large, could establish a loop that calls Sleep with a
		// shorter delay.
		// Sleep(m_pInstanceRef->m_iSvcDelay);
		if(m_pInstanceRef->m_bRunningService == FALSE)
			break;
	}
	return 0;
}

// Dispatches events received from the service 
// control manager
void ServiceCtrlHandler(DWORD controlCode)
{
	int iretVal = 0;
	DWORD  currentState = 0;
	BOOL success;
	CEventLog CLog( "TransWriter" );

	switch(controlCode)
	{
		// There is no START option because
		// ServiceMain gets called on a start

		// Stop the service
		case SERVICE_CONTROL_STOP:
			// Post Stop Event to the Event Log
			success = CLog.PostWarning ( MSG_TWRT_STOP, NULL, 0 );
//			iretVal = DumpTrans ( );  // EVS no longer needed with TransBuff.mdb
			currentState = SERVICE_STOP_PENDING;
			// Tell the SCM what's happening
			success = m_pInstanceRef->SendStatusToSCM(
				SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 5000);
			// Not much to do if not successful

			// Stop the service
			m_pInstanceRef->StopService();
			return;

		// Pause the service
		case SERVICE_CONTROL_PAUSE:
			// Post Pause Event to the Event Log
			success = CLog.PostWarning ( MSG_TWRT_PAUSE, NULL, 0 );
//			iretVal = DumpTrans ( );  // EVS no longer needed with TransBuff.mdb
			if (m_pInstanceRef->m_bRunningService && 
				!m_pInstanceRef->m_bPauseService)
			{
				// Tell the SCM what's happening
				success = m_pInstanceRef->SendStatusToSCM(
					SERVICE_PAUSE_PENDING, NO_ERROR, 0, 1, 1000);
				m_pInstanceRef->PauseService();
				currentState = SERVICE_PAUSED;
			}
			break;

		// Resume from a pause
		case SERVICE_CONTROL_CONTINUE:
			// Post Continue Event to the Event Log
			success = CLog.PostInfo ( MSG_TWRT_CONTINUE, NULL, 0 );
			if (m_pInstanceRef->m_bRunningService && 
				m_pInstanceRef->m_bPauseService)
			{
				// Tell the SCM what's happening
				success = m_pInstanceRef->SendStatusToSCM(
					SERVICE_CONTINUE_PENDING, NO_ERROR, 0, 1, 1000);
					m_pInstanceRef->ResumeService();
					currentState = SERVICE_RUNNING;
			}
			break;

		// Update current status
		case SERVICE_CONTROL_INTERROGATE:
			// it will fall to bottom and send status
			break;

		// Do nothing in a shutdown. Could do cleanup
		// here but it must be very quick.
		case SERVICE_CONTROL_SHUTDOWN:
			// Post Shutdown Event to the Event Log
			success = CLog.PostWarning ( MSG_TWRT_SHUTDOWN, NULL, 0 );
			// Do nothing on shutdown
			return;

		default:
 			break;
	}
	m_pInstanceRef->SendStatusToSCM(currentState, NO_ERROR, 0, 0, 0);
}

