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

# if !defined (_CVIEWPALETTE_INCLUED_HEADER_CXX_)
#  define _CVIEWPALETTE_INCLUED_HEADER_CXX_

// @BUG: Hit DoPaint in WM_MOUSEMOVE in other ctrl.
// TODO: reduce WM_PAINT item count
struct CViewPalette_T : public CDialogImpl<CViewPalette_T>
                           , public CScrollImpl<CViewPalette_T>
{
  static const int SQU_SIZE = 9;
  static const int mesh_clr = 0xff7e30;
  static const int mesh_lock = 0xFF;

  enum { IDD = IDD_GPU_PALETTE };

  typedef CDialogImpl<CViewPalette_T> baseClass;
  typedef CScrollImpl<CViewPalette_T> baseClass2;
  typedef CViewPalette_T thisClass;

  CListCtrlREF<CViewPalette_T> m_GuiLV_BGCanvas;
  CListCtrlREF<CViewPalette_T> m_GuiLV_SPCanvas;
  CStatic m_GuiST_Pos;
  CStatic m_GuiST_Addr;
  CStatic m_GuiST_Idx;
  CStatic m_GuiST_BGR;
  CButton m_GuiBT_Update;
  CButton m_GuiCK_Lock;
  CButton m_GuiCK_QFlush;
  CStatic m_GuiST_B;
  CStatic m_GuiST_G;
  CStatic m_GuiST_R;
  CStatic m_GuiPT_B;
  CStatic m_GuiPT_G;
  CStatic m_GuiPT_R;
  CStatic m_GuiPT_Pixout;

  gba *m_GBA;

  CViewPalette_T (void): m_GuiLV_BGCanvas(this), 
    m_GuiLV_SPCanvas (this),
    m_bTrackSp (FALSE),
  m_ptTrack (0, 0),
  m_dwColMesh (mesh_clr), 
  m_dwColTrack ( mesh_lock)  {}
  ~CViewPalette_T (void) {}

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
    UpdateGBAItemOut (TRUE);
  }

  void AdjustTrackPos (CPoint &pt) {
    static const int iVec = SQU_SIZE + 1;
    if (pt.x < 0)
      pt.x = 0;
    if (pt.y < 0)
      pt.y = 0;

    pt.x /= iVec;
    pt.y /= iVec;

    if (pt.x > 15)
      pt.x = 15;
    if (pt.y > 15)
      pt.y = 15;
  }

  void UpdatePaneInfos (BOOL bSpritePane, const CPoint &pt) {

    int iIdx = pt.y * 16 + pt.x;
    int iPal = iIdx + (bSpritePane ? 256 : 0);
    int iPal2 = iPal  * 2;

    uint16_t tPal = *(uint16_t *)& m_GBA->gpu.palette[iPal2];
    uint16_t tPal2 = *(uint16_t *)& m_GBA->gpu.palette2[iPal2];
    uint32_t tPal3 = *(uint32_t *)& m_GBA->gpu.palette3[iPal2];
    uint32_t tPal4 = *(uint32_t *)& m_GBA->gpu.palette4[iPal2];

    uint32_t b = tPal >> 10 & 0x1F;
    uint32_t g = tPal >> 5 & 0x1F;
    uint32_t r = tPal >> 0 & 0x1F;
    
    // Update Control Item. 
    CSetWindowText (m_GuiST_Addr, _T ("Addr:%03X"), iPal2);
    CSetWindowText (m_GuiST_Pos, _T ("Pos:%X, %X"), pt.x, pt.y);
    CSetWindowText (m_GuiST_BGR, _T ("BGR:%04X"), tPal);
    CSetWindowText (m_GuiST_Idx, _T ("Idx:%02X"), iIdx);

    CSetWindowText (m_GuiST_B, _T ("B:%02X"), tPal2 & 0x1F);
    CSetWindowText (m_GuiST_G, _T ("G:%02X"), tPal2 >> 5 & 0x1F);
    CSetWindowText (m_GuiST_R, _T ("R:%02X"), tPal2 >>10 & 0x1F);

    CClientDC dc_b (m_GuiPT_B);
    CClientDC dc_g (m_GuiPT_G);
    CClientDC dc_r (m_GuiPT_R);
    CClientDC dc_out (m_GuiPT_Pixout);
    CRect rc_b;
    CRect rc_g; 
    CRect rc_r;
    CRect rc_out;

    m_GuiPT_B.GetClientRect (rc_b);
    m_GuiPT_G.GetClientRect (rc_g);
    m_GuiPT_R.GetClientRect (rc_r);
    m_GuiPT_Pixout.GetClientRect (rc_out);

    dc_b.FillSolidRect (rc_b, tPal3 & 0xFF0000);
    dc_g.FillSolidRect (rc_g, tPal3 & 0x00FF00);
    dc_r.FillSolidRect (rc_r, tPal3 & 0x0000FF);
    dc_out.FillSolidRect (rc_out, tPal3);
  }

  void UpdatePalette (BOOL bSpritePane, CPoint &ptTrack, HDC attachDC = NULL) {

    CClientDC dc_bg (m_GuiLV_BGCanvas);
    CClientDC dc_sp (m_GuiLV_SPCanvas);

    uint32_t *bg_palette = (uint32_t *)& m_GBA->gpu.palette3[0];
    uint32_t *sp_palette = & bg_palette[256];

    // Draw GBA Palette
    for (int y = 0; y != 16; y++)  {
      for (int x = 0; x != 16; x++) {
        int xPos = x * (SQU_SIZE + 1) + 1;
        int yPos = y * (SQU_SIZE + 1) + 1;
        int iPal = (y << 4) +x;
        dc_bg.FillSolidRect (xPos, yPos, SQU_SIZE, SQU_SIZE, bg_palette[iPal]);
        dc_sp.FillSolidRect (xPos, yPos, SQU_SIZE, SQU_SIZE, sp_palette[iPal]);
      }
    }
    // Draw Mesh colour.
    for (int Id = 0; Id != 17; Id++) {
      int iVec = Id * (SQU_SIZE + 1);
      static const int iVecTotal = (SQU_SIZE + 1) * 16 + 1;

      // draw hori.
      dc_bg.FillSolidRect (0, iVec, iVecTotal, 1, m_dwColMesh);
      dc_sp.FillSolidRect (0, iVec, iVecTotal, 1, m_dwColMesh);
      
      // draw vert 
      dc_bg.FillSolidRect (iVec, 0, 1, iVecTotal, m_dwColMesh);
      dc_sp.FillSolidRect (iVec, 0, 1, iVecTotal, m_dwColMesh);
    }
    // Mark track color 
    int xPos = ptTrack.x * (SQU_SIZE + 1);
    int yPos = ptTrack.y * (SQU_SIZE + 1);
    // draw mesh col.
    CClientDC &dc = bSpritePane ? dc_sp :dc_bg;
    dc.FillSolidRect (xPos, yPos, SQU_SIZE + 2, 1, m_dwColTrack);
    dc.FillSolidRect (xPos, yPos + SQU_SIZE + 1, SQU_SIZE + 2, 1, m_dwColTrack);
    dc.FillSolidRect (xPos, yPos, 1, SQU_SIZE + 2, m_dwColTrack);
    dc.FillSolidRect (xPos + SQU_SIZE + 1, yPos, 1, SQU_SIZE + 2, m_dwColTrack);
  }
  
  LRESULT OnColSettings (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) { 
    // Pick new color .
    PDWORD pdwCol = NULL;
    if (wID == IDBT_PALETTE_GRID_COL)
      pdwCol = & m_dwColMesh;
    else if (wID == IDBT_PALETTE_LOCK_COL)
      pdwCol = & m_dwColTrack;
    ATLASSERT (pdwCol != NULL);

    CColorDialog col_dlg (* pdwCol);
    if (col_dlg.DoModal () == IDOK) {
      * pdwCol =  col_dlg.GetColor ();
      // Update item output.
      UpdateGBAItemOut (TRUE);
      bHandled = TRUE;
    } else {
      bHandled = FALSE;
    }
    return 0;
  }

  LRESULT OnUpdate (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) { 
    UpdateGBAItemOut (TRUE);
    bHandled = TRUE;
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
    m_GuiLV_BGCanvas.SubclassWindow (GetDlgItem (IDLV_PALETTE_BG));
    m_GuiLV_BGCanvas.add_msg_map (WM_MOUSEMOVE, & CViewPalette_T::OnLockStuff);
    m_GuiLV_BGCanvas.add_msg_map (WM_PAINT, & CViewPalette_T::OnPaint);
    m_GuiLV_BGCanvas.add_msg_map (WM_LBUTTONUP, & CViewPalette_T::OnLockStuff);
    m_GuiLV_SPCanvas.SubclassWindow (GetDlgItem (IDLV_PALETTE_SP));
    m_GuiLV_SPCanvas.add_msg_map (WM_MOUSEMOVE, & CViewPalette_T::OnLockStuff);
    m_GuiLV_SPCanvas.add_msg_map (WM_PAINT, & CViewPalette_T::OnPaint);
    m_GuiLV_SPCanvas.add_msg_map (WM_LBUTTONUP, & CViewPalette_T::OnLockStuff);

    m_GuiST_Pos = GetDlgItem (IDST_PALETTE_POS);
    m_GuiST_Addr = GetDlgItem (IDST_PALETTE_ADDR);
    m_GuiST_Idx = GetDlgItem (IDST_PALETTE_IDX);
    m_GuiST_BGR = GetDlgItem (IDST_PALETTE_BGR);
    m_GuiBT_Update = GetDlgItem (IDBT_PALETTE_UPDATE);
    m_GuiCK_Lock = GetDlgItem (IDCK_PALETTE_LOCK);
    m_GuiCK_QFlush = GetDlgItem (IDCK_PALETTE_QFLUSH);
    m_GuiST_B = GetDlgItem (IDST_PALETTE_B);
    m_GuiST_G = GetDlgItem (IDST_PALETTE_G);
    m_GuiST_R = GetDlgItem (IDST_PALETTE_R);
    m_GuiPT_B = GetDlgItem (IDPT_PALETTE_B);
    m_GuiPT_G = GetDlgItem (IDPT_PALETTE_G);
    m_GuiPT_R = GetDlgItem (IDPT_PALETTE_R);
    m_GuiPT_Pixout = GetDlgItem (IDPT_PALETTE_PIXOUT);

    ModifyStyleEx (0, WS_EX_COMPOSITED | WS_EX_TRANSPARENT);
    bHandled = FALSE;
    return 0;
  }
  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE, HDC attachDC = NULL) {

    // Draw BG Palette Pixel.
    if (m_GuiCK_QFlush.GetCheck () != BST_CHECKED
      && bForceUpdate == FALSE)
      return ;
    if (IsWindowVisible () == FALSE
      || isClientRectEmpty  (*this) != FALSE)
      return ;

    UpdatePalette (m_bTrackSp, m_ptTrack, attachDC);
    UpdatePaneInfos (m_bTrackSp, m_ptTrack);
  }

  LRESULT OnPaint (CDnMsg<CViewPalette_T>* pMsg, BOOL& bHandled) {
    CPaintDC dc (pMsg->from);

    UpdateGBAItemOut (TRUE);
    bHandled= TRUE;
    return 0;
  }
  LRESULT OnPaintBG (CDnMsg<CViewPalette_T>* pMsg, BOOL& bHandled) {
    return 0;
  }
  LRESULT OnPaintSP (CDnMsg<CViewPalette_T>* pMsg, BOOL& bHandled) {
    return 0;
  }

  LRESULT OnLockStuff (CDnMsg<CViewPalette_T>* pMsg, BOOL& bHandled) {

    HWND curWindow = pMsg->from;
    if (curWindow == m_GuiLV_BGCanvas
      || curWindow == m_GuiLV_SPCanvas)
    {
      BOOL bLockCheck = m_GuiCK_Lock.GetCheck () == BST_CHECKED;
      if ( (bLockCheck
        && pMsg->msg == WM_LBUTTONUP)
        ||  (bLockCheck == FALSE
        && pMsg->msg == WM_MOUSEMOVE) )
      {
        // Update mouse pos infos. 
        m_ptTrack.x = LOWORD (pMsg->lparam); 
        m_ptTrack.y = HIWORD (pMsg->lparam);

        AdjustTrackPos (m_ptTrack);
        if (curWindow == m_GuiLV_BGCanvas)
          m_bTrackSp = FALSE;
        else if (curWindow == m_GuiLV_SPCanvas)
          m_bTrackSp = TRUE;
        else 
          ATLASSERT (FALSE);

        UpdatePalette (m_bTrackSp, m_ptTrack);
        UpdatePaneInfos (m_bTrackSp, m_ptTrack);
        bHandled= TRUE;
        return 0;
      }
    }
    bHandled = FALSE;
    return 1;
  }

  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER (WM_INITDIALOG, OnCreate)
    COMMAND_ID_HANDLER (IDBT_PALETTE_UPDATE, OnUpdate)
    COMMAND_ID_HANDLER (IDBT_PALETTE_GRID_COL, OnColSettings)
    COMMAND_ID_HANDLER (IDBT_PALETTE_LOCK_COL, OnColSettings)
    CHAIN_MSG_MAP(baseClass2)
  END_MSG_MAP()

  BOOL m_bTrackSp;
  CPoint m_ptTrack;
  DWORD m_dwColMesh;
  DWORD m_dwColTrack;
};

class CViewPalette : public dockwins::CDockingTabChild<CViewPalette> {
public:
  CMenuHandle m_AttachMenu;
  CViewPalette_T m_ContainerOverview;

  typedef    dockwins::CDockingTabChild<CViewPalette> baseClass;
  typedef CViewPalette  thisClass;

  CViewPalette (void) {}
 ~CViewPalette (void) {}

  DECLARE_WND_CLASS_FX (_T ("CViewPalette"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, IDI_PALETTE)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
    m_AttachMenu.EnableMenuItem (ID_GPU_PALETTERVIEWER, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // Post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }
  
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rc;
    GetClientRect (& rc);
     ::SetWindowPos(m_ContainerOverview, NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rcClient;
    m_ContainerOverview.Create (*this);
    // m_ContainerOverview.ModifyStyleEx (0, WS_EX_COMPOSITED, 0); // Set double buffer dialog.
    m_ContainerOverview.GetClientRect (rcClient);
    ResizeClient (rcClient.Width (), rcClient.Height ());
    m_ContainerOverview.SetScrollSize (rcClient.Width (), rcClient.Height ());
    m_ContainerOverview.DoSize (rcClient.Width (), rcClient.Height ());
    m_ContainerOverview.ShowWindow (SW_SHOWNORMAL);
    
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_GPU_PALETTERVIEWER, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, gba *attachGBA) {
    m_ContainerOverview.m_GBA = attachGBA;
    Create (parent, CRect (0,0, 120, 96), _T ("Palette"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
  }
};

# endif 