#pragma once

#include <combase.h>

#include "VSSppfApi.h"

[uuid("5C45E508-8D8E-4FB8-9933-CFA33217401C")]
class CVobSubPicProviderAlfaFactory final :
	public CUnknown, 
	public IVobSubPicProviderAlfaFactory
{

public:
	CVobSubPicProviderAlfaFactory(LPUNKNOWN punk, HRESULT* phr);
	
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP CreateContext(IVobSubPicProviderContext **ppContext) const;
	STDMETHODIMP CreateProvider(IVobSubPicProviderContext *pContext, WCHAR const *pStrSubtitlePath, ISubPicProviderAlfa **ppProvider) const;

};