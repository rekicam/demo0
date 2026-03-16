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

# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVIEWCOMMENT_INCLUED_HEADER_CXX_)
#  define _CVIEWCOMMENT_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <WTL/atltheme.h>
# include <WTL/atlcrack.h>
# include <tchar.h>

class CViewComment : public dockwins::CDockingTabChild<CViewComment> {
  // Global Settings 
  static  const  COLORREF background_color = 0x6e4e29;
  static  const  COLORREF font_color = 0xFFFFFF;
  static  const  DWORD windowHeight = 260;
  static  const  DWORD windowWidth = 480;

public:
  CMenuHandle m_AttachMenu;
  typedef    dockwins::CDockingTabChild<CViewComment> baseClass;
  typedef CViewComment  thisClass;

  typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,WS_EX_STATICEDGE>    CViewComment_TTraits;

  CViewComment (void) {}
 ~CViewComment (void) {}

  class CViewComment_T:public CWindowImpl<CViewComment_T,CWindow,CViewComment_TTraits> {
  public :
    typedef    CWindowImpl<CViewComment_T,CWindow,CViewComment_TTraits> baseClass;
    typedef CViewComment_T  thisClass;
  
    // Ctrl .
    CRichEditCtrl m_GuiRT_Comment;

    CViewComment_T (void)  {}
   ~CViewComment_T (void) {}
 
    DECLARE_WND_CLASS(_T ("CViewComment_T"))
  
    BEGIN_MSG_MAP(thisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
      SetClassLongPtr (m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR) GetStockObject (NULL_BRUSH)) ;
      HINSTANCE hInst = _Module.GetModuleInstance ();
      HWND hWindow = m_hWnd;

      HFONT hFont = CreateFont(-8, 0, 0, 0, 500, FALSE, FALSE, FALSE, 0, 500, 2, 1, 1, _T ("Press Start 2P"));
    
      // RichEdit.
      m_GuiRT_Comment  = CreateWindowEx(0, CRichEditCtrl::GetWndClassName (), _T(""), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE |  ES_WANTRETURN, 0, 0, 363, 218, *this, (HMENU)0, hInst, 0);
      m_GuiRT_Comment.SetFont (hFont, FALSE);
      m_GuiRT_Comment.SetBackgroundColor (background_color);
      
      CHARFORMAT ch_format;
      ZeroMemory (&ch_format, sizeof(CHARFORMAT));

      m_GuiRT_Comment.GetDefaultCharFormat(ch_format);
      ch_format.cbSize = sizeof(ch_format);
      ch_format.dwEffects &= ~CFE_AUTOCOLOR;
      ch_format.dwMask |= CFM_COLOR;
      ch_format.crTextColor = font_color;

      m_GuiRT_Comment.SetDefaultCharFormat(ch_format);

      PARAFORMAT2 pf;
      ZeroMemory(& pf, sizeof (pf));
      pf.cbSize = sizeof(PARAFORMAT2);
      pf.dwMask |= PFM_LINESPACING;
      pf.bLineSpacingRule =5;
      pf.dyLineSpacing = 24;
      m_GuiRT_Comment.SetSelAll ();
      m_GuiRT_Comment.SetParaFormat(pf);
      m_GuiRT_Comment.SetSelNone ();

      m_GuiRT_Comment.SetWindowText (_T ("moecmks done."));
      return 0;
    }
  } m_panel;
  
  DECLARE_WND_CLASS_FX (_T ("CViewComment"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, 0)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  int AppendText (LPTSTR String) {
    return m_panel.m_GuiRT_Comment.AppendText (String);
  }

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_COMMENT, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // Post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }
  
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rc;
    GetClientRect (& rc);
    ::SetWindowPos(m_panel, NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    ::SetWindowPos(m_panel.m_GuiRT_Comment, NULL, rc.left, rc.top, rc.Width (), rc.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    m_panel.Create (m_hWnd, rcDefault);
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_COMMENT, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent) {
    Create (parent, CRect (0,0, windowWidth, windowHeight), _T ("Comment"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
  }
};

# endif 