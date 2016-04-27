
#include "stdafx.h"

#include "..\dsutil\DSUtil.h"

#include "VobSubPicProviderContext.h"

CVobSubPicProviderContext::CVobSubPicProviderContext() :
	CUnknown(_T("CVobSubPicProviderContext"), NULL),
	m_ProviderLock()
{
}

STDMETHODIMP CVobSubPicProviderContext::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(IVobSubPicProviderContext)
		__super::NonDelegatingQueryInterface(riid, ppv);
}


void CVobSubPicProviderContext::Lock()
{
	this->m_ProviderLock.Lock();
}

void CVobSubPicProviderContext::Unlock()
{
	this->m_ProviderLock.Unlock();
}
