// ServiceControl.cpp: implementation of the CServiceControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServiceControl.h"
#include "registry.h"

#define SMADKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"

CServiceControl::SvcStopList *CServiceControl::m_SvcStopList = NULL;
CServiceControl::SvcStartList *CServiceControl::m_SvcStartList = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceControl::CServiceControl()
{
	CRegistry CReg;
	long lRes = 0;
	HKEY hKey;
	DWORD dwType = 0,dwcbData = 0,dwTemp = 0,dwCount = 0;
	LPBYTE bVal,lpTemp;

	m_Log->GetInstance();

	m_Log->WriteLogEntry("Constructor for CServiceControl",0);

	hKey = CReg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		if(CReg.m_dwDisposition == REG_OPENED_EXISTING_KEY)
		{

			lRes = CReg.QueryRegValue(hKey,"StopServiceList",&dwType,NULL,&dwcbData);
			bVal = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwcbData + 1);
			lRes = CReg.QueryRegValue(hKey,"StopServiceList",&dwType,bVal,&dwcbData);

			if(dwcbData != 0)
			{
				dwCount = dwcbData/72;
				if(dwCount != 0)
					m_SvcStopList = new SvcStopList[dwCount + 1];
			}

			if(lRes == ERROR_SUCCESS && m_SvcStopList != NULL)
			{
				lpTemp = bVal;
				for(int i = 0; (DWORD)i < dwCount; i++)
				{
					m_SvcStopList[i].strServiceName = lpTemp;

					memcpy(&m_SvcStopList[i].dwServiceTimeOut,lpTemp + 68,sizeof(DWORD));
					m_SvcStopList[i + 1].dwServiceTimeOut = END_SERVICE_LIST;
					lpTemp += 72;
				}
			}
			if(m_SvcStopList == NULL)
			{
				m_SvcStopList = new SvcStopList[1];
				m_SvcStopList[0].dwServiceTimeOut = END_SERVICE_LIST;
			}



			lRes = CReg.QueryRegValue(hKey,"StartServiceList",&dwType,NULL,&dwcbData);
			bVal = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwcbData + 1);
			lRes = CReg.QueryRegValue(hKey,"StartServiceList",&dwType,bVal,&dwcbData);

			if(dwcbData != 0)
			{
				dwCount = dwcbData/72;
				if(dwCount != 0)
					m_SvcStartList = new SvcStartList[dwCount + 1];
			}

			if(lRes == ERROR_SUCCESS && m_SvcStartList != NULL)
			{
				lpTemp = bVal;
				for(int i = 0; (DWORD)i < dwCount; i++)
				{
					m_SvcStartList[i].strServiceName = lpTemp;

					memcpy(&m_SvcStartList[i].dwServiceTimeOut,lpTemp + 68,sizeof(DWORD));
					m_SvcStartList[i + 1].dwServiceTimeOut = END_SERVICE_LIST;
					lpTemp += 72;
				}
			}
			if(m_SvcStartList == NULL)
			{
				m_SvcStartList = new SvcStartList[1];
				m_SvcStartList[0].dwServiceTimeOut = END_SERVICE_LIST;
			}

		}

		LocalFree(bVal);
		CReg.CloseRegKey(hKey);
	}
}

CServiceControl::~CServiceControl()
{
}

void CServiceControl::DestroyServiceLists()
{
	if(m_SvcStopList != NULL)
		delete[] m_SvcStopList;
	if(m_SvcStartList != NULL)
		delete[] m_SvcStartList;
}

void CServiceControl::StopServices()
{
	BOOL bRetVal = FALSE;
	DWORD dwl = 0,dwStatus = 0;
	int i = 0;
	CString strMsg;
	LPVOID lpMsgBuf;

	// Open the SCM and the desired service.
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	// error condition
	while(m_SvcStopList[i].dwServiceTimeOut != END_SERVICE_LIST)
	{
		SC_HANDLE hService = OpenService(hSCM, m_SvcStopList[i].strServiceName, SERVICE_ALL_ACCESS);

		strMsg.Format("\tCServiceControl::StopServices(): Stopping %s service",
			m_SvcStopList[i].strServiceName);
		m_Log->WriteLogEntry(strMsg,0);

		// error condition
		// Tell the service to stop.
		SERVICE_STATUS ss;
		::QueryServiceStatus(hService, &ss);
		// error condition
		if(ss.dwCurrentState != SERVICE_STOPPED)
		{
			if(!ControlService(hService, SERVICE_CONTROL_STOP, &ss))
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);

				strMsg.Format("\tCServiceControl::StartServices(): %s",(LPTSTR)lpMsgBuf);
				strMsg.TrimRight();
				m_Log->WriteLogEntry(strMsg,0);
				LocalFree(lpMsgBuf);
			}

			// Wait up to 60 seconds for the service to stop.
			bRetVal = WaitForServiceState(hService, SERVICE_STOPPED, &ss, m_SvcStopList[i].dwServiceTimeOut);
			if(!bRetVal)
			{
				strMsg.Format("\tCServiceControl::StopServices() failed for %s",m_SvcStopList[i].strServiceName);
				m_Log->WriteLogEntry(strMsg,0);
			}
		}

		// Close the service and the SCM.
		CloseServiceHandle(hService);
		i++;
	}
	CloseServiceHandle(hSCM);
}

void CServiceControl::StartServices()
{
	BOOL bRetVal = FALSE;
	DWORD dwl = 0,dwStatus = 0;
	int i = 0;
	CString strMsg;
	LPVOID lpMsgBuf;

	// Open the SCM and the desired service.
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	// error condition
	while(m_SvcStartList[i].dwServiceTimeOut != END_SERVICE_LIST)
	{
		SC_HANDLE hService = OpenService(hSCM, m_SvcStartList[i].strServiceName, SERVICE_ALL_ACCESS);
		// error condition
		strMsg.Format("\tCServiceControl::StartServices(): Starting %s service",
			m_SvcStartList[i].strServiceName);
		m_Log->WriteLogEntry(strMsg,0);

		SERVICE_STATUS ss;
		::QueryServiceStatus(hService, &ss);
		// error condition
		if(ss.dwCurrentState != SERVICE_RUNNING)
		{
			// Tell the service to start.

			if(!StartService(hService,NULL,NULL))
			{
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,0,NULL);

				strMsg.Format("\tCServiceControl::StartServices(): %s",(LPTSTR)lpMsgBuf);
				strMsg.TrimRight();
				m_Log->WriteLogEntry(strMsg,0);
				LocalFree(lpMsgBuf);
			}

			// Wait up to the configured number of seconds for the service to stop.
			bRetVal = WaitForServiceState(hService, SERVICE_RUNNING, &ss, m_SvcStartList[i].dwServiceTimeOut);
			if(!bRetVal)
			{
				strMsg.Format("\tCServiceControl::StartServices() failed for %s",m_SvcStartList[i].strServiceName);
				m_Log->WriteLogEntry(strMsg,0);
			}
		}

		// Close the service and the SCM.
		CloseServiceHandle(hService);
		i++;
	}
	CloseServiceHandle(hSCM);
}


BOOL CServiceControl::WaitForServiceState(SC_HANDLE hService, DWORD dwDesiredState,
					SERVICE_STATUS * pss, DWORD dwMilliseconds)
{
	DWORD dwLastState, dwLastCheckPoint;
	BOOL  fFirstTime = TRUE; // Don't compare state & checkpoint the first time through
	BOOL  fServiceOk = TRUE;
	int iRetry = 0;
	DWORD dwTimeout = GetTickCount() + dwMilliseconds;
	// Loop until the service reaches the desired state,
	// an error occurs, or we timeout
  
	while  (TRUE)
	{
		// Get current state of service
		fServiceOk = ::QueryServiceStatus(hService, pss);
		// If we can't query the service, we're done
		// error condition
		if (!fServiceOk) break;
		// If the service reaches the desired state, we're done
		if (pss->dwCurrentState == dwDesiredState)
			break;

		// If we timed-out, we're done

		if ((dwMilliseconds != INFINITE) && (dwTimeout < GetTickCount()))
		{
			SetLastError(ERROR_TIMEOUT);
			break;
		}
		// If this is our first time, save the service's state & checkpoint
		if (fFirstTime)
		{
			dwLastState = pss->dwCurrentState;
			dwLastCheckPoint = pss->dwCheckPoint;
			fFirstTime = FALSE;
		}
		else
		{
			// If not first time & state has changed, save state & checkpoint
			if (dwLastState != pss->dwCurrentState)
			{
				dwLastState = pss->dwCurrentState;
				dwLastCheckPoint = pss->dwCheckPoint;
			}
			else
			{
				// State hasn't change, check that checkpoint is increasing
				if (pss->dwCheckPoint > dwLastCheckPoint)
				{
					// Checkpoint has increased, save checkpoint
					dwLastCheckPoint = pss->dwCheckPoint;
				}
				else
				{
					// Checkpoint hasn't increased, service failed, we're done!
					if(iRetry < 5)
					{
						Sleep(2000);
						iRetry++;
					}
					else
					{
						fServiceOk = FALSE;
						break;
					}
				}
			}
		}      // We're not done, wait the specified period of time
		Sleep(pss->dwWaitHint);
	}
	// Note: The last SERVICE_STATUS is returned to the caller so
	// that the caller can check the service state and error codes.
	return(fServiceOk);
}