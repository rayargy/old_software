// CabFile.cpp: implementation of the CCompressFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressFile.h"
#include "Macros.h"
#include "registry.h"

CCompressFile *CCompressFile::c_instance = NULL;
HFCI CCompressFile::m_hfci = NULL;

#define REGKEY "SOFTWARE\\CTRSystems\\PARCS-CS\\SystemConfiguration\\SMAD"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressFile::CCompressFile()
{
	CRegistry Reg;
	HKEY hKey;
	DWORD dwValType = REG_EXPAND_SZ,dwValSize = 0;
	LPBYTE lpBuf;
	int iSPos = 0;
	long lRes = 0;

	m_Log->GetInstance();
	m_Log->WriteLogEntry("Constructor for CCompressFile",0);

	hKey = Reg.OpenRegKey(&lRes,REGKEY);
	if(lRes == ERROR_SUCCESS)
	{
		CString sTemp,sFile;
		m_sBackupPath = Reg.FetchStrValue(&lRes,hKey,"BackupPath");

		lRes = Reg.QueryRegValue(hKey,"MoveFileList",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"MoveFileList",&dwValType,lpBuf,&dwValSize);
		sTemp = lpBuf;
		LocalFree(lpBuf);		

		for(int iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			sFile = sTemp.Left(iPos);
			iSPos = sFile.ReverseFind('\\');
			sFile = m_sBackupPath + sFile.Mid(iSPos + 1);
			m_slCabList.AddHead(sFile);
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
		{
			sFile = sTemp;
			iSPos = sFile.ReverseFind('\\');
			sFile = m_sBackupPath + sFile.Mid(iSPos + 1);
			m_slCabList.AddHead(sFile);
		}

		lRes = Reg.QueryRegValue(hKey,"CopyFileList",&dwValType,NULL,&dwValSize);
		lpBuf = (LPBYTE)LocalAlloc(LMEM_ZEROINIT,dwValSize + 1);
		lRes = Reg.QueryRegValue(hKey,"CopyFileList",&dwValType,lpBuf,&dwValSize);
		sTemp = lpBuf;
		LocalFree(lpBuf);

		for(iPos = sTemp.Find(';'); iPos != -1;iPos = sTemp.Find(';'))
		{
			sFile = sTemp.Left(iPos);
			iSPos = sFile.ReverseFind('\\');
			sFile = m_sBackupPath + sFile.Mid(iSPos + 1);
			m_slCabList.AddHead(sFile);
			sTemp = sTemp.Mid(iPos + 1);
		}
		if(sTemp != "")
		{
			sFile = sTemp;
			iSPos = sFile.ReverseFind('\\');
			sFile = m_sBackupPath + sFile.Mid(iSPos + 1);
			m_slCabList.AddHead(sFile);
		}

		sTemp = Reg.FetchStrValue(&lRes,hKey,"EventMdb");
		iPos = sTemp.ReverseFind('\\');
		sTemp = m_sBackupPath + sTemp.Mid(iPos + 1);
		m_slCabList.AddHead(sTemp);

		sTemp = Reg.FetchStrValue(&lRes,hKey,"VersionMdb");
		iPos = sTemp.ReverseFind('\\');
		sTemp = m_sBackupPath + sTemp.Mid(iPos + 1);
		m_slCabList.AddHead(sTemp);

		sTemp = Reg.FetchStrValue(&lRes,hKey,"RegFile");
		iPos = sTemp.ReverseFind('\\');
		sTemp = m_sBackupPath + sTemp.Mid(iPos + 1);
		m_slCabList.AddHead(sTemp);

		m_sCabName = Reg.FetchStrValue(&lRes,hKey,"CtrCabFile");
		Reg.CloseRegKey(hKey);
	}
	CreateFCI();
}

CCompressFile::~CCompressFile()
{
	DestroyFCI();
}

CCompressFile* CCompressFile::GetInstance()
{
	if(c_instance == NULL)
		c_instance = new CCompressFile;
	return c_instance;
}

void CCompressFile::DestroyInstance()
{
	CString strMsg;

	if(c_instance != NULL)
	{
		delete c_instance;
		c_instance = NULL;
	}
}

void CCompressFile::CompressMain()
{
	CString strMsg;

	if(m_hfci)
	{
		POSITION iPos = m_slCabList.GetHeadPosition();
		while(iPos != NULL)
		{
			CString sTempFile = m_slCabList.GetNext(iPos);
			strMsg.Format("\tCCompressFile::CompressMain() Adding %s to archive!",sTempFile);
			m_Log->WriteLogEntry(strMsg,0);
			if(!AddFileFCI(sTempFile))
			{
				// when AddFileFCI fails(false), the error is returned in sTempFile
				strMsg.Format("\tCCompressFile::CompressMain() AddFileFCI failed %s",
					sTempFile);
				strMsg.TrimRight();
				m_Log->WriteLogEntry(strMsg,0);
			}
			else
				DeleteFile(sTempFile);
		}
	}
	else
	{
		strMsg = "\tCCompressFile::CompressMain() CreateFCI returned an invalid handle";
		m_Log->WriteLogEntry(strMsg,0);
	}
}

void CCompressFile::store_cab_name(char *cabname,int iCab)
{
	sprintf(cabname, "%s");
}

void CCompressFile::set_cab_parameters(PCCAB cab_parms)
{
	memset(cab_parms, 0, sizeof(CCAB));


	cab_parms->cb = MEDIA_SIZE;
	cab_parms->cbFolderThresh = FOLDER_THRESHOLD;

	/*
	 * Don't reserve space for any extensions
	 */
	cab_parms->cbReserveCFHeader = 0;
	cab_parms->cbReserveCFFolder = 0;
	cab_parms->cbReserveCFData   = 0;

	/*
	 * We use this to create the cabinet name
	 */
	cab_parms->iCab = 1;

	/*
	 * If you want to use disk names, use this to
	 * count disks
	 */
	cab_parms->iDisk = 0;

	/*
	 * Choose your own number
	 */
	cab_parms->setID = 0;

	/*
	 * Only important if CABs are spanning multiple
	 * disks, in which case you will want to use a
	 * real disk name.
	 *
	 * Can be left as an empty string.
	 */
	strcpy(cab_parms->szDisk, "");

	int iPos = m_sCabName.ReverseFind('\\');
	/* where to store the created CAB files */
	strcpy(cab_parms->szCabPath,m_sCabName.Left(iPos + 1));

	/* store name of first CAB file */
//	store_cab_name(cab_parms->szCab, cab_parms->iCab);
	sprintf(cab_parms->szCab,"%s",m_sCabName.Mid(iPos + 1));
}

BOOL CCompressFile::AddFileFCI(CString & file)
{
	CString strErr;
	int				i = 0;
	char	stripped_name[256];

	strip_path(file.GetBuffer(file.GetLength()), stripped_name);

	if (FALSE == FCIAddFile(
		m_hfci,
		file.GetBuffer(file.GetLength()),	// file to add
		stripped_name,						// file name in cabinet file
		FALSE,								// file is not executable
		get_next_cabinet,
		progress,
		get_open_info,
        COMPRESSION_TYPE))
	{
		file.Format("FCICreate() failed: code %d [%s]\n",
			m_erf.erfOper, return_fci_error_string((FCIERROR)m_erf.erfOper));
		return FALSE;
	}

	return TRUE;
}

void CCompressFile::strip_path(char *filename, char *stripped_name)
{
	char	*p;

	p = strrchr(filename, ':');

	if (p == NULL)
		strcpy(stripped_name, filename);
	else
		strcpy(stripped_name, p+1);
}

int CCompressFile::get_percentage(unsigned long a, unsigned long b)
{
	while (a > 10000000)
	{
		a >>= 3;
		b >>= 3;
	}

	if (b == 0)
		return 0;

	return ((a*100)/b);
}

char *CCompressFile::return_fci_error_string(FCIERROR err)
{
	switch (err)
	{
		case FCIERR_NONE:
			return "No error";

		case FCIERR_OPEN_SRC:
			return "Failure opening file to be stored in cabinet";
		
		case FCIERR_READ_SRC:
			return "Failure reading file to be stored in cabinet";
		
		case FCIERR_ALLOC_FAIL:
			return "Insufficient memory in FCI";

		case FCIERR_TEMP_FILE:
			return "Could not create a temporary file";

		case FCIERR_BAD_COMPR_TYPE:
			return "Unknown compression type";

		case FCIERR_CAB_FILE:
			return "Could not create cabinet file";

		case FCIERR_USER_ABORT:
			return "Client requested abort";

		case FCIERR_MCI_FAIL:
			return "Failure compressing data";

		default:
			return "Unknown error";
	}
}

void CCompressFile::CreateFCI()
{
	set_cab_parameters(&m_cab_parameters);

	/*  
	 * Initialise our internal state
	 */
    m_cs.total_compressed_size = 0;
	m_cs.total_uncompressed_size = 0;

	m_hfci = FCICreate(
		&m_erf,
		file_placed,
		mem_alloc,
		mem_free,
        fci_open,
        fci_read,
        fci_write,
        fci_close,
        fci_seek,
        fci_delete,
		get_temp_file,
        &m_cab_parameters,
        &m_cs
	);
}

BOOL CCompressFile::DestroyFCI()
{
	// This error exists, but we can't do anything with because
	// the error log will be close when this destructor is called.
	CString sErr;
	/*
	 * This will automatically flush the folder first
	 */
	if (FALSE == FCIFlushCabinet(
		m_hfci,
		FALSE,
		get_next_cabinet,
		progress))
	{
		sErr.Format("FCIFlushCabinet() failed: code %d [%s]\n",
			m_erf.erfOper, return_fci_error_string((FCIERROR)m_erf.erfOper));

		return FALSE;
	}

	return(FCIDestroy(m_hfci));
}
