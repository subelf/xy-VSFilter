// sppfactory.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "..\dsutil\DSUtil.h"

#include "..\subtitles\VobSubFile.h"
#include "..\subtitles\SSF.h"
#include "..\subtitles\RTS.h"

#include "VSSppf.h"
#include "SubPicProviderAlfa.h"
#include "VobSubPicProviderContext.h"


CFactoryTemplate g_Templates[] =
{
	{L"CVobSubPicProviderAlfaFactory", &__uuidof(CVobSubPicProviderAlfaFactory), CreateInstance<CVobSubPicProviderAlfaFactory>},
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


CVobSubPicProviderAlfaFactory::CVobSubPicProviderAlfaFactory(LPUNKNOWN punk, HRESULT * phr)
	:CUnknown(_T("CVobSubPicProviderEx2Factory"), punk)
{
	CRenderedTextSubtitle::GlobalStaticInit();
}

STDMETHODIMP CVobSubPicProviderAlfaFactory::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(IVobSubPicProviderAlfaFactory)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CVobSubPicProviderAlfaFactory::SetMaxCacheSize(size_t max_size) const
{
	CRenderedTextSubtitle::SetMaxCacheSize(max_size);

	return S_OK;
}

STDMETHODIMP CVobSubPicProviderAlfaFactory::CreateContext(IVobSubPicProviderContext **ppContext) const
{
	if (ppContext == nullptr)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	CComPtr<IVobSubPicProviderContext> apContext(new CVobSubPicProviderContext());
	
	if (apContext == nullptr)
	{
		hr = E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		(*ppContext = apContext)->AddRef();
	}
	return hr;
}

static ISubPicProviderAlfa * CreateSubPicProviderAlfa(ISubPicProviderEx *pSppx)
{
	if (pSppx == nullptr) return nullptr;

	CComQIPtr<ISubPicProviderEx2> ifpSppx2 = pSppx;

	if (ifpSppx2)
	{
		return new CSubPicProviderAlfaX2(pSppx, ifpSppx2);
	}
	else
	{
		return new CSubPicProviderAlfaX(pSppx);
	}
}

STDMETHODIMP CVobSubPicProviderAlfaFactory::CreateProvider(IVobSubPicProviderContext *pContext, WCHAR const *pStrSubtitlePath, ISubPicProviderAlfa **ppProvider) const
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
	CComPtr<ISubPicProviderEx> pSubProvider;

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

	CComPtr<ISubPicProviderAlfa> pSppAlfa(CreateSubPicProviderAlfa(pSubProvider));

	if (!pSppAlfa)
	{
		hr = E_FAIL;
	}

	if(SUCCEEDED(hr))
	{
		(*ppProvider = pSppAlfa)->AddRef();
	}

	return hr;
}

