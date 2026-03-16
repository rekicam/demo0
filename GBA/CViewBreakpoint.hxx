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

// XXX: Bad code better case in show breakpoint 
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVIEWBREAKPOINT_INCLUED_HEADER_CXX_)
#  define _CVIEWBREAKPOINT_INCLUED_HEADER_CXX_

# include "CBreakpointSettings.hxx"

typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0>    CViewBreakpointT_Traits;

class CViewBreakpoint_T : public CWindowImpl <CViewBreakpoint_T, CWindow, CViewBreakpointT_Traits>// public CMessageFilter
{
public :
  //  fixed min size
  //  some settings 
  static  const  int width = 608;
  static  const  int height = 320;

  typedef CWindowImpl<CViewBreakpoint_T,CWindow,CViewBreakpointT_Traits> baseClass;
  typedef CViewBreakpoint_T  thisClass;
  
  CTreeCtrlREF <CViewBreakpoint_T>  m_GuiTR_Breakpoint;
  CListCtrlREF <CViewBreakpoint_T>  m_GuiLV_Breakpoint;
  CSplitterWindow  m_GuiSWinSplitter_View;

  struct gba *m_GBA;
  DWORD m_CurrentRegion; // 0: addr 1:mem 2:interrupt 

  CViewBreakpoint_T (void):m_GuiTR_Breakpoint (this), m_GuiLV_Breakpoint (this) { m_CurrentRegion = 0;  }
 ~CViewBreakpoint_T (void) {}

  LRESULT OnSysCharMessage (CDnMsg<CViewBreakpoint_T>* pMsg, BOOL& bHandled) {
    // Mask alt failure warning sound effect
    bHandled= TRUE;
    return 0;
  }

  LRESULT OnScroll (CDnMsg<CViewBreakpoint_T>* pMsg, BOOL& bHandled) {
    bHandled= FALSE;
    enumShowCurrentBreakPoint2 ();
    return 0;
  }

  BREAKPOINT_REASON getBpReason (void) {
    switch (m_CurrentRegion) {
    case 0:  return BREAKPOINT_REASON_R15;
    case 1:  return BREAKPOINT_REASON_MEM_READ;
    case 2:  return BREAKPOINT_REASON_MEM_WRITE;
    case 3:  return BREAKPOINT_REASON_HARDWARE_INTERRUPT;
    case 4:  return BREAKPOINT_REASON_SOFTWARE_INTERRUPT;
    default: ATLASSERT (FALSE); break;
    }
    return BREAKPOINT_REASON_R15;
  }

  LRESULT OnSysKeyUpMessage (CDnMsg<CViewBreakpoint_T>* pMsg, BOOL& bHandled) {
    switch (pMsg->wparam) {
    case 'Q': case 'q': // Query Breakopint.
      if (pMsg->from == m_GuiTR_Breakpoint)
      {
        CBreakpointSettings bpDialogSettings (getBpReason (), FALSE, NULL);
        if ( bpDialogSettings.DoModal () == IDOK) {
          if (m_CurrentRegion == 0) {
            struct breakpoint *prev_bpnode = gba_get_addr_breakpoint_slot (m_GBA, bpDialogSettings.m_CrreateBP->address);
            gba_set_addr_breakpoint_slot (m_GBA, bpDialogSettings.m_CrreateBP->address, bpDialogSettings.m_CrreateBP);
            if (prev_bpnode != null)
              gba_breakpoint_remove (m_GBA, prev_bpnode);
          }
          gba_breakpoint_insert (m_GBA, bpDialogSettings.m_CrreateBP);
          enumShowCurrentBreakPoint ();
        }
      }
      else 
      {
        int Id = m_GuiLV_Breakpoint.GetSelectedIndex ();
        if (Id >= 0) {
           struct list_chunk *lchunk = (struct list_chunk *) m_GuiLV_Breakpoint.GetItemData (Id);
           CBreakpointSettings bpDialogSettings (getBpReason (), TRUE, (struct breakpoint *)lchunk->dat_ptr);
            if ( bpDialogSettings.DoModal () == IDOK) {
              enumShowCurrentBreakPoint ();
            }
        }
      }
      m_GuiTR_Breakpoint.SetFocus ();
      break;
    case 'D': case 'd': // Delete breakpoint item.
      if (m_GuiLV_Breakpoint == pMsg->from) {
        // delete item. 
        if (m_GuiLV_Breakpoint.GetSelectedIndex () >= 0 ) {
          struct list_chunk *item = (struct list_chunk *) m_GuiLV_Breakpoint.GetItemData (m_GuiLV_Breakpoint.GetSelectedIndex ());
          if (item != NULL) {

            struct breakpoint *dev_bp = (struct breakpoint *) item->dat_ptr;
            switch (m_CurrentRegion) {
            case 0: // address !
              gba_set_addr_breakpoint_slot (m_GBA, dev_bp->address, NULL);
              list_remove_body_  (m_GBA->dbg.addr_bp_list, item);
              break;
            case 1: // memory -read 
              list_remove_body_  (m_GBA->dbg.read_bp_list, item);
              break;
            case 2: // memory write 
              list_remove_body_  (m_GBA->dbg.write_bp_list, item);
              break;
            case 3:
              list_remove_body_  (m_GBA->dbg.int_bp_list , item);
              break;
            default:
              ATLASSERT (FALSE);
              break;
            }

            if (dev_bp != null)
                gba_breakpoint_remove (m_GBA, dev_bp);

            enumShowCurrentBreakPoint ();
          }
        }
      }
    default:
      break;
    }
    return 0;
  }

  DECLARE_WND_CLASS(_T ("CViewBreakpoint_T"))
  
  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
  END_MSG_MAP()

  LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = FALSE;
    NMHDR *pNotifyBase = (NMHDR *) lParam;
    if (pNotifyBase->code == TVN_SELCHANGED
      && pNotifyBase->hwndFrom == m_GuiTR_Breakpoint)
    {
      NMTREEVIEW *pThisNDR = (NMTREEVIEW *) pNotifyBase;
      m_CurrentRegion =m_GuiTR_Breakpoint.GetItemData (  pThisNDR->itemNew.hItem);
      ATLASSERT (m_CurrentRegion <= 3);
      resetListCaption (m_CurrentRegion);
      enumShowCurrentBreakPoint ();
    }
    else if  (pNotifyBase->code == LVN_ITEMCHANGED
      && pNotifyBase->hwndFrom == m_GuiLV_Breakpoint)
    {
      // Check Enable status 
      NMLISTVIEW *pThisNDR = (NMLISTVIEW *) pNotifyBase;
      if (pThisNDR->iItem >= 0) {
        // INDEXTOSTATEIMAGEMASK((fCheck)?2:1)
        int nCheckMask = 2;   // one based index
        int nNoCheckMask = 1;   // one based index
        BOOL bPrevState = pThisNDR->uOldState & INDEXTOSTATEIMAGEMASK (nCheckMask) ? TRUE : FALSE;
        BOOL bChecked= pThisNDR->uNewState & INDEXTOSTATEIMAGEMASK (nCheckMask)  ? TRUE : FALSE;
        if (bPrevState != bChecked) {
          struct list_chunk *p = (struct list_chunk *)m_GuiLV_Breakpoint.GetItemData (pThisNDR->iItem);
          struct breakpoint *node;
          if (p != NULL) {
            if ((node = (struct breakpoint *) p->dat_ptr) != NULL) {
              node->enable = !! bChecked;
            }
          }
        }
      }
    }
    return 0;
  }

  void enumShowCurrentBreakPoint (void) {
      // Clear all item
      int id;
      struct list_chunk *pv;
      CString format;

      // Empty item will myebe cause deadloop.
      if (m_GuiLV_Breakpoint.GetItemCount () > 0)
        m_GuiLV_Breakpoint.DeleteAllItems ();

      LVITEM evItem; 
      evItem.mask = LVIF_TEXT; 

      switch (m_CurrentRegion) {
      case 0: // address !
        LIST_EACH_ (m_GBA->dbg.addr_bp_list, id, pv) {
          struct breakpoint *data = (struct breakpoint *) pv->dat_ptr;
          
          format.Format (_T ("%07X"), data->address);
          m_GuiLV_Breakpoint.InsertItem (id, format.GetBuffer ());
          format.ReleaseBuffer ();

          CString cExpr (data->describe);
          evItem.iSubItem = 1;  
          evItem.iItem = id;
          evItem.pszText = cExpr.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          format.Format (_T ("%d"), data->match_loop);
          evItem.iSubItem = 2;  
          evItem.pszText = format.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          format.Format (_T ("%d"), data->match_expect);
          evItem.iSubItem = 3;  
          evItem.pszText = format.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          m_GuiLV_Breakpoint.SetItemData (id, (DWORD_PTR) pv);
          m_GuiLV_Breakpoint.SetCheckState (id, data->enable);
        }
        break;
      case 1: // memory -read 
      case 2: // memory write 
       LIST_EACH_ ((m_CurrentRegion == 1)? m_GBA->dbg.read_bp_list : m_GBA->dbg.write_bp_list, id, pv) {
          struct breakpoint *data = (struct breakpoint *) pv->dat_ptr;
          format.Format (_T ("%07X"), data->address);
          m_GuiLV_Breakpoint.InsertItem (id, format.GetBuffer ());
          format.ReleaseBuffer ();

          format.Format (_T ("%d"), data->size);
          evItem.iSubItem = 1;  
          evItem.pszText = format.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          CString cExpr (data->describe);
          evItem.iSubItem = 2;  
          evItem.pszText = cExpr.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          format.Format (_T ("%d"), data->match_loop);
          evItem.iSubItem = 3;  
          evItem.pszText = format.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          format.Format (_T ("%d"), data->match_expect);
          evItem.iSubItem = 4;  
          evItem.pszText = format.GetBuffer ();  
          m_GuiLV_Breakpoint.SetItem (& evItem);
          format.ReleaseBuffer ();

          m_GuiLV_Breakpoint.SetItemData (id, (DWORD_PTR) pv);
          m_GuiLV_Breakpoint.SetCheckState (id, data->enable);
        }
        break;
      case 3:
        LIST_EACH_ (m_GBA->dbg.int_bp_list, id, pv) {
          struct breakpoint *data = (struct breakpoint *) pv->dat_ptr;
          
          CString cExpr (data->describe);
          m_GuiLV_Breakpoint.InsertItem (id, cExpr.GetBuffer ());

          m_GuiLV_Breakpoint.SetItemData (id, (DWORD_PTR) pv);
          m_GuiLV_Breakpoint.SetCheckState (id, data->enable);
        }
        break;
      default:
        ATLASSERT (FALSE);
        return ;
      }
  }

  void enumShowCurrentBreakPoint2 (void) {
     int iTopIndex = m_GuiLV_Breakpoint.GetTopIndex ();
     CRect rcClient;
     CString format;
      LVITEM evItem; 
      evItem.mask = LVIF_TEXT; 
     m_GuiLV_Breakpoint.GetClientRect (rcClient);
     if (iTopIndex >= 0
       && rcClient.Width () > 0
       && rcClient.Height () > 0)
     {
       int iEndIndex = m_GuiLV_Breakpoint.GetItemCount ();
       for (int Id = iTopIndex; Id < iEndIndex; Id++) {
         // Check item' s rect 
         CRect ItemRect;
         m_GuiLV_Breakpoint.GetItemRect (Id, ItemRect, LVIR_BOUNDS);
         if (ItemRect.top >= rcClient.top 
           && ItemRect.top < rcClient.bottom)
         {          
            struct list_chunk *lchunk = (struct list_chunk *) m_GuiLV_Breakpoint.GetItemData (Id);
            struct breakpoint *data = (struct breakpoint *) lchunk->dat_ptr;

            evItem.iItem = Id;
            evItem.iSubItem = 0;  
            evItem.pszText = data->enable ? _T ("enable") : _T ("disable");  
            m_GuiLV_Breakpoint.SetItem (& evItem);
            switch (m_CurrentRegion) {
            case 0: // address !
                format.Format (_T ("%07X"), data->address);
                evItem.iSubItem = 0;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                {
                CString cExpr (data->describe);
                evItem.iSubItem = 1;  
                evItem.pszText = cExpr.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                format.Format (_T ("%d"), data->match_loop);
                evItem.iSubItem = 2;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                format.Format (_T ("%d"), data->match_expect);
                evItem.iSubItem = 3;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();
                m_GuiLV_Breakpoint.SetCheckState (Id, data->enable);
                }
              break;
            case 1: // memory -read 
            case 2: // memory write 
                format.Format (_T ("%07X"), data->address);
                evItem.iSubItem = 0;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                format.Format (_T ("%d"), data->size);
                evItem.iSubItem = 1;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();
                {
                CString cExpr (data->describe);
                evItem.iSubItem = 2;  
                evItem.pszText = cExpr.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                format.Format (_T ("%d"), data->match_loop);
                evItem.iSubItem = 3;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();

                format.Format (_T ("%d"), data->match_expect);
                evItem.iSubItem = 4;  
                evItem.pszText = format.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();
                m_GuiLV_Breakpoint.SetCheckState (Id, data->enable);
                }
              break;
            case 3:
                {
                CString cExpr (data->describe);
                evItem.iSubItem = 0;  
                evItem.pszText = cExpr.GetBuffer ();  
                m_GuiLV_Breakpoint.SetItem (& evItem);
                format.ReleaseBuffer ();
                }
              break;
            default:
              ATLASSERT (FALSE);
              return ;
            }

         }
         else 
         {
           break;
         }
       }
     }

  }

  void resetListCaption (DWORD dwBreakpointID = 0) {
    while (m_GuiLV_Breakpoint.DeleteColumn (0) != FALSE)
      continue;

    int pos = 0;
    LVCOLUMN vcl;  
    vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;  
    vcl.iSubItem = 0;
    vcl.pszText = _T ("ADDR");
    vcl.cx = 90;

    switch (dwBreakpointID) {
    case 0: // address !
    case 1: // memory -read 
    case 2: // memory write 
      m_GuiLV_Breakpoint.InsertColumn (pos++, & vcl);
      if (dwBreakpointID != 0) {
        vcl.pszText = _T ("SIZE");
        vcl.cx = 45;
        m_GuiLV_Breakpoint.InsertColumn (pos++, & vcl);
      }
      break;
    case 3: // interrupt 
      break;
    default:
      ATLASSERT (FALSE);
      return;
    }
    vcl.pszText = _T ("EXPR");
    vcl.cx = 250;
    m_GuiLV_Breakpoint.InsertColumn (pos++, & vcl);
    vcl.pszText = _T ("POLL");
    vcl.cx = 50;
    m_GuiLV_Breakpoint.InsertColumn (pos++, & vcl);
    vcl.pszText = _T ("MATCH");
    vcl.cx = 65;
    m_GuiLV_Breakpoint.InsertColumn (pos++, & vcl);
  }

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
    CRect rcWindow;
    HINSTANCE hInst = _Module.GetModuleInstance ();
    HWND hWindow = m_hWnd;

    HFONT hFont = :: CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, 0, 400, 2, 1, 1, _T ("Microsoft YaHei"));

    GetClientRect (rcWindow);
    m_GuiSWinSplitter_View.Create (m_hWnd, rcWindow, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    m_GuiLV_Breakpoint.Create (m_GuiSWinSplitter_View, rcDefault, _T (""), WS_VISIBLE | WS_CHILD  | LVS_ALIGNLEFT | LVS_SINGLESEL | LVS_REPORT, 0);
    m_GuiLV_Breakpoint.SetBkColor (RGB (237,250,254));
    m_GuiLV_Breakpoint.SetExtendedListViewStyle (LVS_EX_HEADERDRAGDROP |  LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_GRIDLINES |   LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

    m_GuiLV_Breakpoint.SetFont (hFont);
    m_GuiTR_Breakpoint .Create (m_GuiSWinSplitter_View, rcDefault, _T (""), WS_VISIBLE | WS_CHILD  | TVS_HASBUTTONS | TVS_HASLINES );
    m_GuiTR_Breakpoint.SetFont (hFont);

    m_GuiLV_Breakpoint.add_msg_map (WM_SYSCHAR, & CViewBreakpoint_T:: OnSysCharMessage);
    m_GuiLV_Breakpoint.add_msg_map (WM_SYSKEYUP, & CViewBreakpoint_T:: OnSysKeyUpMessage);
    m_GuiLV_Breakpoint.add_msg_map (WM_HSCROLL, & CViewBreakpoint_T:: OnScroll);
    m_GuiLV_Breakpoint.add_msg_map (WM_VSCROLL, & CViewBreakpoint_T:: OnScroll);

    m_GuiTR_Breakpoint.add_msg_map (WM_SYSCHAR, & CViewBreakpoint_T:: OnSysCharMessage);
    m_GuiTR_Breakpoint.add_msg_map (WM_SYSKEYUP, & CViewBreakpoint_T:: OnSysKeyUpMessage);

    HTREEITEM hSubItem;
    hSubItem = m_GuiTR_Breakpoint.InsertItem (_T ("Addr"), TVI_ROOT, NULL);
    m_GuiTR_Breakpoint.SetItemData (hSubItem, 0);
    hSubItem = m_GuiTR_Breakpoint.InsertItem (_T ("Read"), TVI_ROOT, hSubItem);
    m_GuiTR_Breakpoint.SetItemData (hSubItem, 1);
    hSubItem = m_GuiTR_Breakpoint.InsertItem (_T ("Write"), TVI_ROOT, hSubItem);
    m_GuiTR_Breakpoint.SetItemData (hSubItem, 2);
    hSubItem = m_GuiTR_Breakpoint.InsertItem (_T ("Int"), TVI_ROOT,hSubItem);
    m_GuiTR_Breakpoint.SetItemData (hSubItem, 3);
    //hSubItem = m_GuiTR_Breakpoint.InsertItem (_T ("Soft-Interrupt"), TVI_ROOT,hSubItem);
    //m_GuiTR_Breakpoint.SetItemData (hSubItem, 4);

    m_GuiSWinSplitter_View.SetSplitterPanes (m_GuiTR_Breakpoint, m_GuiLV_Breakpoint);
    m_GuiSWinSplitter_View.SetSplitterPos (60);
    resetListCaption ();
    return 0;
  }
};

class CViewBreakpoint : public dockwins::CDockingTabChild<CViewBreakpoint>
{
public:
  CMenuHandle m_AttachMenu;
  typedef    dockwins::CDockingTabChild<CViewBreakpoint> baseClass;
  typedef CViewBreakpoint  thisClass;
  CViewBreakpoint_T m_panel_arm7;

  CViewBreakpoint (void) {}
  ~CViewBreakpoint (void) {}

  DECLARE_WND_CLASS_FX(_T ("CViewBreakpoint"), 0, COLOR_WINDOW, IDI_BREAKPOINT)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
  { 
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_BREAKPOINT, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) 
  {
    CRect rc;
    GetClientRect (& rc);
    ::SetWindowPos(m_panel_arm7, NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    ::SetWindowPos(m_panel_arm7.m_GuiSWinSplitter_View, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) 
  {
    CRect rcWindow;
    GetClientRect (rcWindow);
    m_panel_arm7.Create (m_hWnd, rcWindow, _T (""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_BREAKPOINT, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, struct gba *dev) {
    m_panel_arm7.m_GBA = dev;
    Create (parent, CRect (0,0, CViewBreakpoint_T ::width, CViewBreakpoint_T::height), _T ("BREAKPOINT"), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,  WS_EX_TRANSPARENT);
    return ;
  }
};

#endif