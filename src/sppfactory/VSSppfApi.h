#pragma once

#include <initguid.h>

// {5C45E508-8D8E-4FB8-9933-CFA33217401C}
DEFINE_GUID(CLSID_VobSubPicProviderAlfaFactory,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x1c);

// {5C45E508-8D8E-4FB8-9933-CFA332174010}
DEFINE_GUID(IID_VobSubPicProviderContext,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x10);

// {5C45E508-8D8E-4FB8-9933-CFA332174011}
DEFINE_GUID(IID_VobSubRectList,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x11);

// {5C45E508-8D8E-4FB8-9933-CFA332174012}
DEFINE_GUID(IID_VobSubPicProviderAlfaFactory,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x12);

// {5C45E508-8D8E-4FB8-9933-CFA332174014}
DEFINE_GUID(IID_SubPicProviderAlfa,
	0x5c45e508, 0x8d8e, 0x4fb8, 0x99, 0x33, 0xcf, 0xa3, 0x32, 0x17, 0x40, 0x14);


#pragma pack(push, 1)
struct SubPicAlfaDesc {
	int w, h, pitch;
	void* bits;
	RECT vidrect;
};
#pragma pack(pop)

interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174010"))
	IVobSubPicProviderContext :
	public IUnknown
{
	STDMETHOD_(void, Lock)() PURE;
	STDMETHOD_(void, Unlock)() PURE;
};

interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174011"))
	IVobSubRectList :
	public IUnknown
{
	STDMETHOD_(POSITION, GetHeadPosition) () PURE;
	STDMETHOD_(RECT, GetNext) (POSITION &pos) PURE;

	STDMETHOD_(void, RemoveAll)() PURE;
	STDMETHOD_(POSITION, AddTail)(RECT const &rect) PURE;
};

interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174014"))
	ISubPicProviderAlfa :
	public IUnknown
{
	STDMETHOD (Lock) () PURE;
	STDMETHOD (Unlock) () PURE;

	STDMETHOD_(POSITION, GetStartPosition) (REFERENCE_TIME rt, double fps) PURE;
	STDMETHOD_(POSITION, GetNext) (POSITION pos) PURE;

	STDMETHOD_(REFERENCE_TIME, GetStart) (POSITION pos, double fps) PURE;
	STDMETHOD_(REFERENCE_TIME, GetStop) (POSITION pos, double fps) PURE;

	STDMETHOD_(bool, IsAnimated) (POSITION pos) PURE;

	STDMETHOD (CreateRectList) (IVobSubRectList **ppRectList) PURE;

	STDMETHOD (RenderEx) (SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, IVobSubRectList *pRectList) PURE;
	STDMETHOD (RenderAlpha) (SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, IVobSubRectList *pRectList) PURE;
	STDMETHOD (RenderAlpha) (SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, RECT& bbox) PURE;

    STDMETHOD (GetTextureSize) (POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft) PURE;
};

interface __declspec(uuid("5C45E508-8D8E-4FB8-9933-CFA332174012"))
	IVobSubPicProviderAlfaFactory :
	public IUnknown
{
	STDMETHOD (CreateContext) (IVobSubPicProviderContext **ppContext) const PURE;
	STDMETHOD (CreateProvider) (IVobSubPicProviderContext *pContext, WCHAR const *pStrSubtitlePath, ISubPicProviderAlfa **ppProvider) const PURE;
};


