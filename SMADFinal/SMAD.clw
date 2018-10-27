; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSMADDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "SMAD.h"

ClassCount=11
Class1=CSMADApp
Class2=CAboutDlg

ResourceCount=10
Resource1=IDD_BACKUPDIALOG
Resource2=IDR_MAINFRAME
Class3=CSMADDlg
Resource3=IDD_SERVICEDIALOG
Resource4=IDD_ABOUTBOX
Resource5=IDD_CABDIALOG
Resource6=IDD_EVENTLOGDIALOG
Resource7=IDD_VERSIONDIALOG
Resource8=IDD_REGISTRYDIALOG
Class4=CServiceDlg
Class5=CBackupDlg
Class6=CCabFileDlg
Class7=CSMADErrorLogDlg
Class8=CVersionInfoDlg
Class9=CRegistryDlg
Class10=CEventLogDlg
Resource9=IDD_ERRORDIALOG
Class11=CTestSmad
Resource10=IDD_TESTSMAD

[CLS:CSMADApp]
Type=0
HeaderFile=SMAD.h
ImplementationFile=SMAD.cpp
Filter=N

[CLS:CAboutDlg]
Type=0
HeaderFile=SMADDlg.h
ImplementationFile=SMADDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[CLS:CSMADDlg]
Type=0
HeaderFile=SMADDlg.h
ImplementationFile=SMADDlg.cpp
BaseClass=CPropertySheet
Filter=W
LastObject=CSMADDlg
VirtualFilter=hWC

[DLG:IDD_SERVICEDIALOG]
Type=1
Class=CServiceDlg
ControlCount=9
Control1=IDC_SERVICELIST1,listbox,1352728835
Control2=IDC_STATIC,static,1342308352
Control3=IDC_SERVICELIST2,listbox,1352728833
Control4=IDC_TIMEOUTEDIT,edit,1350633602
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_MOVEUPBUTTON,button,1342242816
Control8=IDC_MOVEDOWNBUTTON,button,1342242816
Control9=IDC_TIMEOUTSPIN,msctls_updown32,1342177312

[DLG:IDD_BACKUPDIALOG]
Type=1
Class=CBackupDlg
ControlCount=10
Control1=IDC_COPYLIST,listbox,1352728835
Control2=IDC_MOVELIST,listbox,1352728835
Control3=IDC_BACKUPLOC,edit,1350633600
Control4=IDC_STATIC,static,1342308352
Control5=IDC_ADDCOPYBUTTON,button,1342242816
Control6=IDC_ADDMOVEBUTTON,button,1342242816
Control7=IDC_CHANGEBUTTON,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352

[DLG:IDD_REGISTRYDIALOG]
Type=1
Class=CRegistryDlg
ControlCount=3
Control1=IDC_REGEDIT,edit,1350633600
Control2=IDC_REGBUTTON,button,1342242816
Control3=IDC_STATIC,static,1342308352

[DLG:IDD_VERSIONDIALOG]
Type=1
Class=CVersionInfoDlg
ControlCount=6
Control1=IDC_VERSIONLIST,listbox,1352728835
Control2=IDC_STATIC,static,1342308352
Control3=IDC_VERSIONMDB,edit,1350633600
Control4=IDC_VERSIONBUTTON,button,1342242816
Control5=IDC_VSLISTBUTTON,button,1342242816
Control6=IDC_STATIC,static,1342308352

[DLG:IDD_EVENTLOGDIALOG]
Type=1
Class=CEventLogDlg
ControlCount=8
Control1=IDC_EVENTLIST1,listbox,1352728835
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EVENTLIST2,listbox,1352728835
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EVENTLOGEDIT,edit,1350633600
Control6=IDC_EVENTLOGBUTTON,button,1342242816
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352

[DLG:IDD_ERRORDIALOG]
Type=1
Class=CSMADErrorLogDlg
ControlCount=6
Control1=IDC_SMADLOGEDIT,edit,1350633600
Control2=IDC_SMADLOGBUTTON,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_DEBUG,edit,1350633600
Control5=IDC_DEBUGSPIN,msctls_updown32,1342177312
Control6=IDC_STATIC,static,1342308352

[DLG:IDD_CABDIALOG]
Type=1
Class=CCabFileDlg
ControlCount=3
Control1=IDC_CABEDIT,edit,1350633600
Control2=IDC_CABBUTTON,button,1342242816
Control3=IDC_STATIC,static,1342308352

[CLS:CServiceDlg]
Type=0
HeaderFile=ServiceDlg.h
ImplementationFile=ServiceDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CServiceDlg

[CLS:CBackupDlg]
Type=0
HeaderFile=BackupDlg.h
ImplementationFile=BackupDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CBackupDlg

[CLS:CCabFileDlg]
Type=0
HeaderFile=CabFileDlg.h
ImplementationFile=CabFileDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CCabFileDlg

[CLS:CSMADErrorLogDlg]
Type=0
HeaderFile=SMADErrorLogDlg.h
ImplementationFile=SMADErrorLogDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=IDC_DEBUGSPIN

[CLS:CRegistryDlg]
Type=0
HeaderFile=RegistryDlg.h
ImplementationFile=RegistryDlg.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CRegistryDlg

[CLS:CVersionInfoDlg]
Type=0
HeaderFile=VersionInfoDlg.h
ImplementationFile=VersionInfoDlg.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_VERSIONLIST
VirtualFilter=idWC

[CLS:CEventLogDlg]
Type=0
HeaderFile=EventLogDlg.h
ImplementationFile=EventLogDlg.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CEventLogDlg
VirtualFilter=idWC

[DLG:IDD_TESTSMAD]
Type=1
Class=CTestSmad
ControlCount=3
Control1=IDC_EDIT1,edit,1350631552
Control2=IDC_TESTBUTTON,button,1342242816
Control3=IDC_PROGRESS1,msctls_progress32,1350565888

[CLS:CTestSmad]
Type=0
HeaderFile=TestSmad.h
ImplementationFile=TestSmad.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CTestSmad

