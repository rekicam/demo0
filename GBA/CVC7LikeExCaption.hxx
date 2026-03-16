/////////////////////////////////////////////////////////////////////////////
// Written by Daniel Bowen (dbowen@es.com)
// Copyright (c) 2002 Daniel Bowen.
// WTL Docking windows

// Copyripht 2019 moecmks (agalis01@outlook.com)
// This file is part of ArchBoyAdvance.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 

// SOURCE: Daniel Bowen (dbowen@es.com)'s WTL Docking windows
// Modified to be compatible with the current WTL Dock Library 

# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVC7LINKEXCAPTION_INCLUED_HEADER_CXX_)
#  define _CVC7LINKEXCAPTION_INCLUED_HEADER_CXX_

namespace dockwins{

class CVC7LikeExCaption : public CCaptionBase
{
	typedef CVC7LikeExCaption thisClass;
	typedef CCaptionBase	baseClass;
public:
	enum{fntSpace=4,cFrameSpace=1,btnSpace=1,cMarginTop=2,cMarginBottom=2};
protected:
	typedef baseClass::CButton CButtonBase;

  struct CButton : CButtonBase
  {
      virtual void CalculateRect(CRect& rc, bool bHorizontal)
      {
			  CopyRect(rc);
			  if(bHorizontal)
			  {
				  top+=cMarginTop;
				  bottom-=cMarginBottom;
			  }
			  else
			  {
				  left+=cMarginTop;
				  right-=cMarginBottom;
			  }

			  DeflateRect(cFrameSpace+btnSpace,cFrameSpace+btnSpace);
			  if(bHorizontal)
			  {
				  left=right-Height();
				  rc.right=left+btnSpace;
			  }
			  else
			  {
				  bottom=top+Width();
				  rc.top=bottom+btnSpace;
			  }
      }
  };

	class CCloseButton: public CButton
	{
	public:
		virtual void Draw(CDC& dc)
		{
			CDWSettings settings;

			int cxOffset = 0, cyOffset = 0;
			if(m_state & Pressed)
			{
				cxOffset++;
				cyOffset++;
			}

			const int nMaxHeightSingleWidth = 15;

			// Erase Background
#ifndef DF_FOCUS_FEATURES
			BOOL bDescendantHasFocus = FALSE;
#else
      HWND hCurrent = ::WindowFromDC (dc);
      HWND hFocus = ::GetFocus();
			BOOL bDescendantHasFocus = ::IsChild(hCurrent, hFocus);
#endif
      dc.FillRect(this,::GetSysColorBrush(bDescendantHasFocus ? COLOR_ACTIVECAPTION : COLOR_3DFACE));
			// Paint Internal of Button

			if(settings.CYSmCaption() > nMaxHeightSingleWidth)
			{
				// Note: This may look funny, but DrawFrameControl
				//  actually uses a similar algorithm
				//  (with the Marlett font, etc.)
				CFont fontClose;
				fontClose.CreatePointFont(Height()*7,_T("Marlett"),dc);

				CFontHandle fontOld = dc.SelectFont(fontClose);
				COLORREF colorOld = dc.SetTextColor(::GetSysColor(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));
				int bkModeOld = dc.SetBkMode(TRANSPARENT);

				CSize size;
				dc.GetTextExtent(_T("r"), 1, &size);

				CRect rcDraw(this);
				rcDraw.OffsetRect(cxOffset,cyOffset);

				dc.DrawText(_T("r"),1,&rcDraw,DT_SINGLELINE|DT_CENTER|DT_VCENTER);

				if(bkModeOld != TRANSPARENT)
					dc.SetBkMode(bkModeOld);
				if(colorOld != ::GetSysColor(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT))
					dc.SetTextColor(colorOld);

				dc.SelectFont(fontOld);
				// fontClose gets destroyed when going out of scope
			}
			else
			{
				CPen pen;
				pen.CreatePen(PS_SOLID, 1, ::GetSysColor(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));

				HPEN hPenOld = dc.SelectPen(pen);
				const int sp=2;
				dc.MoveTo(left+cxOffset+sp, top+cyOffset+sp +1);
				dc.LineTo(right+cxOffset-sp -1, bottom+cyOffset-sp);
				dc.MoveTo(left+cxOffset+sp, bottom+cyOffset - sp-1);
				dc.LineTo(right+cxOffset-sp -1, top+cyOffset +sp );

				dc.SelectPen(hPenOld);
			}
      CButton::Draw(dc);
		}
	};
#ifdef DF_AUTO_HIDE_FEATURES
	class CPinButton : public CButton
	{
	public:
		typedef CPinIcons CIcons;
		CPinButton():m_state(CIcons::sPinned)
		{
		}
		void State(CIcons::States state)
		{
			m_state=state;
		}
		virtual void Draw(CDC& dc)
		{
			CDWSettings settings;

			int cxOffset = 0, cyOffset = 0;
			if(m_state & Pressed)
			{
				cxOffset++;
				cyOffset++;
			}

			// Erase Background
#ifndef DF_FOCUS_FEATURES
			BOOL bDescendantHasFocus = FALSE;
#else
      HWND hCurrent = ::WindowFromDC (dc);
			BOOL bDescendantHasFocus = ::IsChild(hCurrent, ::GetFocus());
#endif

			dc.FillRect(this,::GetSysColorBrush(bDescendantHasFocus ? COLOR_ACTIVECAPTION : COLOR_3DFACE));

			// Paint Internal of Button

			// This approximates the VS.NET algorithm.  The pin in
			// VS.NET depends on the small caption height.  This code
			// does as well, but doesn't always exactly match the drawing
			// code of VS.NET.  For example, the width of the lines
			// based on SmCaption are 1 (0-36), 2 (37-58), 3 (59-79), 4 (80-100+)
			int cySmCaption = settings.CYSmCaption();
			int nPenWidth = cySmCaption/20;
			if(nPenWidth < 1) nPenWidth = 1;

			LOGBRUSH lb = {BS_SOLID, ::GetSysColor(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT), 0};
			CPen pen;
			pen.CreatePen(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_SQUARE|PS_JOIN_BEVEL, nPenWidth, &lb);

			int nHCenter = left+Width()/2;
			int nVCenter = top+Height()/2;
			HPEN hPenOld = dc.SelectPen(pen);
			if(m_state == CPinIcons::sUnPinned)
			{
				// Note: there are slight differences in coord. because of our ExtCreatePen
				int nSegment = (cySmCaption/5);
				int nLeftSp = (cySmCaption<20) ? 1 : nPenWidth*3;
				//int nRightSp = (cySmCaption<16) ? 2 : ((cySmCaption<20) ? (cySmCaption-13) : nPenWidth*4);
				int nLip = cySmCaption/10;
				int nBoxWidth = (cySmCaption<20) ? 1 : (nLip-1);

				int nPinLeft = left+nLeftSp;
				int nLineLeft = nPinLeft + nSegment;
				int nPinRight = nLineLeft + 2*nSegment -1;
				//int nPinRight = right-nRightSp;
				int nLineTop = nVCenter-nSegment;
				int nLineBottom = nVCenter+nSegment;

				int nPinBodyTop = nVCenter+nLip-nSegment;
				int nPinBodyBottom = nVCenter-nLip+nSegment;

				dc.MoveTo(nPinLeft+cxOffset, nVCenter+cyOffset);
				dc.LineTo(nLineLeft+cxOffset, nVCenter+cyOffset);
				dc.MoveTo(nLineLeft+cxOffset, nLineTop+cyOffset);
				dc.LineTo(nLineLeft+cxOffset, nLineBottom+cyOffset);

				dc.MoveTo(nLineLeft+cxOffset, nPinBodyTop+cyOffset);
				dc.LineTo(nPinRight+cxOffset, nPinBodyTop+cyOffset);
				dc.LineTo(nPinRight+cxOffset, nPinBodyBottom+cyOffset);
				dc.LineTo(nLineLeft+cxOffset, nPinBodyBottom+cyOffset);

				RECT rcPinRight = {
					nLineLeft+cxOffset+1,
					nPinBodyBottom-nBoxWidth+cyOffset,
					nPinRight+cxOffset,
					nPinBodyBottom+cyOffset};
				dc.FillRect(&rcPinRight, ::GetSysColorBrush(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));
			}
			else
			{
				// Works better one pixel to the left
				cxOffset--;

				// Note: there are slight differences in coord. because of our ExtCreatePen
				int nSegment = (cySmCaption/5);
				int nBottomSp = (cySmCaption<20) ? 2 : nPenWidth*3;
				//int nTopSp = (cySmCaption<16) ? 2 : ((cySmCaption<20) ? (cySmCaption-13) : nPenWidth*4);
				int nLip = cySmCaption/10;
				int nBoxWidth = (cySmCaption<20) ? 1 : (nLip-1);

				int nPinBottom = bottom-nBottomSp;
				int nLineBottom = nPinBottom - nSegment;
				int nPinTop = nLineBottom - 2*nSegment +1;
				//int nPinTop = top+nTopSp;
				int nLineLeft = nHCenter-nSegment;
				int nLineRight = nHCenter+nSegment;

				int nPinBodyLeft = nHCenter+nLip-nSegment;
				int nPinBodyRight = nHCenter-nLip+nSegment;

				dc.MoveTo(nHCenter+cxOffset, nPinBottom+cyOffset);
				dc.LineTo(nHCenter+cxOffset, nLineBottom+cyOffset);
				dc.MoveTo(nLineLeft+cxOffset, nLineBottom+cyOffset);
				dc.LineTo(nLineRight+cxOffset, nLineBottom+cyOffset);

				dc.MoveTo(nPinBodyLeft+cxOffset, nLineBottom+cyOffset);
				dc.LineTo(nPinBodyLeft+cxOffset, nPinTop+cyOffset);
				dc.LineTo(nPinBodyRight+cxOffset, nPinTop+cyOffset);
				dc.LineTo(nPinBodyRight+cxOffset, nLineBottom+cyOffset);
				RECT rcPinRight = {
					nPinBodyRight-nBoxWidth+cxOffset,
					nPinTop+cyOffset+1,
					nPinBodyRight+cxOffset,
					nLineBottom+cyOffset};
				dc.FillRect(&rcPinRight, ::GetSysColorBrush(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));
			}

			dc.SelectPen(hPenOld);
      CButton::Draw(dc);
		}
	protected:
		CIcons::States	m_state;
	};
public:
	void SetPinButtonState(CPinButton::CIcons::States state)
	{
		m_btnPin.State(state);
	}
#endif
public:
	CVC7LikeExCaption():baseClass(0,false)
	{
		SetOrientation(!IsHorizontal());
	}

	void UpdateMetrics()
	{
		CDWSettings settings;

		m_thickness=settings.CYSmCaption()+cFrameSpace+cMarginBottom+cMarginTop;
	}

    void SetOrientation(bool bHorizontal)
    {
		if(IsHorizontal()!=bHorizontal)
		{
			baseClass::SetOrientation(bHorizontal);
			UpdateMetrics();
		}
    }

	bool CalculateRect(CRect& rc,bool bTop)
	{
		bool bRes=baseClass::CalculateRect(rc,bTop);
		CRect rcSpace(*this);
		m_btnClose.CalculateRect(rcSpace,IsHorizontal());
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.CalculateRect(rcSpace,IsHorizontal());
#endif
		return bRes;
	}
	void Draw(HWND hWnd,CDC& dc)
	{
#ifndef DF_FOCUS_FEATURES
		BOOL bDescendantHasFocus = FALSE;
#else
		BOOL bDescendantHasFocus = ::IsChild(hWnd, ::GetFocus());
#endif
	  CRect rcBorder(this);

		if(IsHorizontal())
		{
			rcBorder.top+=cMarginTop;
			rcBorder.bottom-=cMarginBottom;
		}
		else
		{
			rcBorder.left+=cMarginTop;
			rcBorder.right-=cMarginBottom;
		}

		if(bDescendantHasFocus)
		{
			dc.FillRect(this,::GetSysColorBrush(COLOR_3DFACE));
			dc.FillRect(&rcBorder,::GetSysColorBrush(COLOR_ACTIVECAPTION));
		}
		else
		{
			dc.FillRect(this,::GetSysColorBrush(bDescendantHasFocus ? COLOR_ACTIVECAPTION : COLOR_3DFACE));

			//dc.FrameRect(&rcBorder,::GetSysColorBrush(COLOR_BTNSHADOW));
			rcBorder.InflateRect(-1,0);
			dc.DrawEdge(&rcBorder,EDGE_ETCHED,BF_TOP|BF_BOTTOM|BF_FLAT);
			rcBorder.InflateRect(1,-1);
			dc.DrawEdge(&rcBorder,EDGE_ETCHED,BF_LEFT|BF_RIGHT|BF_FLAT);
		}

		int len=GetWindowTextLength(hWnd)+1;
		TCHAR* sText=new TCHAR[len];
		if(GetWindowText(hWnd,sText,len)!=0)
		{
			HFONT hFont = NULL;
			CDWSettings settings;
			CRect rc(rcBorder);
			if(IsHorizontal())
			{
				rc.left+=fntSpace+cFrameSpace;
#ifdef DF_AUTO_HIDE_FEATURES
				rc.right=m_btnPin.left-cFrameSpace-btnSpace;
#else
				rc.right=m_btnClose.left-cFrameSpace-btnSpace;
#endif
				hFont = settings.HSmCaptionFont();
			}
			else
			{
				rc.bottom-=fntSpace-cFrameSpace;
#ifdef DF_AUTO_HIDE_FEATURES
				rc.top=m_btnPin.bottom+cFrameSpace+btnSpace;
#else
				rc.top=m_btnClose.bottom+cFrameSpace+btnSpace;
#endif
				hFont = settings.VSmCaptionFont();
			}
			dc.SetTextColor(::GetSysColor(bDescendantHasFocus ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));
			dc.SetBkMode(TRANSPARENT);
			HFONT hFontOld = dc.SelectFont(hFont);
			if( (rc.left<rc.right) && (rc.top<rc.bottom))
				DrawEllipsisText(dc,sText,(int)_tcslen(sText),&rc,IsHorizontal());
			dc.SelectFont(hFontOld);
		}
		m_btnClose.Draw(dc);
#ifdef DF_AUTO_HIDE_FEATURES
		m_btnPin.Draw(dc);
#endif

		delete [] sText;
	}

	LRESULT HitTest(const CPoint& pt) const
	{
		LRESULT lRes=HTNOWHERE;
		if(PtInRect(pt))
		{
			lRes=HTCAPTION;
			if(m_btnClose.PtInRect(pt))
				lRes=HTCLOSE;
#ifdef DF_AUTO_HIDE_FEATURES
			else
			{
				if(m_btnPin.PtInRect(pt))
					lRes=HTPIN;
			}
#endif
		}
		return lRes;
	}
  bool Action(HWND hWnd, const CPoint& /*pt*/, WPARAM nHitTest)
  {
      bool res = false;

      switch (nHitTest)
      {
          case HTCLOSE:
              m_btnClose.Press(hWnd, true);
              res = true;
              break;
#ifdef DF_AUTO_HIDE_FEATURES

          case HTPIN:
              m_btnPin.Press(hWnd, true);
              res = true;
              break;
#endif
      }

      return res;
  }
  void ActionDone(HWND hWnd, WPARAM nHitTest, bool /*ok*/)
  {
      switch (nHitTest)
      {
          case HTCLOSE:
              m_btnClose.Press(hWnd, false);
              break;
#ifdef DF_AUTO_HIDE_FEATURES

          case HTPIN:
              m_btnPin.Press(hWnd, false);
              break;
#endif
      }
  }
  void HotTrack(HWND hWnd, WPARAM nHitTest)
  {
       m_btnClose.Hot(hWnd, nHitTest == HTCLOSE);
#ifdef DF_AUTO_HIDE_FEATURES
       m_btnPin.Hot(hWnd, nHitTest == HTPIN);
#endif
  }
protected:
#ifdef DF_AUTO_HIDE_FEATURES
	CPinButton		m_btnPin;
#endif
	CCloseButton	m_btnClose;
};
struct CVC7LikeCaption :  CVC7LikeExCaption
{
	void SetOrientation(bool /*bHorizontal*/)
	{
		// horizontal only
	}
};

typedef CDockingWindowTraits<CVC7LikeCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
							 CVC7LikeTitleDockingWindowTraits;

typedef CDockingWindowTraits<CVC7LikeExCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
							 CVC7LikeExTitleDockingWindowTraits;
typedef CDockingWindowTraits<CVC7LikeCaption,
								WS_CAPTION | WS_CHILD |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
								CVC7LikeAutoHidePaneTraits;
typedef CDockingWindowTraits<CVC7LikeExCaption,
								WS_CAPTION | WS_CHILD |
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
								CVC7LikeExAutoHidePaneTraits;
typedef CDockingBoxTraits<CVC7LikeCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP/* WS_CHILD*/ |
								/*WS_VISIBLE |*/ WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								WS_EX_TOOLWINDOW/* WS_EX_CLIENTEDGE*/> CVC7LikeDockingBoxTraits;

typedef CDockingBoxTraits<CVC7LikeExCaption,
								WS_OVERLAPPEDWINDOW | WS_POPUP/* WS_CHILD*/ |
								/*WS_VISIBLE |*/ WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
								WS_EX_TOOLWINDOW/* WS_EX_CLIENTEDGE*/> CVC7LikeExDockingBoxTraits;

typedef CBoxedDockingWindowTraits<CVC7LikeCaption, CTabDockingBox<CVC7LikeDockingBoxTraits>,
									WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
									WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
								CVC7LikeBoxedDockingWindowTraits;

typedef CBoxedDockingWindowTraits<CVC7LikeExCaption, CTabDockingBox<CVC7LikeExDockingBoxTraits>,
									WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
									WS_CLIPCHILDREN | WS_CLIPSIBLINGS,WS_EX_TOOLWINDOW>
								CVC7LikeExBoxedDockingWindowTraits;

#ifdef DF_AUTO_HIDE_FEATURES
typedef CDockingFrameTraitsT< CVC7LikeAutoHidePaneTraits,CSimpleSplitterBar<>,
		WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		WS_EX_APPWINDOW> CVC7LikeDockingFrameTraits;

typedef CDockingFrameTraitsT< CVC7LikeExAutoHidePaneTraits,CSimpleSplitterBar<>,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> CVC7LikeExDockingFrameTraits;

typedef CDockingFrameTraitsT<CVC7LikeAutoHidePaneTraits, CSimpleSplitterBarEx<>,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,0> CVC7LikeDockingSiteTraits;
#endif

}//namespace dockwins
#endif // __WTL_DW__VCLIKECAPTION_H__
