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

# if !defined (_CVIEWOUTPUT_INCLUED_HEADER_CXX_)
#  define _CVIEWOUTPUT_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <tchar.h>
# include "..\resource.h"

class CViewOutput : public dockwins::CDockingTabChild<CViewOutput> {
  // Global Settings 
  static  const  COLORREF background_color = 0;
  static  const  COLORREF font_color = 0xFFFFFF;
  static  const  DWORD windowHeight = 260;
  static  const  DWORD windowWidth = 480;

public:
  CMenuHandle m_AttachMenu;
  typedef    dockwins::CDockingTabChild<CViewOutput> baseClass;
  typedef CViewOutput  thisClass;

  typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0>    CViewOutput_T_Traits;

  CViewOutput (void) {}
 ~CViewOutput (void) {}

  class CViewOutput_T:public CWindowImpl<CViewOutput_T,CWindow,CViewOutput_T_Traits> {
  public :
    typedef    CWindowImpl<CViewOutput_T,CWindow,CViewOutput_T_Traits> baseClass;
    typedef CViewOutput_T  thisClass;
  
    // Ctrl .
    CRichEditCtrl m_GuiRT_Output;

    CViewOutput_T (void)  {}
   ~CViewOutput_T (void) {}
 
    DECLARE_WND_CLASS(_T ("CViewOutput_T"))
  
    BEGIN_MSG_MAP(thisClass)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
      HINSTANCE hInst = _Module.GetModuleInstance ();
      HWND hWindow = m_hWnd;

      HFONT hFont = CreateFont (-8, 0, 0, 0, 500, FALSE, FALSE, FALSE, 0, 500, 2, 1, 1, _T ("Press Start 2P"));
    
      // RichEdit.
      m_GuiRT_Output  = CreateWindowEx (0, CRichEditCtrl::GetWndClassName (), _T(""), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE |  ES_WANTRETURN, 0, 0, 363, 218, *this, (HMENU)0, hInst, 0);
      m_GuiRT_Output.SetFont (hFont, FALSE);
      m_GuiRT_Output.SetBackgroundColor (background_color);
    
      CHARFORMAT ch_format;
      ZeroMemory (&ch_format, sizeof(CHARFORMAT));

      m_GuiRT_Output.GetDefaultCharFormat(ch_format);
      ch_format.cbSize = sizeof(ch_format);
      ch_format.dwEffects &= ~CFE_AUTOCOLOR;
      ch_format.dwMask |= CFM_COLOR;
      ch_format.crTextColor = font_color;

      m_GuiRT_Output.SetDefaultCharFormat(ch_format);

      PARAFORMAT2 pf;
      ZeroMemory(& pf, sizeof (pf));
      pf.cbSize = sizeof (PARAFORMAT2);
      pf.dwMask |= PFM_LINESPACING;
      pf.bLineSpacingRule =5;
      pf.dyLineSpacing = 24;
      m_GuiRT_Output.SetSelAll ();
      m_GuiRT_Output.SetParaFormat(pf);
      m_GuiRT_Output.SetSelNone ();
      return 0;
    }
  } m_panel;
  
  DECLARE_WND_CLASS_FX (_T ("CViewOutput"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, IDI_OUTPUT)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  int AppendText (LPTSTR String) {
    return m_panel.m_GuiRT_Output.AppendText (String);
  }

  int __cdecl AppendTextV (LPTSTR format, ...) {

    CString CStr;
    va_list argList;
	  va_start (argList, format);
	  CStr.FormatV (format, argList);
	  va_end (argList);

    LPTSTR pszBuffer = CStr.GetBuffer ();
    int ret = AppendText (pszBuffer);

    CStr.ReleaseBuffer ();
    return ret;
  }

  void WrapToTail (void) {

    SendMessage (m_panel.m_GuiRT_Output.m_hWnd, WM_VSCROLL, SB_BOTTOM, 0);
  }

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_OUTPUT, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // Post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }
  
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rc;
    GetClientRect (& rc);
    ::SetWindowPos(m_panel, NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    ::SetWindowPos(m_panel.m_GuiRT_Output, NULL, rc.left, rc.top, rc.Width (), rc.Height (), SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    m_panel.Create (m_hWnd, rcDefault, _T (""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TRANSPARENT);
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_OUTPUT, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent) {
    Create (parent, CRect (0,0, windowWidth, windowHeight), _T ("Output"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  WS_EX_TRANSPARENT);
  }
};

# endif 