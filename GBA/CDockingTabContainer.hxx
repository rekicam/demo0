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

// Simple changes support broadcast
// WM_CLOSE messages at closing time to all link windows and HOOK  CTCN_CLOSE message 
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CDOCKINGTABCONTAINER_INCLUED_HEADER_CXX_)
#  define _CDOCKINGTABCONTAINER_INCLUED_HEADER_CXX_

# include <WTL/atlgdi.h>
# include <dockwins/TabDockingBox.h>
# include "CVC7LikeExCaption.hxx"

namespace dockwins
{
template<class TTraits = COutlookLikeDockingBoxTraits>
class CDockingTabContainer : public  dockwins::CTabDockingBox <TTraits>   
{
public:
  typedef    dockwins::CTabDockingBox <TTraits> baseClass;
  typedef CDockingTabContainer  thisClass;

  DECLARE_WND_CLASS(_T("CDockingTabContainer"))

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    NOTIFY_CODE_HANDLER (CTCN_CLOSE, OnCloseButtonHit)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  LRESULT OnCloseButtonHit (int dummy_var, NMHDR *pNMHDR, BOOL& bHandled) 
  {  
    bHandled = FALSE;
    NMCTCITEM *pNMCtc = (NMCTCITEM *) pNMHDR;   
    DFMHDR dfmhdr;
    // dock current tab item. 
    HWND tempWnd = GetItemHWND (pNMCtc->iItem);
    dfmhdr.hWnd = tempWnd;
    dfmhdr.hBar = m_hWnd;
    dfmhdr.code = DC_UNDOCK;
    OnUndock (& dfmhdr);
    // post WM_CLOSE to select's tab item.
    :: PostMessage (tempWnd, WM_CLOSE, 0, 0);
    return 0;
  }  

  LRESULT OnClose (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (wParam == FALSE) { 
      // see TabDockingBox.h's IsStillAlive 
      int n = m_tabs.GetItemCount();
      for (int id = 0; id != n; id++) 
     // :: SendMessage (GetItemHWND(id), WM_CLOSE, 0, 0);
      :: PostMessage (GetItemHWND(id), WM_CLOSE, 0, 0);
    }
    bHandled = FALSE;
    return !0;   // TIPS: https://msdn.microsoft.com/zh-cn/library/ms632617
  }
  LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    //int n = m_tabs.Set();
    //for (int id = 0; id != n; id++) 
     // :: SendMessage (GetItemHWND(id), WM_CLOSE, 0, 0);
     // :: PostMessage (GetItemHWND(id), WM_CLOSE, 0, 0);
    // m_tabs.SetStyle ()
    //SetTabStyle (CTCS_CLOSEBUTTON);
    dockwins::CTabDockingBox <TTraits>  :: OnCreate (uMsg, wParam, lParam, bHandled);
    SetTabStyle (CTCS_CLOSEBUTTON | CTCS_SCROLL | CTCS_BOTTOM| CTCS_FLATEDGE | CTCS_TOOLTIPS);
    bHandled = TRUE;
    return 0;   
  }
  void SetTabStyle (int style)  {
    STYLESTRUCT styleStruct;
    styleStruct.styleOld = :: GetWindowLong (m_tabs.m_hWnd, GWL_STYLE);
    styleStruct.styleNew = styleStruct.styleOld;
    styleStruct.styleNew &=~(CTCS_SCROLL | CTCS_BOTTOM | CTCS_CLOSEBUTTON | CTCS_HOTTRACK 
               | CTCS_FLATEDGE | CTCS_DRAGREARRANGE | CTCS_BOLDSELECTEDTAB | CTCS_TOOLTIPS);
    styleStruct.styleNew |= style;
    SendMessage (m_tabs.m_hWnd, WM_STYLECHANGED, GWL_STYLE, (LPARAM)(& styleStruct));  // see CustomTabCtrl's OnStyleChanged
    :: SetWindowLong (m_tabs.m_hWnd, GWL_STYLE, styleStruct.styleNew);
  }
protected:
  static void SetIndex(DFDOCKRECT* pHdr, int index)
  {
    pHdr->rect.left = index;
    pHdr->rect.right = index;
    pHdr->rect.top = index;
    pHdr->rect.bottom = index;
  }
  static int GetIndex(DFDOCKRECT* pHdr)
  {
    return pHdr->rect.left;
  }
public:
  static HWND CreateInstance(HWND hWnd)
  {
    thisClass* ptr = new thisClass;
    HWND hNewWnd = ptr->Create(hWnd);
    ATLASSERT(hNewWnd);

    if (hNewWnd == NULL)
        delete ptr;

    return hNewWnd;
  }
  virtual bool DockMe(DFDOCKRECT* pHdr)
  {
    if (CDockingBox::IsWindowBox(pHdr->hdr.hBar))
    {
        ATLASSERT(m_wnd.m_hWnd);
        HWND hActiveWnd = m_wnd.m_hWnd;
        int n = m_tabs.GetItemCount();
        ATLASSERT(n >= 2);

        while (n > 0)
        {
            pHdr->hdr.hWnd = GetItemHWND(--n);

            if (pHdr->hdr.hWnd != NULL)
            {
                RemoveWindow(pHdr->hdr.hWnd);
                m_docker.Dock(pHdr);
            }
        }

        pHdr->hdr.hWnd = hActiveWnd;
//            pHdr->hdr.code=DC_ACTIVATE;
        m_docker.Activate(&pHdr->hdr);
        PostMessage(WM_CLOSE);
    }
    else
        m_docker.Dock(pHdr);

    return true;
  }
};

typedef CBoxedDockingWindowTraits < COutlookLikeCaption, CDockingTabContainer<COutlookLikeDockingBoxTraits>,
        WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS >
        COutlookLikeDockingContainerWindowTraits;

typedef CBoxedDockingWindowTraits < COutlookLikeExCaption, CDockingTabContainer<COutlookLikeExDockingBoxTraits>,
        WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS >
        COutlookLikeExDockingContainerWindowTraits;

typedef CBoxedDockingWindowTraits < CVC6LikeCaption, CDockingTabContainer<CVC6LikeDockingBoxTraits>,
        WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS >
        CVC6LikeDockingContainerWindowTraits;

typedef CBoxedDockingWindowTraits < CVC7LikeCaption, CDockingTabContainer<CVC7LikeDockingBoxTraits>,
        WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS >
        CVC7LikeDockingContainerWindowTraits;
typedef CBoxedDockingWindowTraits < CVC7LikeExCaption, CDockingTabContainer<CVC7LikeExDockingBoxTraits>,
        WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS >
        CVC7LikeExDockingContainerWindowTraits;
}//namespace dockwins

# endif // !defined(_CDOCKINGTABCONTAINER_INCLUED_HEADER_CXX_)