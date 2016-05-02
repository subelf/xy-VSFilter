#include "stdafx.h"

#include "..\dsutil\DSUtil.h"
#include "SubPicProviderAlfa.h"

#include "..\subpic\SimpleSubpicImpl.h"


CSubPicProviderAlfaImpl::CSubPicProviderAlfaImpl(ISubPicProviderEx *pSppx2) :
	CUnknown(_T("CSubPicProviderAlfa"), NULL),
	m_pSppx(pSppx2)
{
}

STDMETHODIMP CSubPicProviderAlfaImpl::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return
		QI(ISubPicProviderAlfa)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CSubPicProviderAlfaImpl::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, RECT & bbox)
{
	CAtlList<CRect> rectList;
	HRESULT result = this->RenderAlpha(spad, rt, fps, rectList);
	POSITION pos = rectList.GetHeadPosition();
	CRect bbox2(0, 0, 0, 0);
	while (pos != NULL)
	{
		bbox2 |= rectList.GetNext(pos);
	}
	bbox = bbox2;
	return result;
}

CSubPicProviderAlfaX2::CSubPicProviderAlfaX2(ISubPicProviderEx *pSppx, ISubPicProviderEx2 *pSppx2):
	CSubPicProviderAlfaImpl(pSppx),
	m_pSppx2(pSppx2)
{
}

STDMETHODIMP CSubPicProviderAlfaX2::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, CAtlList<CRect>& rectList)
{
	SubPicDesc spd = GetSubPicDescFrom(spad);

	CSize output_size = CSize(spad.w, spad.h);
	rectList.RemoveAll();

	CComPtr<IXySubRenderFrame> sub_render_frame;
	HRESULT hr = m_pSppx2->RenderEx(&sub_render_frame, spd.type, 
		output_size, output_size, spad.vidrect, rt, fps);
	
	if (!sub_render_frame)
	{
		hr = E_FAIL;
	}

	int count = 0;
	if (SUCCEEDED(hr))
	{
		hr = sub_render_frame->GetBitmapCount(&count);
	}

	if (SUCCEEDED(hr))
	{
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
			rectList.AddTail(CRect(pos, size));
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
	CSubPicProviderAlfaImpl(pSppx),
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

STDMETHODIMP CSubPicProviderAlfaX::RenderAlpha(SubPicAlfaDesc & spad, REFERENCE_TIME rt, double fps, CAtlList<CRect>& rectList)
{
	rectList.RemoveAll();

	SubPicDesc spd = GetSubPicDescFrom(spad);
	HRESULT hr = m_pSppx->RenderEx(spd, rt, fps, rectList);

	if (SUCCEEDED(hr))
	{
		if (!this->CurrentIsCompatibleTo(spd))
		{
			this->ReCreateFfBuffer(spd);
		}

		for (POSITION pos = rectList.GetHeadPosition(); pos != NULL; )
		{
			this->EreaseFfBuffer(rectList.GetNext(pos));
		}

		hr = m_pSppx->RenderEx(m_lastSpd, rt, fps, rectList);
	}

	if (SUCCEEDED(hr))
	{
		for (POSITION pos = rectList.GetHeadPosition(); pos != NULL; )
		{
			this->RecoverRGBA(spd, rectList.GetNext(pos));
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
	int const &xDelta = (rect.left * m_lastSpd.bpp) >> 3;
	BYTE* const &b1 = ((BYTE*) m_lastSpd.bits) + xDelta;

	for (int y = rect.top; y != rect.bottom; y++)
		//for (int x = xTrueLeft; x != xTrueRight; x+=xStep)
	{
		DWORD* const &r1 = ((DWORD*)(b1 + y * m_lastSpd.pitch));
		::memset(r1, 0xff, ((rect.right - rect.left) * m_lastSpd.bpp) >> 3);
	}
}

void CSubPicProviderAlfaX::RecoverRGBA(SubPicDesc const &spd, CRect const & rect)
{
	//bool const &fYDirectionPositive = (rect.top <= rect.bottom);
	//int const &yStep = fYDirectionPositive ? 1 : -1;
	//int const &yTrueTop = fYDirectionPositive ? rect.top : rect.bottom;
	//int const &yTrueBottom = (fYDirectionPositive ? rect.bottom : rect.top) + yStep;

	//bool const& fXDirectionPositive = (rect.left <= rect.right);
	//int xStep = fXDirectionPositive ? 1 : -1;
	//int const &xWidth = 1 + (fXDirectionPositive ? rect.right - rect.left : rect.left - rect.right);
	//int const &xTrueLeft = fXDirectionPositive ? rect.left : rect.right;
	//int const &xTrueRight = (fXDirectionPositive ? rect.right : rect.left) + xStep;

	//int const &xDelta = (rect.left * m_lastSpd.bpp) >> 3;

	BYTE* const &b0 = ((BYTE*)spd.bits);
	BYTE* const &b1 = ((BYTE*)m_lastSpd.bits);

	for (int y = rect.top; y != rect.bottom; y++)
	{
		DWORD* const &r0 = ((DWORD*)(b0 + y * spd.pitch));
		DWORD* const &r1 = ((DWORD*)(b1 + y * m_lastSpd.pitch));

		for (int x = rect.left; x != rect.right; x++)
		{
			DWORD &x0 = r0[x];
			DWORD const &x1 = r1[x];

			DWORD const maskR = 0x000000ff;
			DWORD const maskG = 0x0000ff00;
			DWORD const maskB = 0x00ff0000;
			//DWORD const maskRB = 0x00ff00ff;
			DWORD const &rA2 = ((x1 & maskR) - (x0 & maskR)) + 
				(((x1 & maskG) - (x0 & maskG)) >> 8);
			DWORD const &tA = 0xff - (rA2 >> 1);

			if (tA)	// alpha != 0;
			{
				DWORD const& iAff00 = 255 * 256 / tA;
				DWORD const& tR = (x0 & maskR);
				DWORD const& tG = (x0 & maskG) >> 8;
				DWORD const& tB = (x0 & maskB) >> 8;

				x0 = (tA << 24) |
					((tR * iAff00) >> 8) |
					((tG * iAff00) & maskG) |
					((tB * iAff00) & maskB);
			}
			else
			{
				x0 = 0;
			}
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

