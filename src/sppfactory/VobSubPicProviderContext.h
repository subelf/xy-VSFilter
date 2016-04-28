#pragma once

#include "VSppfApi.h"

// {5C45E508-8D8E-4FB8-9933-CFA33217401B}
DEFINE_GUID(CLSID_VobSubPicProviderContext,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x1b);


[uuid("5C45E508-8D8E-4FB8-9933-CFA33217401B")]
class CVobSubPicProviderContext final :
	public CUnknown,
	public IVobSubPicProviderContext
{
public:
	CVobSubPicProviderContext();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP_(void) Lock();
	STDMETHODIMP_(void) Unlock();

	CCritSec *GetProviderLock() { return &this->m_ProviderLock; }

private:
	CCritSec m_ProviderLock;

};

