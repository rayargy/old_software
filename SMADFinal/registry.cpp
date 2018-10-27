// Registry.cpp: implementation of the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxmt.h>			// MFC multi thread support
#include "Registry.h"

//MCCONFIG{{  Do not modify these lines - use MCCONFIG! 
   // 
   // This custom block declares one instance of the
   // current source filename to pass to MemCheck.  
   // Only one copy of the filename is created per module (efficient).
   //
   // MCCONFIG for Windows used the MFC block style (see main screen)
   // to insert this #include block after the last header or occurrence
   // of an IMPLEMENT_SERIAL or IMPLEMENT_DYNAMIC statement.
   //
   #ifndef _MCSF_
      #define _MCSF_ szMCFile
   #endif
   // The MEMCHECK constant is defined by default in the MemCheck
   // header file.  If you want to compile MemCheck out of your
   // code, simply define the symbol NOMC or NOMEMCHECK.
   // It's not necessary to remove the header file to compile
   // MemCheck out of your code.
   //
   #if defined(MEMCHECK)  // defined in memcheck.h if !defined(NOMC)
      static char szMCFile[] = __FILE__; 
   #endif
//}}MCCONFIG

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistry::CRegistry()
{
}

CRegistry::~CRegistry()
{
}

////////////////////////////////////////////////////////////////
// Close an open key in the Registry
long CRegistry::CloseRegKey(HKEY hKeyToClose)
{
	return ( RegCloseKey( hKeyToClose ) );				// Close the key
}

////////////////////////////////////////////////////////////////
// Connects to the registry of another computer
// hKey needs to contain one of the predefined registry keys.
// Currently, only HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER are
//	supported.
long CRegistry::ConnectToRegistry(LPTSTR lpMachineName, HKEY *phkResult, HKEY hKey)
{
	return( RegConnectRegistry( lpMachineName,			// address of name of remote computer 
								hKey,					// predefined registry handle - defaults to HKEY_LOCAL_MACHINE
								phkResult ) );			// address of buffer for remote registry handle 
}

////////////////////////////////////////////////////////////////
// Create a key in the Registry
long CRegistry::CreateRegKey(char *szKeyToCreate, HKEY hBaseKey)
{
	long	RetVal;
	HKEY	hKeyHandle;

	RetVal = RegCreateKeyEx( hBaseKey,					// Defaults to HKEY_LOCAL_MACHINE
							 szKeyToCreate,				// This is the key we want to create
							 0,							// RESERVED - must be 0
							 NULL,						// No class info.
							 REG_OPTION_NON_VOLATILE,	// We want this permanent
							 KEY_ALL_ACCESS,			// No restrictions
							 NULL,						// No security descriptor
							 &hKeyHandle,				// Resultant handle
							 &m_dwDisposition);			// Disposition of call
	// If successful, we have an open key
	if ( RetVal == ERROR_SUCCESS || RetVal == REG_CREATED_NEW_KEY || RetVal == REG_OPENED_EXISTING_KEY )
		CloseRegKey( hKeyHandle );
	// Return results
	return ( RetVal );
}

////////////////////////////////////////////////////////////////
// Deletes a key in the Registry
long CRegistry::DeleteRegKey(char *szKeyToDelete, HKEY hBaseKey)
{
	return ( RegDeleteKey ( hBaseKey,					// Defaults to HKEY_LOCAL_MACHINE
					(const char *) szKeyToDelete) );	// Key to delete
}

////////////////////////////////////////////////////////////////
// Deletes a value name in the Registry
long CRegistry::DeleteRegValue(HKEY hKey, LPTSTR lpValueName)
{
	return ( RegDeleteValue( hKey,						// handle of key 
						(const char *) lpValueName ) );	// address of value name 
}

////////////////////////////////////////////////////////////////
// Enumerates a key's subkeys in the Registry
// Increment dwIndex to proceed through subkeys - Starts at 0
long CRegistry::EnumRegKey(HKEY hKey, LPTSTR lpName, LPDWORD lpcbName,
						   PFILETIME lpftLastWrite, DWORD dwIndex)
{
	return ( RegEnumKeyEx( hKey,						// handle of key to enumerate 
						   dwIndex,						// index of subkey to enumerate 
						   lpName,						// address of buffer for subkey name 
						   lpcbName,					// address for size of subkey buffer 
						   NULL,						// reserved 
						   NULL,						// address of buffer for class string 
						   NULL,						// address for size of class buffer 
						   lpftLastWrite ) );			// address for time key last written to 
}

////////////////////////////////////////////////////////////////
// Enumerates a key's values in the Registry
// Increment dwIndex to proceed through values - Starts at 0
long CRegistry::EnumRegValue(HKEY hKey, LPTSTR lpName, LPDWORD lpcbName,
							 LPDWORD lpType, LPBYTE lpValue, LPDWORD lpcbValue,
							 DWORD dwIndex)
{
	return ( RegEnumValue( hKey,						// handle of key to query 
						   dwIndex,						// index of value to query 
						   lpName,						// address of buffer for value string 
						   lpcbName,					// address for size of value buffer 
						   NULL,						// reserved 
						   lpType,						// address of buffer for type code 
						   lpValue,						// address of buffer for value data 
						   lpcbValue ) );				// address for size of data buffer 
}

////////////////////////////////////////////////////////////////
// Open a key in the Registry
HKEY CRegistry::OpenRegKey(long *Results, char *KeyToOpen, HKEY hOpenKey)
{
	long	RetVal;
	HKEY	hKeyHandle;

	// We use RegCreateKeyEx instead of RegOpenKeyEx because you
	// have to open only one layer at a time with RegOpenKeyEx and
	// you can open a fully qualified key via RegCreateKeyEx.
	RetVal = RegCreateKeyEx( hOpenKey,					// Defaults to HKEY_LOCAL_MACHINE
							 KeyToOpen,					// This is the key we want to create
							 0,							// RESERVED - must be 0
							 NULL,						// No class info.
							 REG_OPTION_NON_VOLATILE,	// We want this permanent
							 KEY_ALL_ACCESS,			// No restrictions
							 NULL,						// No security descriptor
							 &hKeyHandle,				// Resultant handle
							 &m_dwDisposition);			// Disposition of call
	*Results = RetVal;
	return ( hKeyHandle );
}

////////////////////////////////////////////////////////////////
// Queries a key in the Registry to find out info about sub-keys
//	and values associated with that key.
long CRegistry::QueryRegKey(HKEY hKey, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen,
							LPDWORD lpcValues, LPDWORD lpcbMaxValueNameLen,
							LPDWORD lpcbMaxValueLen)
{
	return ( RegQueryInfoKey (	hKey,					// handle of key to query 
								NULL,					// address of buffer for class string 
								NULL,					// address of size of class string buffer 
								NULL,					// reserved 
								lpcSubKeys,				// address of buffer for number of subkeys 
								lpcbMaxSubKeyLen,		// address of buffer for longest subkey name length  
								NULL,					// address of buffer for longest class string length 
								lpcValues,				// address of buffer for number of value entries 
								lpcbMaxValueNameLen,	// address of buffer for longest value name length 
								lpcbMaxValueLen,		// address of buffer for longest value data length 
								NULL,					// address of buffer for security descriptor length 
								NULL ) );				// address of buffer for last write time 
}

////////////////////////////////////////////////////////////////
// Retrieves a specific value from a key in the Registry
long CRegistry::QueryRegValue(HKEY hKey, LPTSTR lpName, LPDWORD lpType,
							  LPBYTE lpData, LPDWORD lpcbData)
{
	return ( RegQueryValueEx( hKey,						// handle of key to query 
							  lpName,					// address of name of value to query 
							  NULL,						// reserved 
							  lpType,					// address of buffer for value type 
							  lpData,					// address of data buffer 
							  lpcbData ) );				// address of data buffer size 
}

////////////////////////////////////////////////////////////////
// Sets a specific value form an item under a key in the Registry
long CRegistry::SetRegValue(HKEY hKey, LPCTSTR lpValueName, DWORD dwType, CONST BYTE * lpData, DWORD cbData)
{
	return ( RegSetValueEx( hKey,						// handle of key to set value for  
							lpValueName,				// address of value to set 
							NULL,						// reserved 
							dwType,						// flag for value type 
							lpData,						// address of value data 
							cbData ) );					// size of value data 
}

DWORD CRegistry::FetchDWValue(long * RetValue, HKEY OpenKey, char * ItemToRead)
{
	DWORD lpType, lpcbData, lRetVal = 0;
	unsigned char lpData[128];

	lpcbData = 128;
	*RetValue = QueryRegValue( OpenKey, ItemToRead, &lpType, lpData, &lpcbData );
	if ( *RetValue == ERROR_SUCCESS )
		memcpy( &lRetVal, &lpData, sizeof ( lRetVal ) );
	return lRetVal;
}

CString CRegistry::FetchStrValue(long * RetValue, HKEY OpenKey, char * ItemToRead)
{
	CString sRetVal = "";
	DWORD lpType, lpcbData;
	unsigned char lpData[512];

	lpcbData = 512;
	*RetValue = QueryRegValue( OpenKey, ItemToRead, &lpType, lpData, &lpcbData );
	if ( *RetValue == ERROR_SUCCESS )
	{
		lpData[lpcbData] = 0;
		sRetVal = (char *) lpData;
	}
	return sRetVal;
}

BOOL CRegistry::FetchBoolValue(long * RetValue, HKEY OpenKey, char * ItemToRead)
{
	DWORD lpType, lpcbData;
	unsigned char lpData[128];

	lpcbData = 128;
	*RetValue = QueryRegValue( OpenKey, ItemToRead, &lpType, lpData, &lpcbData );
	if ( *RetValue == ERROR_SUCCESS )
		for (unsigned int iCount = 0; iCount < lpcbData; iCount++)
			if ( lpData[iCount] != 0 )
				return TRUE;
	return FALSE;
}
