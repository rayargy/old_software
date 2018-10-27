// ServiceControl.h: interface for the CServiceControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICECONTROL_H__816F4201_4370_11D3_807A_0090271354E9__INCLUDED_)
#define AFX_SERVICECONTROL_H__816F4201_4370_11D3_807A_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define END_SERVICE_LIST (-1)

#include "winsvc.h"

class CServiceControl  
{

public:
	void StartServices();
	void StopServices();
	struct SvcStopList
	{
		CString strServiceName;
		DWORD	dwServiceTimeOut;
	};

	struct SvcStartList
	{
		CString strServiceName;
		DWORD	dwServiceTimeOut;
	};

	CServiceControl();
	virtual ~CServiceControl();
	void DestroyServiceLists(void);
	static SvcStopList *m_SvcStopList;
	static SvcStartList *m_SvcStartList;
	CSMADErrorLog* m_Log;

protected:
	BOOL WaitForServiceState(SC_HANDLE,DWORD,SERVICE_STATUS*,DWORD);
};

#endif // !defined(AFX_SERVICECONTROL_H__816F4201_4370_11D3_807A_0090271354E9__INCLUDED_)
