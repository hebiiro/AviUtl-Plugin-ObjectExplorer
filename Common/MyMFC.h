#pragma once

class COffscreenDC : public CDC
{
public:

	CBitmap m_bitmap;
	CBitmap* m_oldBitmap;
	int m_w, m_h;

	COffscreenDC()
	{
		m_oldBitmap = 0;
		m_w = 0;
		m_h = 0;
	}

	~COffscreenDC()
	{
		if (!GetSafeHdc())
			SelectObject(m_oldBitmap);
	}

	BOOL isValid()
	{
		return !!m_bitmap.GetSafeHandle();
	}

	BOOL update(CWnd* wnd)
	{
		CRect rc; wnd->GetClientRect(&rc);
		int w = rc.Width();
		int h = rc.Height();
		return update(wnd, w, h);
	}

	BOOL update(CWnd* wnd, int w, int h)
	{
		CClientDC dc(wnd);

		if (!GetSafeHdc())
		{
			CreateCompatibleDC(&dc);
			m_oldBitmap = GetCurrentBitmap();
		}

		if (m_w == w && m_h == h)
			return FALSE;

		m_w = w;
		m_h = h;

		if (m_bitmap.GetSafeHandle())
		{
			SelectObject(m_oldBitmap);
			m_bitmap.DeleteObject();
		}

		m_bitmap.CreateCompatibleBitmap(&dc, w, h);
		SelectObject(&m_bitmap);

		return TRUE;
	}
};
