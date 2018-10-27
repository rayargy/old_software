// SMAD.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SMAD.h"
#include "SMADDlg.h"
#include "TestSmad.h"
#include "registry.h"
#include "ServiceControl.h"
#include "BackupFiles.h"
#include "SaveRegKey.h"
#include "VersionInfo.h"
#include "SMADErrorLog.h"
#include "DumpEventLog.h"
#include "registry.h"
#include "CompressFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SMADKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
#define EventKey "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD\\Event"
#define VersionKey "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD\\Version"

/////////////////////////////////////////////////////////////////////////////
// CSMADApp

BEGIN_MESSAGE_MAP(CSMADApp, CWinApp)
	//{{AFX_MSG_MAP(CSMADApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSMADApp construction

CSMADApp::CSMADApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSMADApp object

CSMADApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSMADApp initialization

BOOL CSMADApp::InitInstance()
{
	CString sCmdLine;
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	sCmdLine = this->m_lpCmdLine;

	if(sCmdLine == "/C")
	{
		CSMADDlg dlg("PARCS-CS SMAD",NULL,0);

		dlg.DoModal();
	}

	if(sCmdLine == "/T")
	{
		if(CheckSMADConfig())
		{
			CTestSmad dlg;
			m_pMainWnd = &dlg;
			int nResponse = dlg.DoModal();
		}
	}

	if(sCmdLine == "")
	{
		if(CheckSMADConfig())
			RunSilent();
	}
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

bool CSMADApp::CheckSMADConfig()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	bool bRetVal = false;

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		if(Reg.m_dwDisposition == REG_CREATED_NEW_KEY)
		{
			AfxMessageBox("Please run SMAD in configuration mode before attempting this process",MB_OK);
			Reg.DeleteRegKey(SMADKEY);
			bRetVal = false;
		}
		else
			bRetVal = true;
	}
	else
		bRetVal = false;

	return bRetVal;
}

void CSMADApp::RunSilent()
{
	m_Log->GetInstance();
	m_Log->WriteLogEntry("Begin SMAD",0);

	CServiceControl SC;

	SC.StopServices();

	CBackupFiles BF;
	BF.CopyFiles();
	BF.MoveFiles();

	SC.StartServices();

	SC.DestroyServiceLists();

	ProcessEventLogs();
	ProcessVersionInfo();

	CSaveRegKey SRK;
	SRK.SaveKey("SOFTWARE\\CTRSystems");

	m_Log->DestroyInstance();
	CreateCabArchive();
}

bool CSMADApp::CreateAccessDb(CMDBMaintenance *db)
{
	int iRetVal = 0;

	// iRetVal 1 = MDB repaired -1 = MDB Rebuilt 0 = failed
	// DisablePolling() disables polling at the commserver level!

	if(db->m_bRetVal = db->CreateMdb())
	{
		iRetVal = 1;
		db->MoveMDB();
	}

	return(iRetVal);
}

void CSMADApp::CreateCabArchive()
{
	CString sTemp,strMsg;
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CCompressFile *p;

	p = CCompressFile::GetInstance();
	
	p->CompressMain();

	// The Log file must be closed before it can be added to the cab
	m_Log->DestroyInstance();

	hKey = Reg.OpenRegKey(&lRes,SMADKEY);
	if(lRes == ERROR_SUCCESS)
	{
		sTemp = Reg.FetchStrValue(&lRes,hKey,"SMADLog");
		if(p->m_hfci)
		{
			if(!p->AddFileFCI(sTemp))
			{
				strMsg.Format("CSMADApp::CreateCabArchive() AddFileFCI failed\n\t%s",
					sTemp);
//				m_Log->WriteLogEntry(strMsg,0); //Can't do this the logfile is closed
			}
			else
				DeleteFile(sTemp);

			sTemp = p->m_sBackupPath + COleDateTime::GetCurrentTime().Format("%Y%m%d%H");
			sTemp += ".CAB";
			CopyFile(p->m_sCabName,sTemp,false);
		}
		Reg.CloseRegKey(hKey);
	}
	p->DestroyInstance();
}

void CSMADApp::ProcessEventLogs()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;

	CDumpEventLog de;
	CMDBMaintenance dbEvent(EventKey);


	if(!de.FileExists(de.m_strMdbName))
	{
		// originally created to rebuild transbuff so check that the default
		// from MDBMaintenance was not used. Set if it was.
		if(dbEvent.m_strDestinationDb.Find("TransBuff"))
		{
			hKey = Reg.OpenRegKey(&lRes,SMADKEY);
			if(lRes == ERROR_SUCCESS)
			{
				dbEvent.m_strDestinationDb = Reg.FetchStrValue(&lRes,hKey,"EventMdb");
				Reg.CloseRegKey(hKey);
			}
			if(lRes != ERROR_SUCCESS)
				dbEvent.m_strDestinationDb = "C:\\Events.mdb";
		}

		if(dbEvent.m_strTempDb == "TransTemp.mdb")
			dbEvent.m_strTempDb = "EvtTemp.mdb";
		if(CreateAccessDb(&dbEvent))
			de.DumpLogs();
	}
}

void CSMADApp::ProcessVersionInfo()
{
	CRegistry Reg;
	HKEY hKey;
	long lRes = 0;
	CMDBMaintenance dbVersion(VersionKey);

	// originally created to rebuild transbuff so check that the default
	// from MDBMaintenance was not used. Set if it was.
	if(dbVersion.m_strDestinationDb.Find("TransBuff"))
	{
		hKey = Reg.OpenRegKey(&lRes,SMADKEY);
		if(lRes == ERROR_SUCCESS)
		{
			dbVersion.m_strDestinationDb = Reg.FetchStrValue(&lRes,hKey,"VersionMdb");
			Reg.CloseRegKey(hKey);
		}
		if(lRes != ERROR_SUCCESS)
			dbVersion.m_strDestinationDb = "C:\\Version.mdb";
	}

	if(dbVersion.m_strTempDb == "TransTemp.mdb")
		dbVersion.m_strTempDb = "VSTemp.mdb";
	if(CreateAccessDb(&dbVersion))
	{
		CVersionInfo VI;
		VI.GetFileVersions();
	}
}
