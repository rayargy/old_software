// Registry.h: interface for the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRY_H__062AA593_0756_11D1_A5CB_00AA00BBBEE3__INCLUDED_)
#define AFX_REGISTRY_H__062AA593_0756_11D1_A5CB_00AA00BBBEE3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CRegistry  
{
public:
	BOOL FetchBoolValue(long * RetValue, HKEY OpenKey, char * ItemToRead);
	CString FetchStrValue( long * RetValue, HKEY OpenKey, char * ItemToRead );
	DWORD FetchDWValue( long *RetValue, HKEY OpenKey, char *ItemToRead );
	CRegistry();
	virtual ~CRegistry();
	long CloseRegKey(HKEY hKeyToClose);
	long ConnectToRegistry(LPTSTR lpMachineName, HKEY *phkResult, 
		HKEY hKey = HKEY_LOCAL_MACHINE);
	long CreateRegKey(char *szKeyToCreate, 
		HKEY hBaseKey = HKEY_LOCAL_MACHINE);
	long DeleteRegKey(char *szKeyToDelete, 
		HKEY hBaseKey = HKEY_LOCAL_MACHINE);
	long DeleteRegValue(HKEY hKey, LPTSTR lpValueName);
	long EnumRegKey(HKEY hKey, LPTSTR lpName, LPDWORD lpcbName, 
		PFILETIME lpftLastWrite, DWORD dwIndex = 0);
	long EnumRegValue(HKEY hKey, LPTSTR lpName, LPDWORD lpcbName, 
		LPDWORD lpType, LPBYTE lpValue, LPDWORD lpcbValue, 
		DWORD dwIndex = 0);
	HKEY OpenRegKey(long *Results, char *KeyToOpen, 
		HKEY hOpenKey = HKEY_LOCAL_MACHINE);
	long QueryRegKey(HKEY hKey, LPDWORD lpcSubKeys, 
		LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcValues, 
		LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen);
	long QueryRegValue(HKEY hKey, LPTSTR lpName, LPDWORD lpType, 
		LPBYTE lpData, LPDWORD lpcbData);
	long SetRegValue(HKEY hKey, LPCTSTR lpValueName, DWORD dwType, 
		CONST BYTE *lpData, DWORD cbData);

	DWORD m_dwDisposition;

private:
};

#endif // !defined(AFX_REGISTRY_H__062AA593_0756_11D1_A5CB_00AA00BBBEE3__INCLUDED_)
