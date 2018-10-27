// VersionSet.cpp : implementation of the CVersionSet class
//

#include "stdafx.h"
#include "Version.h"
#include "VersionSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVersionSet implementation

IMPLEMENT_DYNAMIC(CVersionSet, CDaoRecordset)

CVersionSet::CVersionSet(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CVersionSet)
	m_FileSize = _T("");
	m_ProductName = _T("");
	m_ProductVersion = _T("");
	m_OriginalFilename = _T("");
	m_FileDescription = _T("");
	m_FileVersion = _T("");
	m_CompanyName = _T("");
	m_LegalCopyright = _T("");
	m_LegalTrademarks = _T("");
	m_InternalName = _T("");
	m_PrivateBuild = _T("");
	m_SpecialBuild = _T("");
	m_Comments = _T("");
	m_nFields = 14;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}

CString CVersionSet::GetDefaultSQL()
{
	return _T("[VersionInfo]");
}

void CVersionSet::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CVersionSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_DateTime(pFX,_T("[LastModDate]"),m_LastModDate);
	DFX_Text(pFX,_T("[FileSize]"),m_FileSize);
	DFX_Text(pFX, _T("[ProductName]"), m_ProductName);
	DFX_Text(pFX, _T("[ProductVersion]"), m_ProductVersion);
	DFX_Text(pFX, _T("[OriginalFilename]"), m_OriginalFilename);
	DFX_Text(pFX, _T("[FileDescription]"), m_FileDescription);
	DFX_Text(pFX, _T("[FileVersion]"), m_FileVersion);
	DFX_Text(pFX, _T("[CompanyName]"), m_CompanyName);
	DFX_Text(pFX, _T("[LegalCopyright]"), m_LegalCopyright);
	DFX_Text(pFX, _T("[LegalTrademarks]"), m_LegalTrademarks);
	DFX_Text(pFX, _T("[InternalName]"), m_InternalName);
	DFX_Text(pFX, _T("[PrivateBuild]"), m_PrivateBuild);
	DFX_Text(pFX, _T("[SpecialBuild]"), m_SpecialBuild);
	DFX_Text(pFX, _T("[Comments]"), m_Comments);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CVersionSet diagnostics

#ifdef _DEBUG
void CVersionSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CVersionSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
