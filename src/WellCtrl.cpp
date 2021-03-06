// WellCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "WPhast.h"
#include "WellCtrl.h"


// CWellCtrl

IMPLEMENT_DYNAMIC(CWellCtrl, CStatic)
CWellCtrl::CWellCtrl()
: m_bByDepth(FALSE)
{
	this->m_grid.SetUniformRange(0.0, 1.0, 2);
}

CWellCtrl::~CWellCtrl()
{
}


BEGIN_MESSAGE_MAP(CWellCtrl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CWellCtrl message handlers

void CWellCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages

	LPCTSTR LABEL_FORMAT = _T(" %1.4g");
	LPCTSTR ELEVATION    = _T("Elevation");
	LPCTSTR DEPTH        = _T("Depth");

	const int top_border        = 35;
	const int ground_height     = 15;
	const int bottom_border     = 14;
	const int pipe_inner_radius = 8;
	const int pipe_outer_radius = pipe_inner_radius + 2;
	const int ground_indent     = 8;


	const COLORREF clrWhite = RGB(255, 255, 255);
	const COLORREF clrBlue  = RGB(  0,   0, 255);
	const COLORREF clrBrown = RGB(128,  42,  42);
	const COLORREF clrAqua  = RGB(  0, 200, 200);
	const COLORREF clrRed   = RGB(255,   0,   0);
	const COLORREF clrBlack = RGB(  0,   0,   0);

	CRect rcClient;
	this->GetClientRect(&rcClient);

	
	CDC memDC;
	CBitmap memBitmap;
	CBitmap* oldBitmap;

	memDC.CreateCompatibleDC(&dc);
	memBitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	oldBitmap = (CBitmap*)memDC.SelectObject(&memBitmap);

	// Setup grid
	//
	this->m_grid.Setup();
	double zMin = this->m_grid.coord[0];
	double zMax = this->m_grid.coord[this->m_grid.count_coord - 1];

	// Fill background
	//
	CDC *pDC = &memDC;
	pDC->FillSolidRect(&rcClient, clrWhite);

	// determine depth label indent
	//
	int indent_right = 0;

	if (this->m_bByDepth)
	{
		int nSavedDC = pDC->SaveDC();
		pDC->SelectObject(CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)));
		CString strLabel;
		CSize extent;
		CSize max_extent(0, 0);
		for (int i = 0; i < this->m_grid.count_coord; ++i)
		{
			if (((this->m_zLSD - this->m_grid.coord[i]) < 1e-12) && (this->m_grid.coord[i] > 1e-4))
			{
				strLabel.Format(LABEL_FORMAT, 0.0);
			}
			else
			{
				strLabel.Format(LABEL_FORMAT, this->m_zLSD - this->m_grid.coord[i]);
			}
			extent = pDC->GetTextExtent(strLabel);
			max_extent.cx = __max(max_extent.cx, extent.cx);
			max_extent.cy = __max(max_extent.cy, extent.cy);
		}
		indent_right = max_extent.cx + 3;
		pDC->RestoreDC(nSavedDC);
	}

	// determine elevation label indent
	//
	int indent_left = 0;
	{
		int nSavedDC = pDC->SaveDC();
		pDC->SelectObject(CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)));
		CString strLabel;
		CSize extent;
		CSize max_extent(0, 0);
		for (int i = 0; i < this->m_grid.count_coord; ++i)
		{
			strLabel.Format(LABEL_FORMAT, this->m_grid.coord[i]);
			extent = pDC->GetTextExtent(strLabel);
			max_extent.cx = __max(max_extent.cx, extent.cx);
			max_extent.cy = __max(max_extent.cy, extent.cy);
		}
		indent_left = max_extent.cx + 4 + 3;
		pDC->RestoreDC(nSavedDC);
	}

	// Determine dashed lines modulus
	//
	int line_modulus = 1;
	if (this->m_grid.count_coord >= 200)
	{
		line_modulus = this->m_grid.count_coord / 20;
	}
	else if (this->m_grid.count_coord >= 80)
	{
		line_modulus = 5;
	}

	// Draw dashed lines
	//
	{
		int nSavedDC = pDC->SaveDC();

		CPen pen(PS_DOT, 0, clrBlue);
		pDC->SelectObject(pen);

		// top
		pDC->MoveTo(indent_left                    , top_border);
		pDC->LineTo(rcClient.Width() - indent_right, top_border);

		// middle points
		for (int i = line_modulus; i < this->m_grid.count_coord - 1; i += line_modulus)
		{
			int cy = rcClient.Height() - top_border - bottom_border;
			int y = top_border + int( cy * ( (zMax - this->m_grid.coord[i]) / (zMax - zMin) ) );
			pDC->MoveTo(indent_left                    , y);
			pDC->LineTo(rcClient.Width() - indent_right, y);
		}

		// bottom
		pDC->MoveTo(indent_left                    , rcClient.Height() - bottom_border);
		pDC->LineTo(rcClient.Width() - indent_right, rcClient.Height() - bottom_border);

		pDC->RestoreDC(nSavedDC);
	}

	// Determine label modulus
	//
	int modulus = 1;
	if (this->m_grid.count_coord >= 200)
	{
		modulus = this->m_grid.count_coord / 10;
	}
	else if (this->m_grid.count_coord >= 80)
	{
		modulus = 10;
	}
	else if (this->m_grid.count_coord >= 40)
	{
		modulus = 5;
	}
	else if (this->m_grid.count_coord >= 20)
	{
		modulus = 2;
	}


	// Draw Depth Labels
	//
	if (this->m_bByDepth)
	{
		int nSavedDC = pDC->SaveDC();

		pDC->SetTextColor(clrBlue);
		pDC->SelectObject(CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)));

		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		CString strLabel;
		CSize extent;

		// top
		if (((this->m_zLSD - zMax) < 1e-12) && (zMax > 1e-4))
		{
			strLabel.Format(LABEL_FORMAT, 0.0);
		}
		else
		{
			strLabel.Format(LABEL_FORMAT, this->m_zLSD - zMax);
		}

		extent = pDC->GetTextExtent(strLabel);
		int yLast = top_border;

		// bottom
		strLabel.Format(LABEL_FORMAT, this->m_zLSD - zMin);
		extent = pDC->GetTextExtent(strLabel);

		// middle points
		int cy = rcClient.Height() - top_border - bottom_border;
		for (int i = this->m_grid.count_coord - 1; i >= 0; i -= modulus)
		{
			int y = top_border + int( cy * ( (zMax - this->m_grid.coord[i]) / (zMax - zMin) ) );
			strLabel.Format(LABEL_FORMAT, this->m_zLSD - this->m_grid.coord[i]);
			extent = pDC->GetTextExtent(strLabel);
			pDC->TextOut(rcClient.Width() - extent.cx - 3, y - extent.cy / 2, strLabel);
		}

		// bottom
		strLabel.Format(LABEL_FORMAT, this->m_zLSD - zMin);
		extent = pDC->GetTextExtent(strLabel);
		pDC->TextOut(rcClient.Width() - extent.cx - 3, rcClient.Height() - bottom_border - extent.cy / 2, strLabel);

		// top
		strLabel.Format(LABEL_FORMAT, this->m_zLSD - zMax);
		extent = pDC->GetTextExtent(strLabel);
		pDC->TextOut(rcClient.Width() - extent.cx - 3, top_border - extent.cy / 2, strLabel);

		pDC->RestoreDC(nSavedDC);
	}

	// Draw Elevation Labels
	//
	{
		int nSavedDC = pDC->SaveDC();

		pDC->SetTextColor(clrBlue);
		pDC->SelectObject(CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)));

		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		CString strLabel;
		CSize extent;

		// middle points
		for (int i = modulus; i < this->m_grid.count_coord - 1; /*++i*/ i += modulus)
		{
			int cy = rcClient.Height() - top_border - bottom_border;
			int y = top_border + int( cy * ( (zMax - this->m_grid.coord[i]) / (zMax - zMin) ) );
			strLabel.Format(LABEL_FORMAT, this->m_grid.coord[i]);
			extent = pDC->GetTextExtent(strLabel);
			pDC->TextOut(indent_left - extent.cx - 3, y - extent.cy / 2, strLabel);
		}

		// top
		strLabel.Format(LABEL_FORMAT, zMax);
		extent = pDC->GetTextExtent(strLabel);
		pDC->TextOut(indent_left - extent.cx - 3, top_border - extent.cy / 2, strLabel);

		// bottom
		strLabel.Format(LABEL_FORMAT, zMin);
		extent = pDC->GetTextExtent(strLabel);
		pDC->TextOut(indent_left - extent.cx - 3, rcClient.Height() - bottom_border - extent.cy / 2, strLabel);

		pDC->RestoreDC(nSavedDC);
	}

	// Draw ground
	//
	int yTotal = (rcClient.Height() - top_border - bottom_border);
	int yGround = top_border + int( yTotal * ( (zMax - this->m_zLSD) / (zMax - zMin) ) );
	if (this->m_bByDepth)
	{
		int ground_left = indent_left + (((zMax - this->m_zLSD) == 0) ? ground_indent : 0);

		CRect rcGround(indent_left, yGround, rcClient.Width() - indent_right, yGround + ground_height);
		CBrush brGround;

		// draw ground hatch
		brGround.CreateHatchBrush(HS_DIAGCROSS, clrBrown);
		pDC->FillRect(&rcGround, &brGround);

		// draw ground edge
		pDC->MoveTo(indent_left                    , yGround);
		pDC->LineTo(rcClient.Width() - indent_right, yGround);
	}

	// Determine the top of the well
	//
	int well_top = (this->m_bByDepth) ? __max(top_border, yGround) : top_border;


	// Erase pipe background
	//
	{
		// Set outer pipe rectangle
		CRect rcPipe( rcClient.Width() / 2 - pipe_outer_radius, well_top, rcClient.Width() / 2 + pipe_outer_radius, rcClient.Height() - bottom_border);
		pDC->FillSolidRect(&rcPipe, clrWhite);
	}


	// Draw pipe gradient
	//
	{
		CRect rcPipe;
		CRect rcBand;

		// Set inner pipe rectangle
		rcPipe.SetRect(rcClient.Width() / 2 - pipe_inner_radius, well_top, rcClient.Width() / 2 + pipe_inner_radius, rcClient.Height() - bottom_border);

		// Determine how large each band should be in order to cover the
		// client with 100 bands
		float fStep = (float)(rcPipe.right - rcPipe.left) / 100.0f;

		for (int nBand = 0; nBand <= 100; ++nBand)
		{
			rcBand.SetRect(
				rcPipe.left + (int)(nBand * fStep),        // Upper left X
				rcPipe.top,                                // Upper left Y
				rcPipe.left + (int)((nBand + 1) * fStep),  // Lower right X
				rcPipe.bottom                              // Lower right Y
				);

			// Create a brush with the appropriate color for this band
			CBrush brBand;
			brBand.CreateSolidBrush( RGB(255 - nBand, 255 - nBand, 255 - nBand) );

			// Fill the band
			pDC->FillRect(&rcBand, &brBand);
		}
	}	

	// Draw screens
	//
	for (std::list<screen>::const_iterator i = this->m_screens.begin(); i != this->m_screens.end(); ++i)
	{
		CRect rcPipe(
			rcClient.Width() / 2 - pipe_inner_radius,
			top_border, rcClient.Width() / 2 + pipe_inner_radius,
			rcClient.Height() - bottom_border
			);

		CRect rcScreen(
			rcPipe.left,
			(int)(rcPipe.Height() * ((zMax - (*i).top) / (zMax - zMin))) + rcPipe.top,
			rcPipe.right,
			(int)(rcPipe.Height() * ((zMax - (*i).bottom) / (zMax - zMin))) + rcPipe.top
			);

		CBrush brScreen;
		brScreen.CreateSolidBrush(clrAqua);
		pDC->FillRect(&rcScreen, &brScreen);

		// Draw horz stripes
		//
		CBrush brStripe;
		brStripe.CreateSolidBrush(clrBlack);

		CRect rcStripe;
		for (int n = 0; n <= rcScreen.Height(); n += 3)
		{

			rcStripe.SetRect(
				rcScreen.left,
				rcScreen.top + n,
				rcScreen.right,
				rcScreen.top + n + 1
				);

			// Fill the stripe
			pDC->FillRect(&rcStripe, &brStripe);
		}
	}

	// Draw pipe outline
	//
	pDC->MoveTo(rcClient.Width() / 2 - pipe_outer_radius,                          well_top);  // UpperLeft
	pDC->LineTo(rcClient.Width() / 2 + pipe_outer_radius,                          well_top);  // UpperRight
	pDC->LineTo(rcClient.Width() / 2 + pipe_outer_radius, rcClient.Height() - bottom_border);  // LowerRight
	pDC->LineTo(rcClient.Width() / 2 - pipe_outer_radius, rcClient.Height() - bottom_border);  // LowerLeft
	pDC->LineTo(rcClient.Width() / 2 - pipe_outer_radius,                          well_top);  // UpperLeft

	pDC->MoveTo(rcClient.Width() / 2 - pipe_inner_radius,                          well_top);
	pDC->LineTo(rcClient.Width() / 2 - pipe_inner_radius, rcClient.Height() - bottom_border);

	pDC->MoveTo(rcClient.Width() / 2 + pipe_inner_radius,                          well_top);
	pDC->LineTo(rcClient.Width() / 2 + pipe_inner_radius, rcClient.Height() - bottom_border);


	// Draw axis titles
	//
	{
		int nSavedDC = pDC->SaveDC();
		pDC->SetTextColor(clrBlue);
		pDC->SelectObject(CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)));
		pDC->TextOut(3, 3, ELEVATION);
		if (this->m_bByDepth)
		{
			CSize extent;
			extent = pDC->GetTextExtent(DEPTH);
			pDC->TextOut(rcClient.Width() - extent.cx - 3, 3, DEPTH);
		}
		pDC->RestoreDC(nSavedDC);
	}

	// Draw client frame
	//
	pDC->FrameRect(&rcClient, CBrush::FromHandle((HBRUSH)::GetStockObject(BLACK_BRUSH)));

	// Send result to the display
	//
	if (memDC.GetSafeHdc() != NULL)
	{
		dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);
	}
	memDC.SelectObject(oldBitmap);
}

BOOL CWellCtrl::SetGrid(const struct grid &g)
{
	this->m_grid = g;
	return TRUE; // ok
}

BOOL CWellCtrl::SetLSD(double zLSD)
{
	this->m_zLSD = zLSD;

	if (this->GetSafeHwnd()) this->Invalidate();
	return TRUE; // ok
}

BOOL CWellCtrl::AddScreen(double zMin, double zMax)
{
	if (zMin > zMax) {
		double temp = zMin;
		zMin = zMax;
		zMax = temp;
	}

	screen scr;
	scr.top    = zMax;
	scr.bottom = zMin;

	BOOL bOk = FALSE;
	try
	{
		this->m_screens.push_back(scr);
		bOk = TRUE;
	}
	catch (...)
	{
	}

	if (this->GetSafeHwnd()) this->Invalidate();
	return bOk;
}

BOOL CWellCtrl::RemoveAllScreens(void)
{
	this->m_screens.clear();
	if (this->GetSafeHwnd()) this->Invalidate();
	return TRUE;
}

BOOL CWellCtrl::RemoveScreen(double zMin, double zMax)
{
	ASSERT(zMin < zMax);

	screen scr;
	scr.top    = zMax;
	scr.bottom = zMin;

	std::list<screen>::iterator iter = this->m_screens.begin();
	for (; iter != this->m_screens.end(); ++iter) {
		if ((*iter).top == (scr.top) && (*iter).bottom == (scr.bottom)) break;
	}

	if (iter != this->m_screens.end()) {
		this->m_screens.erase(iter);
		if (this->GetSafeHwnd()) this->Invalidate();
		return TRUE; // found
	}

	return FALSE; // NOT found
}

int CWellCtrl::GetScreenCount() const
{
	return (int) this->m_screens.size();
}

BOOL CWellCtrl::GetByDepth(void)const
{
	return this->m_bByDepth;
}

void CWellCtrl::SetByDepth(BOOL bByDepth)
{
	this->m_bByDepth = bByDepth;
	if (this->GetSafeHwnd()) this->Invalidate();
}
