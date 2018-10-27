// ServiceMain.h: interface for the CServiceMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICEMAIN_H__0AFEB851_0CBF_11D1_A5CE_00AA00BBBEE3__INCLUDED_)
#define AFX_SERVICEMAIN_H__0AFEB851_0CBF_11D1_A5CE_00AA00BBBEE3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CServiceMain  
{
public:
	BOOL InitService();
	void ResumeService();
	void PauseService();
	void StopService();
	BOOL SendStatusToSCM ( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint, DWORD dwWaitHint );
	void Terminate( DWORD error );
	CServiceMain();
	virtual ~CServiceMain();

	char * SERVICE_NAME;		// This holds the name for the service
	HANDLE m_hThreadHandle;		// Thread for the actual work
	BOOL m_bRunningService;		// Flag holding current state of service
	BOOL m_bPauseService;		// Flag holding current state of service
	int m_iSvcDelay;			// The service delay interval in ms.
	DWORD m_dwServiceStatusHandle;	// Handle used to communicate status
								// info with the SCM.
	HANDLE m_hTerminateEvent;	// Event used to hold ServiceMain from completing

};

#endif // !defined(AFX_SERVICEMAIN_H__0AFEB851_0CBF_11D1_A5CE_00AA00BBBEE3__INCLUDED_)
