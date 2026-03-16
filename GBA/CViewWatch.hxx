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

# if !defined (_CVIEWWATCH_INCLUED_HEADER_CXX_)
#  define _CVIEWWATCH_INCLUED_HEADER_CXX_

# include "CWatchSettings.hxx"

class CViewWatch : public dockwins::CDockingTabChild<CViewWatch>
{
public:
  CMenuHandle m_AttachMenu;
  typedef    dockwins::CDockingTabChild<CViewWatch> baseClass;
  typedef CViewWatch  thisClass;
  CListCtrlREF <CViewWatch>  m_GuiLV_Watch;
  struct gba *m_GBA;
  BOOL m_bAutoUpdate;

  CViewWatch (void): m_GuiLV_Watch (this), m_bAutoUpdate (FALSE) {}
  ~CViewWatch (void) {}

  DECLARE_WND_CLASS_FX(_T ("CViewWatch"), 0, COLOR_WINDOW, IDI_WATCH)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

   void UpdateWatchInfos (BOOL bForceUpdate = FALSE) {
     if (bForceUpdate || m_bAutoUpdate) {
       UpdateWatch ();
     }
   }

   void UpdateWatch (void) {
     // Get Top List.
     int iTopIndex = m_GuiLV_Watch.GetTopIndex ();
     CRect rcClient;
     m_GuiLV_Watch.GetClientRect (rcClient);
     if (iTopIndex >= 0
       && rcClient.Width () > 0
       && rcClient.Height () > 0)
     {
       int iEndIndex = m_GuiLV_Watch.GetItemCount ();
       for (int Id = iTopIndex; Id < iEndIndex; Id++) {
         // Check item' s rect 
         CRect ItemRect;
         m_GuiLV_Watch.GetItemRect (Id, ItemRect, LVIR_BOUNDS);
         if (ItemRect.top >= rcClient.top 
           && ItemRect.top < rcClient.bottom)
         {          
            UINT32 ItemData = m_GuiLV_Watch.GetItemData (Id);
            UpdateSpecItem (Id, ItemData);
         }
         else 
         {
           break;
         }
       }
     }
   }

   void UpdateSpecItem (int iItem, UINT32 ItemData) {
     
      WatchSettings settings (ItemData);
      CString StringContent;
      LVITEM evItem; 
      
      evItem.mask = LVIF_TEXT; 
      evItem.iItem = iItem;
      StringContent.Format (_T ("%07X"), settings.iAddress);
      evItem.iSubItem = 0;  
      evItem.pszText = StringContent.GetBuffer ();  
      m_GuiLV_Watch.SetItem (& evItem);
      
      StringContent.ReleaseBuffer ();
      StringContent.Format (_T ("%d"), settings.iSize);
      evItem.iSubItem = 1;  
      evItem.pszText = StringContent.GetBuffer ();  
      m_GuiLV_Watch.SetItem (& evItem);
      
      StringContent.ReleaseBuffer ();

      DWORD dwAccess = 0;
      if (settings.iSize == 1) {
        dwAccess = gba_fast_rhw (m_GBA, settings.iAddress) & 255;
        StringContent.Format (_T ("%02X"), dwAccess);
      } else if (settings.iSize == 2) {
        dwAccess = gba_fast_rhw (m_GBA, settings.iAddress & -2);
        StringContent.Format (_T ("%04X"), dwAccess);
      } else {
        dwAccess = gba_fast_rhw (m_GBA, settings.iAddress & -2);
        dwAccess |= ((DWORD )gba_fast_rhw (m_GBA, ((settings.iAddress & -2)+2)))<< 16;
        StringContent.Format (_T ("%08X"), dwAccess);
      }
      
      evItem.iSubItem = 2;  
      evItem.pszText = StringContent.GetBuffer ();  
      m_GuiLV_Watch.SetItem (& evItem);
      
      StringContent.ReleaseBuffer ();
      StringContent.Format (_T ("%d"), dwAccess);
      evItem.iSubItem = 3;  
      evItem.pszText = StringContent.GetBuffer ();  
      m_GuiLV_Watch.SetItem (& evItem);
      
      StringContent.ReleaseBuffer ();
      StringContent.Format (_T ("%u"), dwAccess);
      evItem.iSubItem = 4;  
      evItem.pszText = StringContent.GetBuffer ();  
      m_GuiLV_Watch.SetItem (& evItem);
      
      m_GuiLV_Watch.SetItemData (evItem.iItem, ItemData);
   }

  LRESULT OnSysCharMessage (CDnMsg<CViewWatch>* pMsg, BOOL& bHandled) {
    // Mask alt failure warning sound effect
    bHandled= TRUE;
    return 0;
  }

  LRESULT OnScroll (CDnMsg<CViewWatch>* pMsg, BOOL& bHandled) {
    bHandled= FALSE;
    UpdateWatch ();
    return 0;
  }

  void InsertTailItem (UINT32 Item) {

    LVITEM evItem; 
    
    evItem.mask = LVIF_TEXT; 
    evItem.iItem = m_GuiLV_Watch.GetItemCount ();
    
    m_GuiLV_Watch.InsertItem (evItem.iItem, _T ("asd123"));
    UpdateSpecItem (evItem.iItem, Item);
  }

  LRESULT OnSysKeyUpMessage (CDnMsg<CViewWatch>* pMsg, BOOL& bHandled) {
    switch (pMsg->wparam) {
    case 'Q':
    case 'q': // Query Watch Node.
      if (1)
      {
        CWatchSettings watchDialogSettings;
        if ( watchDialogSettings.DoModal () == IDOK) {
          UINT32 uNewItem  = watchDialogSettings.m_WatchSave;
          InsertTailItem (uNewItem);
        }
      }
      m_GuiLV_Watch.SetFocus ();
      break;
    case 'E':
    case 'e':
      {
      int iSelectItem = m_GuiLV_Watch.GetSelectedIndex ();
      if (iSelectItem >= 0)
      {
        UINT32 ItemData=  m_GuiLV_Watch.GetItemData (iSelectItem);
        CWatchSettings watchDialogSettings (TRUE, ItemData);
        if ( watchDialogSettings.DoModal () == IDOK) {
          UINT32 uNewItem  = watchDialogSettings.m_WatchSave;
          UpdateSpecItem (iSelectItem, uNewItem);
        }
        m_GuiLV_Watch.SetFocus ();
      }
      }
      break;
    case 'A':
    case 'a':
      if (m_bAutoUpdate == FALSE) 
        m_GuiLV_Watch.SetExtendedListViewStyle ( m_GuiLV_Watch.GetExtendedListViewStyle () | LVS_EX_GRIDLINES);
      else 
        m_GuiLV_Watch.SetExtendedListViewStyle ( m_GuiLV_Watch.GetExtendedListViewStyle () & ~LVS_EX_GRIDLINES);
      m_bAutoUpdate = !m_bAutoUpdate;
      break;
    case 'D':
    case 'd':
      // delete item. 
      if (m_GuiLV_Watch.GetSelectedIndex () >= 0 ) {
        m_GuiLV_Watch.DeleteItem (m_GuiLV_Watch.GetSelectedIndex ());
      }
    default:
      break;
    }
    return 0;
  }

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
  { 
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_WATCH, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) 
  {
    CRect rc;
    GetClientRect (& rc);
    m_GuiLV_Watch.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) 
  {
    HINSTANCE hInst = _Module.GetModuleInstance ();
    HWND hWindow = m_hWnd;

    HFONT hFont = :: CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, 0, 400, 2, 1, 1, _T ("Microsoft YaHei"));

    m_GuiLV_Watch.Create (*this, rcDefault, _T (""), WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_ALIGNLEFT | LVS_SINGLESEL | LVS_REPORT);
    m_GuiLV_Watch.SetExtendedListViewStyle (LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER | LVS_EX_AUTOSIZECOLUMNS |   LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
    m_GuiLV_Watch.SetFont (hFont);
    
    m_GuiLV_Watch.SetBkColor (0);
    m_GuiLV_Watch.SetTextColor (0xFFFFFF);
    m_GuiLV_Watch.SetTextBkColor (0);
    
    m_GuiLV_Watch.add_msg_map (WM_SYSCHAR, & CViewWatch:: OnSysCharMessage);
    m_GuiLV_Watch.add_msg_map (WM_SYSKEYUP, & CViewWatch:: OnSysKeyUpMessage);
    m_GuiLV_Watch.add_msg_map (WM_HSCROLL, & CViewWatch:: OnScroll);
    m_GuiLV_Watch.add_msg_map (WM_VSCROLL, & CViewWatch:: OnScroll);

    LVCOLUMN vcl;  
    vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;  
    vcl.iSubItem = 0;
    vcl.pszText = _T ("Addr");
    vcl.cx = 72;
    m_GuiLV_Watch.InsertColumn (0, & vcl);
    
    vcl.pszText = _T ("Size");
    vcl.cx = 72;
    m_GuiLV_Watch.InsertColumn (1, & vcl);
    
    vcl.pszText = _T ("Hex");
    vcl.cx = 108;
    m_GuiLV_Watch.InsertColumn (2, & vcl);
    
    vcl.pszText = _T ("Sdec");
    vcl.cx = 108;
    m_GuiLV_Watch.InsertColumn (3, & vcl);
    
    vcl.pszText = _T ("Udec");
    vcl.cx = 108;
    m_GuiLV_Watch.InsertColumn (4, & vcl);

    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_WATCH, MF_DISABLED | MF_BYCOMMAND );
    
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, struct gba *dev) {
    m_GBA = dev;
    Create (parent, CRect (0,0, 480, 402), _T ("Watch"), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS);
    return ;
  }
};

#endif