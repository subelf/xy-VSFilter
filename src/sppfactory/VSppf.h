#pragma once


#include <streams.h>
#include <combase.h>

#include "VSppfApi.h"

// {5C45E508-8D8E-4FB8-9933-CFA33217401C}
DEFINE_GUID(CLSID_VobSubPicProviderFactory,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x1c);

[uuid("5C45E508-8D8E-4FB8-9933-CFA33217401C")]
class CVobSubPicProviderFactory final :
	public CUnknown, 
	public IVobSubPicProviderFactory
{

public:
	CVobSubPicProviderFactory(LPUNKNOWN punk, HRESULT* phr);
	
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP CreateContext(IVobSubPicProviderContext **ppContext) const;
	STDMETHODIMP CreateProvider(IVobSubPicProviderContext *pContext, WCHAR *pStrSubtitlePath, ISubPicProvider **ppProvider) const;

};