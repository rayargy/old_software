// TransRegistry.cpp: implementation of the CTransRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "commserver.h"

#include "TransRegistry.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTransRegistry::CTransRegistry()
{
	long lResults;
	HKEY hTransKey;
	BOOL bStatus;

	m_Reg = new CRegistry;
	m_hKeyHandle = m_Reg->OpenRegKey(&lResults, "SOFTWARE\\CTRSystems\\PARCS-CS\\Transactions", 
		HKEY_LOCAL_MACHINE);

	m_dwNumTrans = m_Reg->FetchDWValue(&lResults, m_hKeyHandle, "NumTrans");

	// In order to prevent (or at least delay) the possibility of 
	// transactions getting overwritten before they are retrieved,
	// we are establishing a minimum value of "Trans" subkeys here.
	if ((lResults != ERROR_SUCCESS) || (m_dwNumTrans < MIN_NUMTRANS))
	{
		m_dwNumTrans = MIN_NUMTRANS;
		m_Reg->SetRegValue(m_hKeyHandle, "NumTrans", REG_DWORD, (CONST BYTE *) &m_dwNumTrans, sizeof(DWORD));
	}
	
	m_dwNextToRead = m_Reg->FetchDWValue(&lResults, m_hKeyHandle, "NextToRead");

	// If there was an error reading the index of NextToRead or if
	// it didn't exist, then we'll reset it to zero and, since it
	// was an invalid index in any case, set the status byte of
	// that transaction to STATUS_READ_TRANS.
	if ((lResults != ERROR_SUCCESS) || (m_dwNextToRead >= m_dwNumTrans))
	{
		m_dwNextToRead = 0;
		m_Reg->SetRegValue(m_hKeyHandle, "NextToRead", REG_DWORD, (CONST BYTE *) &m_dwNextToRead, sizeof(DWORD));
		hTransKey = OpenTransKey(m_dwNextToRead);
		bStatus = STATUS_READ_TRANS;
		m_Reg->SetRegValue(hTransKey, "Status", REG_BINARY, (CONST BYTE *) &bStatus, sizeof(BOOL));
		m_Reg->CloseRegKey(hTransKey);
	}
	
	m_dwNextToWrite = m_Reg->FetchDWValue(&lResults, m_hKeyHandle, "NextToWrite");

	// If there was an error reading this index, 
	// then we will look for the first TransKey 
	// after NextToRead whose status is STATUS_READ_TRANS
	// and use that slot to insert the next transaction.
	if ((lResults != ERROR_SUCCESS) || (m_dwNextToWrite >= m_dwNumTrans))
	{
		m_dwNextToWrite = m_dwNextToRead;

		while (GetStatusByte(m_dwNextToWrite) == STATUS_NEW_TRANS)
		{
			m_dwNextToWrite++;
			if (m_dwNextToWrite == m_dwNumTrans)
				m_dwNextToWrite = 0;

			// Check to see if we've looped back around.
			// If so, then we'll just have arbitrarily
			// pick 0 as the next index to start writing
			// since the queue is full.
			if (m_dwNextToWrite == m_dwNextToRead)
			{
				m_dwNextToWrite = 0;
				break;
			}

		}

		m_Reg->SetRegValue(m_hKeyHandle, "NextToWrite", REG_DWORD, (CONST BYTE *) &m_dwNextToWrite, sizeof(DWORD));
	}

}

CTransRegistry::~CTransRegistry()
{
	m_Reg->CloseRegKey(m_hKeyHandle);
}

// Inserts the transaction contained in cTransX into
// the key indexed by m_dwNextToWrite.
BOOL CTransRegistry::WriteTrans(CString cTransX)
{
	HKEY hTransKey;
	char szBuffer[512];
	BOOL bStatus;
	long lResults;

	strcpy(szBuffer, (LPCTSTR) cTransX);
	bStatus = STATUS_NEW_TRANS;

	hTransKey = OpenTransKey(m_dwNextToWrite);

	lResults = m_Reg->SetRegValue(hTransKey, "Trans", REG_SZ, (CONST BYTE *) szBuffer, strlen(szBuffer));
	if (lResults != ERROR_SUCCESS)
	{
		m_Reg->CloseRegKey(hTransKey);
		return FALSE;
	}

	lResults = m_Reg->SetRegValue(hTransKey, "Status", REG_BINARY, (CONST BYTE *) &bStatus, sizeof(BOOL));
	if (lResults != ERROR_SUCCESS)
	{
		m_Reg->CloseRegKey(hTransKey);
		return FALSE;
	}

	m_Reg->CloseRegKey(hTransKey);

	// Increment index to write in the next
	// available slot (wrap around if necessary)
	m_dwNextToWrite++;
	if (m_dwNextToWrite == m_dwNumTrans)
		m_dwNextToWrite = 0;

	return TRUE;
}

// Reads next transaction
BOOL CTransRegistry::ReadNextTrans(CString *cTransX, long *lResults)
{
	HKEY hTransKey;
	BOOL bStatus;

	bStatus = STATUS_READ_TRANS;

	hTransKey = OpenTransKey(m_dwNextToWrite);

	// Check to see if we're pointing to a transaction that has 
	// already been read (i.e., there are no new tranactions in
	// the queue, and therefore nothing to read).
	if (m_Reg->FetchBoolValue(lResults, hTransKey, "Status") == STATUS_READ_TRANS)
	{
		m_Reg->CloseRegKey(hTransKey);
		return FALSE;
	}


	*cTransX = m_Reg->FetchStrValue(lResults, hTransKey, "Trans");
	if (*lResults != ERROR_SUCCESS)
	{
		m_Reg->CloseRegKey(hTransKey);
		return FALSE;
	}

	*lResults = m_Reg->SetRegValue(hTransKey, "Status", REG_BINARY, (CONST BYTE *) &bStatus, sizeof(BOOL));
	if (*lResults != ERROR_SUCCESS)
	{
		m_Reg->CloseRegKey(hTransKey);
		return FALSE;
	}

	m_Reg->CloseRegKey(hTransKey);

	// Increment index to read the next
	// slot (wrap around if necessary)
	m_dwNextToRead++;
	if (m_dwNextToRead == m_dwNumTrans)
		m_dwNextToRead = 0;

	return TRUE;
}


// Simply returns STATUS_NEW_TRANS or STATUS_READ_TRANS for the
// transaction specified by the index dwIndex.
BOOL CTransRegistry::GetStatusByte(DWORD dwIndex)
{
	HKEY hTransKey;
	long lResults;
	BOOL bReturn;
	char szKeyName[20] = "Tran";

	hTransKey = OpenTransKey(dwIndex);

	bReturn = m_Reg->FetchBoolValue(&lResults, m_hKeyHandle, "Status");

	// If the value did not exist, then return
	// STATUS_READ_TRANS, because it can't be a 
	// new transaction if it doesn't exist and 
	// therefore can be overwritten.
	if (lResults != ERROR_SUCCESS)
		bReturn = STATUS_READ_TRANS;

	m_Reg->CloseRegKey(hTransKey);

	return bReturn;
}

// Opens the key specified by dwIndex and returns
// the handle.
HKEY CTransRegistry::OpenTransKey(DWORD dwIndex)
{
	HKEY hTransKey;
	long lResults;
	char szKeyName[20] = "Tran";
	char szBuffer[10];

	// Construct the name of the subkey 
	// we need to open by appending the
	// index to "Tran".
	_ultoa(dwIndex, szBuffer,10);
	strcat(szKeyName, szBuffer);

	hTransKey = m_Reg->OpenRegKey(&lResults, szKeyName, m_hKeyHandle);

	return hTransKey;
}
