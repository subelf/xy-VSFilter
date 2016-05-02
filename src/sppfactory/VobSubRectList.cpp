#include "stdafx.h"

#include "VobSubRectList.h"

#include "..\dsutil\DSUtil.h"


CVobSubRectList::CVobSubRectList() :
	CUnknown(_T("CVobSubRectList"), NULL),
	m_list()
{
}

CVobSubRectList::CVobSubRectList(CAtlList<CRect> const &rectList) : 
	CVobSubRectList()
{
	this->m_list.AddTailList(&rectList);
}

STDMETHODIMP CVobSubRectList::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(IVobSubRectList)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

POSITION CVobSubRectList::GetHeadPosition()
{
	return this->m_list.GetHeadPosition();
}

RECT CVobSubRectList::GetNext(POSITION &pos)
{
	return this->m_list.GetNext(pos);
}

void CVobSubRectList::RemoveAll()
{
	return this->m_list.RemoveAll();
}

POSITION CVobSubRectList::AddTail(RECT const &rect)
{
	return this->m_list.AddTail(rect);
}
