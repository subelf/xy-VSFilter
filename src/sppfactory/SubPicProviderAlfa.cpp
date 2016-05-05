#include "stdafx.h"

#include "SubPicProviderAlfa.h"

#include "..\dsutil\DSUtil.h"
#include "..\subpic\SimpleSubpicImpl.h"
#include "VobSubRectList.h"


CSubPicProviderAlfaImpl::CSubPicProviderAlfaImpl(TCHAR const *pName, ISubPicProviderEx *pSppx2) :
	CUnknown(pName, NULL),
	m_pSppx(pSppx2)
{
}

STDMETHODIMP CSubPicProviderAlfaImpl::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(ISubPicProviderAlfa)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CSubPicProviderAlfaImpl::CreateRectList(IVobSubRectList ** ppRectList)
{
	HRESULT hr = S_OK;

	if (ppRectList == nullptr)
	{
		hr = E_POINTER;
	}

	CAutoPtr<CVobSubRectList> apRectList;

	if (SUCCEEDED(hr))
	{
		apRectList.Attach(new CVobSubRectList());
		if (apRectList == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr))
	{
		(*ppRectList = apRectList.Detach())->AddRef();
	}

	return hr;
}

STDMETHODIMP CSubPicProviderAlfaImpl::RenderEx(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, IVobSubRectList *pRectList)
{
	SubPicDesc spd = CSubPicProviderAlfaImpl::GetSubPicDescFrom(spad);
	CAtlList<CRect> tRectList;

	HRESULT hr = this->m_pSppx->RenderEx(spd, rt, fps, tRectList);

	if (SUCCEEDED(hr))
	{
		if (pRectList != nullptr)
		{
			pRectList->RemoveAll();

			for (POSITION pos = tRectList.GetHeadPosition(); pos != NULL; )
			{
				pRectList->AddTail(tRectList.GetNext(pos));
			}
		}
	}

	return hr;
}

STDMETHODIMP CSubPicProviderAlfaImpl::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, RECT & bbox)
{
	CVobSubRectList tRectList;
	HRESULT hr = this->RenderAlpha(spad, rt, fps, &tRectList);
	

	if (SUCCEEDED(hr))
	{
		CRect sum(0, 0, 0, 0);
		for (POSITION iPos = tRectList.GetHeadPosition(); iPos != NULL; )
		{
			sum |= tRectList.GetNext(iPos);
		}
		bbox = sum;
	}

	return hr;
}

CSubPicProviderAlfaX2::CSubPicProviderAlfaX2(ISubPicProviderEx *pSppx, ISubPicProviderEx2 *pSppx2):
	CSubPicProviderAlfaImpl(_T("CSubPicProviderAlfaX2"), pSppx),
	m_pSppx2(pSppx2)
{
}

STDMETHODIMP CSubPicProviderAlfaX2::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, IVobSubRectList *pRectList)
{
	SubPicDesc spd = GetSubPicDescFrom(spad);

	CSize output_size = CSize(spd.w, spd.h);
	CRectCoor2 video_rect = CRect(0, 0, spd.w, spd.h);

	CComPtr<IXySubRenderFrame> sub_render_frame;
	HRESULT hr = m_pSppx2->RenderEx(&sub_render_frame, spd.type, 
		video_rect, video_rect, output_size, rt, fps);
	
	if (!sub_render_frame)
	{
		hr = E_FAIL;
	}

	if (pRectList != nullptr)
	{
		int count = 0;
		if (SUCCEEDED(hr))
		{
			hr = sub_render_frame->GetBitmapCount(&count);
		}

		if (SUCCEEDED(hr))
		{
			pRectList->RemoveAll();

			for (int i = 0; i < count; i++)
			{
				POINT pos;
				SIZE size;
				LPCVOID pixels;
				int pitch;

				hr = sub_render_frame->GetBitmap(i, NULL, &pos, &size, &pixels, &pitch);
				if (FAILED(hr))
				{
					break;
				}
				pRectList->AddTail(CRect{ pos, size });
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		CComPtr<ISimpleSubPic> apSimSp(new SimpleSubpic(sub_render_frame, spd.type));
		hr = apSimSp->AlphaBlt(&spd);
	}

	return hr;
}

CSubPicProviderAlfaX::CSubPicProviderAlfaX(ISubPicProviderEx * pSppx) :
	CSubPicProviderAlfaImpl(_T("CSubPicProviderAlfaX"), pSppx),
	m_lastSpd()
{
	m_lastSpd.type = MSP_RGBA;
	m_lastSpd.bpp = 32;
	m_lastSpd.bits = nullptr;
}

CSubPicProviderAlfaX::~CSubPicProviderAlfaX()
{
	if (m_lastSpd.bits != nullptr)
	{
		delete[] m_lastSpd.bits;
		m_lastSpd.bits = nullptr;
	}
}

STDMETHODIMP CSubPicProviderAlfaX::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, IVobSubRectList *pRectList)
{
	SubPicDesc spd = GetSubPicDescFrom(spad);

	CAtlList<CRect> tRectList;
	HRESULT hr = m_pSppx->RenderEx(spd, rt, fps, tRectList);

	if (SUCCEEDED(hr))
	{
		if (!this->CurrentIsCompatibleTo(spd))
		{
			this->ReCreateFfBuffer(spd);
		}

		for (POSITION pos = tRectList.GetHeadPosition(); pos != NULL; )
		{
			this->EreaseFfBuffer(tRectList.GetNext(pos));
		}

		tRectList.RemoveAll();
		hr = m_pSppx->RenderEx(m_lastSpd, rt, fps, tRectList);
	}

	if (SUCCEEDED(hr))
	{
		bool const &hasRectList = pRectList != nullptr;
		for (POSITION pos = tRectList.GetHeadPosition(); pos != NULL; )
		{
			CRect const & tRect = tRectList.GetNext(pos);
			this->RecoverRGBA(spd, tRect);
			if (hasRectList)
			{
				pRectList->AddTail(tRect);
			}
		}
	}

	return hr;
}

bool CSubPicProviderAlfaX::CurrentIsCompatibleTo(SubPicDesc const & spd) const
{
	SubPicDesc const &cur = this->m_lastSpd;
	bool isComp = (cur.w == spd.w) && (cur.h == spd.h) &&
		(cur.vidrect.left == spd.vidrect.left) &&
		(cur.vidrect.right == spd.vidrect.right) &&
		(cur.vidrect.top == spd.vidrect.top) &&
		(cur.vidrect.bottom == spd.vidrect.bottom) &&
		(cur.bits != nullptr);

	return isComp;
}

void CSubPicProviderAlfaX::ReCreateFfBuffer(SubPicDesc const & spd)
{
	SubPicDesc &cur = this->m_lastSpd;
	cur.type = MSP_RGBA;

	cur.w = spd.w; cur.h = spd.h; cur.bpp = spd.bpp;
	cur.pitch = spd.pitch;

	cur.vidrect = spd.vidrect;
	this->ReleaseFfBuffer();

	int const &bufferSize = cur.pitch * cur.h;
	cur.bits = new BYTE[bufferSize];
	//::memset(cur.bits, 0xff, bufferSize);
}

void CSubPicProviderAlfaX::EreaseFfBuffer(CRect const & rect)
{
	bool const &fYDirectionPositive = (rect.top <= rect.bottom);
	int const &yTrueTop = fYDirectionPositive ? rect.top : (rect.bottom - 1);
	int const &yTrueBottom = fYDirectionPositive ? rect.bottom : (rect.top - 1);

	bool const& fXDirectionPositive = (rect.left <= rect.right);
	int const &xTrueLeft = fXDirectionPositive ? rect.left : (rect.right - 1);
	int const &xTrueRight = fXDirectionPositive ? rect.right : (rect.left - 1);
	int const &xWidth = xTrueRight - xTrueLeft;

	int const &xDelta = (xTrueLeft * m_lastSpd.bpp) >> 3;
	BYTE* const &b1 = ((BYTE*)m_lastSpd.bits) + xDelta;

	int const &yBtmOfs1 = yTrueBottom * m_lastSpd.pitch;

	for (int y1 = yTrueTop * m_lastSpd.pitch;
		y1 != yBtmOfs1;
		y1 += m_lastSpd.pitch)
	{
		DWORD* const &r1 = ((DWORD*)(b1 + y1));
		::memset(r1, 0xff, (xWidth * m_lastSpd.bpp) >> 3);
	}
}

DWORD static inline RecoverRGBA_c(DWORD const &pixel00, DWORD const &a)
{
	DWORD const &x0 = pixel00;

	DWORD const maskG = 0x0000ff00;
	DWORD const maskRB = 0x00ff00ff;

	DWORD const& iAff00 = 255 * 256 / a;
	DWORD const& tRB = (x0 & maskRB);
	DWORD const& tG = (x0 & maskG) >> 8;

	return (a << 24) | (((tRB * iAff00) >> 8) & maskRB) | ((tG * iAff00) & maskG);
}

////No improvement
//DWORD static inline RecoverRGBA_sse2(DWORD const &pixel00, DWORD const &a)
//{
//	WORD const& iAff00 = (255 * 512 / a + 1) >> 1;
//
//	__m128i zero = _mm_setzero_si128();
//	__m128i invA = _mm_set_epi16(0, 0, 0, 0, 0, iAff00, iAff00, iAff00);
//	__m128i mmr0 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(pixel00), zero);
//	mmr0 = _mm_srli_epi16(_mm_mullo_epi16(invA, mmr0), 8);
//	mmr0 = _mm_packus_epi16(mmr0, zero);		
//		
//	return (a << 24) | _mm_cvtsi128_si32(mmr0);
//}

typedef DWORD (*FPRecoverRGBA) (DWORD const &pixel00, DWORD const &pixelff);

void CSubPicProviderAlfaX::RecoverRGBA(SubPicDesc const &spd, CRect const & rect)
{
	//FPRecoverRGBA fpRecoverRGBA = (g_cpuid.m_flags & CCpuID::sse2) ? &RecoverRGBA_sse2 : &RecoverRGBA_c;

	bool const &fYDirectionPositive = (rect.top <= rect.bottom);
	int const &yTrueTop = fYDirectionPositive ? rect.top : (rect.bottom - 1);
	int const &yTrueBottom = fYDirectionPositive ? rect.bottom : (rect.top - 1);

	bool const& fXDirectionPositive = (rect.left <= rect.right);
	int const &xTrueLeft = fXDirectionPositive ? rect.left : (rect.right - 1);
	int const &xTrueRight = fXDirectionPositive ? rect.right : (rect.left - 1);
	
	BYTE* const &b0 = ((BYTE*)spd.bits);
	BYTE* const &b1 = ((BYTE*)m_lastSpd.bits);

	int const &yBtmOfs0 = yTrueBottom * spd.pitch;
	int const &yBtmOfs1 = yTrueBottom * m_lastSpd.pitch;

	for (int y0 = yTrueTop * spd.pitch, y1 = yTrueTop * m_lastSpd.pitch;
		y0 != yBtmOfs0;
		y0 += spd.pitch, y1 += m_lastSpd.pitch)
	{
		DWORD* const &r0 = ((DWORD*)(b0 + y0));
		DWORD* const &r1 = ((DWORD*)(b1 + y1));

		for (int x = xTrueLeft; x != xTrueRight; x++)
		{
			DWORD &x0 = r0[x];
			DWORD const &x1 = r1[x];

			DWORD const maskR = 0x000000ff;
			DWORD const maskG = 0x0000ff00;
			DWORD const maskRB = 0x00ff00ff;

			DWORD const &delta = x1 - x0;
			DWORD const &rA2 = (delta & maskR) + ((delta & maskG) >> 8);
			DWORD const &tA = maskR ^ ((rA2 + 1) >> 1);	//255 - round(rA2 / 2)

			x0 = tA ? RecoverRGBA_c(x0, tA) : 0;
		}
	}
}


void CSubPicProviderAlfaX::ReleaseFfBuffer()
{
	SubPicDesc &cur = this->m_lastSpd;
	if (cur.bits != nullptr)
	{
		delete[] cur.bits;
		cur.bits = nullptr;
	}
}

