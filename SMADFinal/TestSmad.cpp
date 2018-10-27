// TestSmad.cpp : implementation file
//

#include "stdafx.h"
#include "SMAD.h"
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

/////////////////////////////////////////////////////////////////////////////
// CTestSmad dialog


CTestSmad::CTestSmad(CWnd* pParent /*=NULL*/)
	: CDialog(CTestSmad::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestSmad)
	m_sProc = _T("");
	//}}AFX_DATA_INIT
}


void CTestSmad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestSmad)
	DDX_Control(pDX, IDC_PROGRESS1, m_SmadProgress);
	DDX_Text(pDX, IDC_EDIT1, m_sProc);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestSmad, CDialog)
	//{{AFX_MSG_MAP(CTestSmad)
	ON_BN_CLICKED(IDC_TESTBUTTON, OnTestbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestSmad message handlers

BOOL CTestSmad::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTestSmad::OnTestbutton() 
{
	// TODO: Add your control notification handler code here
	CSMADApp* p = (CSMADApp *)AfxGetApp();

	m_SmadProgress.SetRange(0,8);
	m_SmadProgress.SetStep(1);

	m_Log->GetInstance();
	m_Log->WriteLogEntry("Begin SMAD",0);

	CServiceControl SC;

	m_sProc = "Stopping Services";
	UpdateData(false);
	UpdateWindow();

	SC.StopServices();
	m_SmadProgress.StepIt();
	AfxMessageBox("Services Stopped, Press OK to continue",MB_OK);

	CBackupFiles BF;

	m_sProc = "Copying Files";
	UpdateData(false);
	UpdateWindow();

	BF.CopyFiles();
	m_SmadProgress.StepIt();
	AfxMessageBox("Files Copied, Press OK to continue",MB_OK);

	m_sProc = "Moving Files";
	UpdateData(false);
	UpdateWindow();

	BF.MoveFiles();
	m_SmadProgress.StepIt();
	AfxMessageBox("Files moved, Press OK to continue",MB_OK);

	m_sProc = "Starting Services";
	UpdateData(false);
	UpdateWindow();

	SC.StartServices();
	m_SmadProgress.StepIt();
	AfxMessageBox("Services Started, Press OK to continue",MB_OK);

	SC.DestroyServiceLists();

	m_sProc = "Processing Event Logs";
	UpdateData(false);
	UpdateWindow();

	p->ProcessEventLogs();
	m_SmadProgress.StepIt();
	AfxMessageBox("Event Logs Processed, Press OK to continue",MB_OK);

	m_sProc = "Processing Version Information";
	UpdateData(false);
	UpdateWindow();

	p->ProcessVersionInfo();
	m_SmadProgress.StepIt();
	AfxMessageBox("Version information processed, Press OK to continue",MB_OK);

	CSaveRegKey SRK;

	m_sProc = "Saving registry key";
	UpdateData(false);
	UpdateWindow();

	SRK.SaveKey("SOFTWARE\\CTRSystems");
	m_SmadProgress.StepIt();
	AfxMessageBox("Registry key saved, Press OK to continue",MB_OK);

	m_sProc = "Creating archive file";
	UpdateData(false);
	UpdateWindow();

	p->CreateCabArchive();
	m_SmadProgress.StepIt();
	AfxMessageBox("Archive created, Press OK to continue",MB_OK);
	Sleep(2000);

	m_SmadProgress.SetPos(0);
}
