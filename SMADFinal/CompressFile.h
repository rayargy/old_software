// CompressFile.h: interface for the CCompressFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPRESSFILE_H__1C55A811_56E8_11D3_80CB_0090271354E9__INCLUDED_)
#define AFX_COMPRESSFILE_H__1C55A811_56E8_11D3_80CB_0090271354E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>

#include "fci.h"

class CCompressFile  
{

public:
	CString m_sBackupPath;
	BOOL DestroyFCI();
	void CreateFCI();
	CCompressFile();
	virtual ~CCompressFile();
	static CCompressFile* c_instance;
	static CCompressFile* CCompressFile::GetInstance();
	static void DestroyInstance();
	
	void    CompressMain();
	void    store_cab_name(char *cabname,int iCab);
	void    set_cab_parameters(PCCAB cab_parms);
	BOOL	AddFileFCI(CString &);
	void    strip_path(char *filename, char *stripped_name);
	int		get_percentage(unsigned long a, unsigned long b);
	char    *return_fci_error_string(FCIERROR err);
	static HFCI	m_hfci;
	ERF		m_erf;
	CCAB	m_cab_parameters;
	client_state	m_cs;
	CString m_sCabName;
	CStringList m_slCabList;
	CSMADErrorLog* m_Log;
};

#endif // !defined(AFX_COMPRESSFILE_H__1C55A811_56E8_11D3_80CB_0090271354E9__INCLUDED_)
