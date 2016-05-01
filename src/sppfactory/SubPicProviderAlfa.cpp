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
