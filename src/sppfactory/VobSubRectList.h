#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include <combase.h>

#include "VSSppfApi.h"

// {5C45E508-8D8E-4FB8-9933-CFA33217401A}
DEFINE_GUID(CLSID_VobSubRectList,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x1a);

[uuid("5C45E508-8D8E-4FB8-9933-CFA33217401A")]
class CVobSubRectList :
	public CUnknown,
	public IVobSubRectList
{
public:
	CVobSubRectList();
	CVobSubRectList(CAtlList<CRect> const &rectList);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// Inherited via IVobSubRectList
	STDMETHODIMP_(POSITION) GetHeadPosition();
	STDMETHODIMP_(RECT) GetNext(POSITION & pos);
	STDMETHODIMP_(void) RemoveAll();
	STDMETHODIMP_(POSITION) AddTail(RECT const & rect);

private:
	CAtlList<CRect> m_list;
};

