// sppfactory.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "..\dsutil\DSUtil.h"

#include "..\subtitles\VobSubFile.h"
#include "..\subtitles\SSF.h"

#include "VSSppf.h"
#include "VobSubPicProviderContext.h"


CFactoryTemplate g_Templates[] =
{
	{L"CVobSubPicProviderFactory", &__uuidof(CVobSubPicProviderFactory), CreateInstance<CVobSubPicProviderFactory>},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}


CVobSubPicProviderFactory::CVobSubPicProviderFactory(LPUNKNOWN punk, HRESULT * phr)
	:CUnknown(_T("CVobSubPicProviderFactory"), punk)
{
}

STDMETHODIMP CVobSubPicProviderFactory::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(IVobSubPicProviderFactory)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CVobSubPicProviderFactory::CreateContext(IVobSubPicProviderContext **ppContext) const
{
	if (ppContext == nullptr)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	CComPtr<IVobSubPicProviderContext> context(new CVobSubPicProviderContext());
	if (context == nullptr)
	{
		hr = E_FAIL;
	}

	(*ppContext = context)->AddRef();
	return hr;
}

STDMETHODIMP CVobSubPicProviderFactory::CreateProvider(IVobSubPicProviderContext *pContext, WCHAR const *pStrSubtitlePath, ISubPicProvider **ppProvider) const
{
	if (ppProvider == nullptr)
	{
		return E_FAIL;
	}

	auto *pContextObj = dynamic_cast<CVobSubPicProviderContext*>(pContext);
	if (pContextObj == nullptr)
	{
		return E_FAIL;
	}

	auto * const &pLock = pContextObj->GetProviderLock();
	HRESULT hr = S_OK;
	CComPtr<ISubPicProvider> pSubProvider;

	if (!pSubProvider)
	{
		CAutoPtr<CRenderedTextSubtitle> pRTS(new CRenderedTextSubtitle(pLock));
		if (pRTS && pRTS->Open(pStrSubtitlePath, DEFAULT_CHARSET) && pRTS->GetStreamCount() > 0)
		{
			pSubProvider = pRTS.Detach();
		}
	}

	if (!pSubProvider)
	{
		CAutoPtr<CVobSubFile> pVSF(new CVobSubFile(pLock));
		if (pVSF && pVSF->Open(pStrSubtitlePath) && pVSF->GetStreamCount() > 0)
		{
			pSubProvider = pVSF.Detach();
		}
	}

	if (!pSubProvider)
	{
		CAutoPtr<ssf::CRenderer> pSSF(new ssf::CRenderer(pLock));
		if (pSSF && pSSF->Open(pStrSubtitlePath) && pSSF->GetStreamCount() > 0)
		{
			pSubProvider = pSSF.Detach();
		}
	}

	if (!pSubProvider)
	{
		hr = E_FAIL;
	}

	(*ppProvider = pSubProvider)->AddRef();

	return hr;
}

