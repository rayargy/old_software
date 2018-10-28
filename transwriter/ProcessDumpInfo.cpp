// ProcessDumpInfo.cpp: implementation of the CProcessDumpInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcessDumpInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessDumpInfo::CProcessDumpInfo()
{

}

CProcessDumpInfo::~CProcessDumpInfo()
{

}

int CProcessDumpInfo::GetDebugLevel()
{
	return m_iDebugLevel;
}

void CProcessDumpInfo::SetDebugLevel(int iDebugLevel)
{
	m_iDebugLevel = iDebugLevel;
}

CString CProcessDumpInfo::GetConnectString()
{
	return m_sConnect;
}

void CProcessDumpInfo::SetConnectString(CString sConnect)
{
	m_sConnect = sConnect;
}
