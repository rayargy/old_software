// EventInsertSet.cpp : implementation of the CEventInsertSet class
//

#include "stdafx.h"
#include "EventInsertSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventInsertSet implementation

IMPLEMENT_DYNAMIC(CEventInsertSet, CDaoRecordset)

CEventInsertSet::CEventInsertSet(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CEventInsertSet)
	m_Category = _T("");
	m_Computer = _T("");
	m_Description = _T("");
//	m_Data = _T("");
	m_HexData = _T("");
	m_ASCIIData = _T("");
	m_EventID = 0;
	m_Source = _T("");
	m_Type = _T("");
	m_User = _T("");
	m_nFields = 11;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenTable;
}

CString CEventInsertSet::GetDefaultSQL()
{
	return m_sDefaultSQL;
}

void CEventInsertSet::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CEventInsertSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Text(pFX, _T("[Category]"), m_Category);
	DFX_Text(pFX, _T("[Computer]"), m_Computer);
	DFX_Text(pFX, _T("[Description]"), m_Description);
//	DFX_Text(pFX, _T("[Data]"), m_Data);
	DFX_Text(pFX, _T("[HexData]"), m_HexData);
	DFX_Text(pFX, _T("[ASCIIData]"), m_ASCIIData);
	DFX_DateTime(pFX, _T("[EventDate]"), m_EventDate);
	DFX_Long(pFX, _T("[EventID]"), m_EventID);
	DFX_DateTime(pFX, _T("[EventTime]"), m_EventTime);
	DFX_Text(pFX, _T("[Source]"), m_Source);
	DFX_Text(pFX, _T("[Type]"), m_Type);
	DFX_Text(pFX, _T("[User]"), m_User);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CEventInsertSet diagnostics

#ifdef _DEBUG
void CEventInsertSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CEventInsertSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
