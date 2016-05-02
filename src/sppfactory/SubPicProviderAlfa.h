#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include <combase.h>

#include "VSSppfApi.h"

[uuid("5C45E508-8D8E-4FB8-9933-CFA332174020")]
class CSubPicProviderAlfaImpl :
	public CUnknown,
	public ISubPicProviderAlfa
{
public:
	CSubPicProviderAlfaImpl(ISubPicProviderEx *pSppx);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP Lock () { return m_pSppx->Lock(); }
	STDMETHODIMP Unlock () { return m_pSppx->Unlock(); }

	STDMETHODIMP_(POSITION) GetStartPosition (REFERENCE_TIME rt, double fps) { 
		return m_pSppx->GetStartPosition(rt, fps); }
	STDMETHODIMP_(POSITION) GetNext (POSITION pos) {
		return m_pSppx->GetNext(pos);
	}

	STDMETHODIMP_(REFERENCE_TIME) GetStart (POSITION pos, double fps) {
		return m_pSppx->GetStart(pos, fps);
	}
	STDMETHODIMP_(REFERENCE_TIME) GetStop (POSITION pos, double fps) {
		return m_pSppx->GetStop(pos, fps);
	}

	STDMETHODIMP_(bool) IsAnimated (POSITION pos) {
		return m_pSppx->IsAnimated(pos);
	}

	STDMETHODIMP RenderAlpha(SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, CAtlList<CRect>& rectList) PURE;
	STDMETHODIMP RenderAlpha(SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, RECT& bbox);

	STDMETHODIMP GetTextureSize (POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft) {
		return m_pSppx->GetTextureSize(pos, MaxTextureSize, VirtualSize, VirtualTopLeft);
	}

public:
	static inline SubPicDesc GetSubPicDescFrom(SubPicAlfaDesc const& spad)
	{
		SubPicDesc spd;
		spd.type = MSP_RGBA;

		spd.w = spad.w; spd.h = spad.h; spd.bpp = 32;
		spd.pitch = spad.pitch;

		spd.bits = spad.bits;
		spd.vidrect = spad.vidrect;

		return spd;
	}

protected:
	CComPtr<ISubPicProviderEx> m_pSppx;
};

[uuid("5C45E508-8D8E-4FB8-9933-CFA332174021")]
class CSubPicProviderAlfaX2 final :
	public CSubPicProviderAlfaImpl
{
public:
	CSubPicProviderAlfaX2(ISubPicProviderEx *pSppx, ISubPicProviderEx2 *pSppx2);

	STDMETHODIMP RenderAlpha(SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, CAtlList<CRect>& rectList);

private:
	CComPtr<ISubPicProviderEx2> m_pSppx2;

};

[uuid("5C45E508-8D8E-4FB8-9933-CFA332174023")]
class CSubPicProviderAlfaX final :
	public CSubPicProviderAlfaImpl
{
public:
	CSubPicProviderAlfaX(ISubPicProviderEx *pSppx);
	~CSubPicProviderAlfaX();

	STDMETHODIMP RenderAlpha(SubPicAlfaDesc& spad, REFERENCE_TIME rt, double fps, CAtlList<CRect>& rectList);

private:
	SubPicDesc m_lastSpd;

private:
	void ReleaseFfBuffer();
	bool CurrentIsCompatibleTo(SubPicDesc const &spd) const;
	void ReCreateFfBuffer(SubPicDesc const &spd);
	void EreaseFfBuffer(CRect const &rect);
	void RecoverRGBA(SubPicDesc const &spd, CRect const & rect);
};
