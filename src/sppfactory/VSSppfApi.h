#pragma once

#include <initguid.h>

#include "..\subpic\ISubPic.h"

// {5C45E508-8D8E-4FB8-9933-CFA33217401C}
DEFINE_GUID(CLSID_VobSubPicProviderFactory,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x1c);

// {5C45E508-8D8E-4FB8-9933-CFA332174010}
DEFINE_GUID(IID_VobSubPicProviderContext,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x10);

// {5C45E508-8D8E-4FB8-9933-CFA332174011}
DEFINE_GUID(IID_VobSubPicProviderFactory,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x11);


interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174010"))
	IVobSubPicProviderContext :
	public IUnknown
{
	STDMETHOD_(void, Lock)() PURE;
	STDMETHOD_(void, Unlock)() PURE;
};

interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174011"))
	IVobSubPicProviderFactory :
	public IUnknown
{
	STDMETHOD (CreateContext) (IVobSubPicProviderContext **ppContext) const PURE;
	STDMETHOD (CreateProvider) (IVobSubPicProviderContext *pContext, WCHAR const *pStrSubtitlePath, ISubPicProvider **ppProvider) const PURE;
};


