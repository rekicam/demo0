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

// TODO: More case, and repeat code, rot mode, and mode 3,  mode 4, mode 5 draw status
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVIEWNAMETABLE_INCLUED_HEADER_CXX_)
#  define _CVIEWNAMETABLE_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <tchar.h>
# include "..\resource.h"
# include "..\GBA\gba.h"
# include <d3d9.h>
# include <algorithm>

template <class WM_MouseMoveHOOK_This , DWORD t_dwNametableID, DWORD t_dwSquSize> /* t_dwNametableID := 0~3 */
struct CViewNametable_T :  
  public CDirectWrite16WindowImpl < CViewNametable_T<WM_MouseMoveHOOK_This, t_dwNametableID, t_dwSquSize>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwSquSize, 
                                    t_dwSquSize >{
  
  typedef CDirectWrite16WindowImpl < CViewNametable_T<WM_MouseMoveHOOK_This, t_dwNametableID, t_dwSquSize>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwSquSize, 
                                    t_dwSquSize > baseClass;
  typedef CViewNametable_T thisClass;
  
  DECLARE_WND_CLASS(_T ("CViewNametable_T"))

  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER (WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER (WM_LBUTTONUP, OnMouseMove)
    CHAIN_MSG_MAP(baseClass)
  END_MSG_MAP()

  WM_MouseMoveHOOK_This *m_pWMMH_this;

  CViewNametable_T (WM_MouseMoveHOOK_This *this_t): m_pWMMH_this (this_t) { }
  ~CViewNametable_T (void) {}

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
    m_pWMMH_this->UpdateGBAItemOut (TRUE);
  }

  LRESULT OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
    m_pWMMH_this->OnMouseMove (uMsg, wParam, lParam, bHandled);
    return 0;
  }
};

template <class WM_CommandHOOK_This>
struct CViewNametable_T3 : public CDialogImpl<CViewNametable_T3<WM_CommandHOOK_This>>
                           , public CScrollImpl<CViewNametable_T3<WM_CommandHOOK_This>>
{
  enum { IDD = IDD_GPU_NAMETABLE };

  typedef CDialogImpl<CViewNametable_T3<WM_CommandHOOK_This>> baseClass;
  typedef CScrollImpl<CViewNametable_T3<WM_CommandHOOK_This>> baseClass2;
  typedef CViewNametable_T3 thisClass;
  
  WM_CommandHOOK_This *pTThis_;
  CViewNametable_T3 (WM_CommandHOOK_This *T) : pTThis_ (T) {}

  void DoPaint(CDCHandle /*dc*/) {
    InvalidateRect (NULL, TRUE);
  }

  // Set resize class. 
  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 

    CRect rcClient;
    GetClientRect (rcClient);
    SetScrollSize (rcClient.Width (), rcClient.Height ());

    bHandled = FALSE;
    return 0;
  }

  LRESULT OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
    pTThis_->OnCommand (uMsg, wParam, lParam, bHandled);
    return 0;
  }

  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER (WM_INITDIALOG, OnCreate)
    MESSAGE_HANDLER (WM_COMMAND, OnCommand)
    CHAIN_MSG_MAP(baseClass2)
  END_MSG_MAP()
};

template <int t_iMenuId, DWORD t_dwNametableID, DWORD t_dwSquSize>
class CViewNametable : public dockwins::CDockingTabChild<CViewNametable<t_iMenuId, t_dwNametableID, t_dwSquSize>> {
  static const short scroll_rect_line = 0x1f << 0;
  static const short win0_rect_line = 0x1f << 5;
  static const short win1_rect_line = 0x1f << 10;
  static const short mesh_edge_line = 0x7fff;
  static const short shadow_rect_line = 0x1f;
public:
  gba *m_GBA;
  DWORD m_dwClrScrAffine;
  DWORD m_dwClrWin0;
  DWORD m_dwClrWin1;
  DWORD m_dwClrGrid;
  DWORD m_dwClrShadow;
  DWORD m_dwMode;
  DWORD m_dwSize;
  CPoint m_ptTrack;
  CSize m_szScreen;
  CMenuHandle m_AttachMenu;
  CViewNametable_T<CViewNametable, t_dwNametableID, t_dwSquSize> m_ContainerOverview;
  CViewNametable_T3<CViewNametable> m_ContainerInfosPane;
  CSplitterWindowT<false> m_GuiSWinSplitter_View;

  typedef    dockwins::CDockingTabChild<CViewNametable<t_iMenuId, t_dwNametableID, t_dwSquSize>> baseClass;
  typedef CViewNametable  thisClass;

  CViewNametable (void) : 
      m_ContainerOverview (this), 
      m_ContainerInfosPane (this), 
      m_ptTrack (0, 0), 
      m_dwSize (9), 
      m_dwMode (7),
      m_dwClrScrAffine ( scroll_rect_line),
      m_dwClrWin0 (win0_rect_line),
      m_dwClrWin1 (win1_rect_line),
      m_dwClrGrid (mesh_edge_line),
      m_dwClrShadow (shadow_rect_line),
      m_szScreen (256, 256) {}
 ~CViewNametable (void) {}

  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE, BOOL bForceUpdateNametableInfos = FALSE) {
    if (isClientRectEmpty (*this))
      return ;
    if (CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_QFLUSH)).GetCheck () == BST_CHECKED
      || bForceUpdate != FALSE)
    {
      // Draw nametable pixel.
      CRect rcClient;
      DWORD vmode = gba_fast_rhw (m_GBA, 0x4000000) & 7;
      DWORD vsize = gba_fast_rhw (m_GBA, 0x4000008 + (t_dwNametableID << 1)) >> 14 & 3;
      BOOL bGridCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_GRID)).GetCheck () == BST_CHECKED;
      BOOL bLock =  CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_LOCK)).GetCheck () == BST_CHECKED;
      BOOL bRect =  CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_RECT)).GetCheck () == BST_CHECKED;
      DWORD x, y;

      m_ContainerOverview.GetClientRect (& rcClient);

      if (rcClient.Width () < 0
        || rcClient.Height () < 0)
        return ;
      if (vmode != m_dwMode
        || vsize != m_dwSize
        || bForceUpdateNametableInfos != FALSE)
      {
        m_dwSize = vsize;
        m_dwMode = vmode;

        // Reset screen buffer size. 
        DWORD vimask = vsize;

        if (t_dwNametableID >= 2
          && vmode >= 1
          && vmode <= 2)
            vimask |= 4;

        if (vmode >= 3) {
          if (t_dwNametableID == 2) {
            if (vmode == 5) {
              m_szScreen.cx = 160;
              m_szScreen.cy = 128;
            } else {
              m_szScreen.cx = 240;
              m_szScreen.cy = 160;
            }
          }
        } else {
          switch (vimask) {
          case 0:
          case 5:
            m_szScreen.cy = 
            m_szScreen.cx = 256;
            break;
          case 1:
            m_szScreen.cx = 512;
            m_szScreen.cy = 256;
            break;
          case 2:
            m_szScreen.cx = 256;
            m_szScreen.cy = 512;
            break;
          case 6:
          case 3:
            m_szScreen.cy = 
            m_szScreen.cx = 512;
            break;
          case 4:
            m_szScreen.cy = 
            m_szScreen.cx = 128;
            break;
          case 7:
            m_szScreen.cy = 
            m_szScreen.cx = 1024;
            break;
          }
        }
        // Reset screen size, pos.
        x  = m_szScreen.cx;
        y = m_szScreen.cy;
        
        if (bGridCheck != FALSE) {
          x += m_szScreen.cx/8 +1;
          y += m_szScreen.cy/8 +1;
        }  
        POINT &ptRef = m_ContainerOverview. m_ptOffset;
        CPoint pt = m_ContainerOverview. m_ptOffset;

        m_ContainerOverview.  SetScrollSize (x, y);
        m_ContainerOverview.  SetScrollPage (rcClient.Width (), rcClient.Height ());

        ptRef = pt;
        if ( ptRef.x >= x) {
          if (bGridCheck != FALSE)
            ptRef.x = x - 1 - 9;
          else 
            ptRef.x = x - 8;
        }
        if ( ptRef.y >= x) {
          if (bGridCheck != FALSE)
            ptRef.y = y - 1 - 9;
          else 
            ptRef.y = y - 8;
        }
        m_ContainerOverview. SetScrollOffset ( ptRef.x,  ptRef.y);
      }
        // Draw Item. 
      switch (vmode) {
      case 0:
        break;
      case 1:
        if (t_dwNametableID == 3)
          goto NoshowServices;
        break;
      case 2:
        if (t_dwNametableID < 2)
          goto NoshowServices;
        break;
      case 3:
      case 4:
      case 5:
        if (t_dwNametableID != 2)
          goto NoshowServices;
      default:
        break;
      }
      int iOffsetSpace = !! bGridCheck;
      int iElemVectorX = rcClient.Width () / (8 + iOffsetSpace) + 2;
      int iElemVectorY = rcClient.Height () / (8 + iOffsetSpace) + 2;
      int iElemStartX;
      int iElemStartY;
      if (bGridCheck != FALSE) {
        iElemStartX =  m_ContainerOverview. m_ptOffset.x  /9 ;
        iElemStartY =  m_ContainerOverview. m_ptOffset.y /9;
        // 0  1... 8    9  10... 17 18
        // |              |                | 
      } else {
        iElemStartX =  m_ContainerOverview. m_ptOffset.x >> 3;
        iElemStartY =  m_ContainerOverview. m_ptOffset.y >> 3;
      }
      if ( (iElemStartX + iElemVectorX) > m_szScreen.cx / 8) 
        iElemVectorX = m_szScreen.cx / 8 - iElemStartX;
      if ( (iElemStartY + iElemVectorY) > m_szScreen.cy / 8) 
        iElemVectorY = m_szScreen.cy / 8 - iElemStartY;

      BOOL bLockRect = FALSE;
      CPoint ptLockStart;

      if (m_ptTrack.x >= iElemStartX
        && m_ptTrack.x < iElemStartX + iElemVectorX
        && m_ptTrack.y >= iElemStartY
        && m_ptTrack.y < iElemStartY + iElemVectorY) {
        bLockRect = TRUE;
        if (bGridCheck != FALSE) {
          ptLockStart.x = (m_ptTrack.x - iElemStartX) * 9 + 1;
          ptLockStart.y = (m_ptTrack.y - iElemStartY) * 9 + 1;
        } else {
          ptLockStart.x = (m_ptTrack.x - iElemStartX) * 8;
          ptLockStart.y = (m_ptTrack.y - iElemStartY) * 8;
        }
      }

    // render tile and nametable.
    uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);
    uint16_t bg_ctl =  gba_fast_rhw (m_GBA, 0x4000008 + (t_dwNametableID << 1));

    DirectWrite16 direct_write;
    BOOL sig = m_ContainerOverview.LockSurface (direct_write);
    ATLASSERT (sig != FALSE);

    uint16_t *pal256 = (uint16_t *) & m_GBA->gpu.palette2[0];
    switch (master_ctl & 7) {
    case 1: // Mode 1
      if (t_dwNametableID < 2)
    case 0:
        {
          // Render std tile.
          uint16_t chr_base_block = (bg_ctl >> 2 & 3) * 0x4000;
          uint16_t nt_base_block = (bg_ctl >> 8 & 31) * 0x800;
          uint16_t nt_bank[4] = { 0, 0, 0, 0 };

          // Get VRAM Mapper Type.
          switch (bg_ctl & 0xC000) {
          case 0x4000:
            nt_bank[1] = 
            nt_bank[3] = 0x0800;   
            break;
          case 0x8000:  
            nt_bank[2] = 
            nt_bank[3] = 0x0800;
            break;
          case 0xC000:
            nt_bank[1] = 0x0800;
            nt_bank[2] = 0x1000;
            nt_bank[3] = 0x1800;
          default:
            break;
          }
          nt_bank[0] += nt_base_block;
          nt_bank[1] += nt_base_block;
          nt_bank[2] += nt_base_block;
          nt_bank[3] += nt_base_block;

          if (bg_ctl & 0x80) {
            // clr 256
            for (int iY = 0; iY != iElemVectorY; iY++) {
              for (int iX = 0; iX != iElemVectorX; iX++) {
                int iTileX = iElemStartX + iX;
                int iTileY = iElemStartY + iY;
                uintptr_t hori_bank = iTileX >> 5 & 1;
                uintptr_t vert_bank = iTileY >> 5 & 1;
                uintptr_t offx_page = iTileX & 31;
                uintptr_t offy_page = iTileY & 31;
                uintptr_t addr_base =  nt_bank[(vert_bank<<1)+hori_bank];
                uintptr_t addr_total = addr_base + (offy_page << 6) + (offx_page << 1);
                uint16_t tile_attr =  * (uint16_t *) &m_GBA->gpu.vram[addr_total & 0xFFFF];
                uint8_t *chr64b = &m_GBA->gpu.vram[chr_base_block + (tile_attr & 1023) * 64 & 0xFFFF];
                intptr_t y_vec = 8;
                uint16_t *ptr16;

                if (bGridCheck == FALSE)
                  ptr16  = &  direct_write.vptr[direct_write.vpitch16 * iY * 8 + iX * 8];
                else {
                  ptr16  = &  direct_write.vptr[ (direct_write.vpitch16 * iY * 9 +direct_write.vpitch16)  + iX * 9 + 1];
                  uint16_t *draw_mesh = ptr16 - 1 - direct_write.vpitch16;
                  for (int Id =0; Id != 2; Id++)
                    for (int x = 0; x != 10; x++)
                      draw_mesh[Id * 9 * direct_write.vpitch16 + x] =  0;

                  for (int Id =0; Id != 2; Id++)
                    for (int x = 0; x != 10; x++)
                      draw_mesh[Id *  9 + x * direct_write.vpitch16] =  0;
                }

                if (  (tile_attr & 0x800) ){
                  y_vec = -8;
                  chr64b = & chr64b[56];
                }
                if (tile_attr & 0x400) {
                  for (int u = 0; u != 8; u++) {
                    ptr16[0] = pal256[chr64b[7]];
                    ptr16[1] = pal256[chr64b[6]];
                    ptr16[2] = pal256[chr64b[5]];
                    ptr16[3] = pal256[chr64b[4]];
                    ptr16[4] = pal256[chr64b[3]];
                    ptr16[5] = pal256[chr64b[2]];
                    ptr16[6] = pal256[chr64b[1]];
                    ptr16[7] = pal256[chr64b[0]];
                    ptr16 += direct_write.vpitch16;
                    chr64b += y_vec;
                  }
                } else {
                  for (int u = 0; u != 8; u++) {
                    ptr16[0] = pal256[chr64b[0]];
                    ptr16[1] = pal256[chr64b[1]];
                    ptr16[2] = pal256[chr64b[2]];
                    ptr16[3] = pal256[chr64b[3]];
                    ptr16[4] = pal256[chr64b[4]];
                    ptr16[5] = pal256[chr64b[5]];
                    ptr16[6] = pal256[chr64b[6]];
                    ptr16[7] = pal256[chr64b[7]];
                    ptr16 += direct_write.vpitch16;
                    chr64b += y_vec;
                  }
                }
              }
            }
          } else {
            // clr 16 
            for (int iY = 0; iY != iElemVectorY; iY++) {
              for (int iX = 0; iX != iElemVectorX; iX++) {
                int iTileX = iElemStartX + iX;
                int iTileY = iElemStartY + iY;
                uintptr_t hori_bank = iTileX >> 5 & 1;
                uintptr_t vert_bank = iTileY >> 5 & 1;
                uintptr_t offx_page = iTileX & 31;
                uintptr_t offy_page = iTileY & 31;
                uintptr_t addr_base =  nt_bank[(vert_bank<<1)+hori_bank];
                uintptr_t addr_total = addr_base + (offy_page << 6) + (offx_page << 1);
                uint16_t tile_attr =  * (uint16_t *) &m_GBA->gpu.vram[addr_total & 0xFFFF];
                uint8_t *chr32b = &m_GBA->gpu.vram[chr_base_block + (tile_attr & 1023) * 32 & 0xFFFF];
                intptr_t y_vec = 4;
                uint16_t *ptr16;
                uint16_t *pal16 = & pal256[(tile_attr >> 12 & 15) << 4];

                if (bGridCheck == FALSE)
                  ptr16  = &  direct_write.vptr[direct_write.vpitch16 * iY * 8 + iX * 8];
                else {
                  ptr16  = &  direct_write.vptr[ (direct_write.vpitch16 * iY * 9 +direct_write.vpitch16)  + iX * 9 + 1];
                  uint16_t *draw_mesh = ptr16 - 1 - direct_write.vpitch16;
                  for (int Id =0; Id != 2; Id++)
                    for (int x = 0; x != 10; x++)
                      draw_mesh[Id * 9 * direct_write.vpitch16 + x] =  m_dwClrGrid;

                  for (int Id =0; Id != 2; Id++)
                    for (int x = 0; x != 10; x++)
                      draw_mesh[Id *  9 + x * direct_write.vpitch16] =  m_dwClrGrid;
                }
                if (  (tile_attr & 0x800) ){
                  y_vec = -4;
                  chr32b = & chr32b[28];
                }
                if (tile_attr & 0x400) {
                  for (int u = 0; u != 8; u++) {
                    ptr16[0] = pal16[chr32b[3] >> 4 & 15];
                    ptr16[1] = pal16[chr32b[3] >> 0 & 15];
                    ptr16[2] = pal16[chr32b[2] >> 4 & 15];
                    ptr16[3] = pal16[chr32b[2] >> 0 & 15];
                    ptr16[4] = pal16[chr32b[1] >> 4 & 15];
                    ptr16[5] = pal16[chr32b[1] >> 0 & 15];
                    ptr16[6] = pal16[chr32b[0] >> 4 & 15];
                    ptr16[7] = pal16[chr32b[0] >> 0 & 15];
                    ptr16 += direct_write.vpitch16;
                    chr32b += y_vec;
                  }
                } else {
                  for (int u = 0; u != 8; u++) {
                    ptr16[0] = pal16[chr32b[0] >> 0 & 15];
                    ptr16[1] = pal16[chr32b[0] >> 4 & 15];
                    ptr16[2] = pal16[chr32b[1] >> 0 & 15];
                    ptr16[3] = pal16[chr32b[1] >> 4 & 15];
                    ptr16[4] = pal16[chr32b[2] >> 0 & 15];
                    ptr16[5] = pal16[chr32b[2] >> 4 & 15];
                    ptr16[6] = pal16[chr32b[3] >> 0 & 15];
                    ptr16[7] = pal16[chr32b[3] >> 4 & 15];
                    ptr16 += direct_write.vpitch16;
                    chr32b += y_vec;
                  }
                }
              }
            }
          }
        }
      else 
    case 2: // Mode 2 
        {
          // Render rot tile.
          uint16_t chr_base_block = (bg_ctl >> 2 & 3) * 0x4000;
          uint16_t nt_base_block = (bg_ctl >> 8 & 31) * 0x800;
          uintptr_t tile_pitch = m_szScreen.cx >> 3;

          for (int iY = 0; iY != iElemVectorY; iY++) {
            for (int iX = 0; iX != iElemVectorX; iX++) {
              int iTileX = iElemStartX + iX;
              int iTileY = iElemStartY + iY;
              uint16_t tid = m_GBA->gpu.vram[nt_base_block + iTileX + tile_pitch * iTileY & 0xFFFF];
              uint8_t *chr64b = &m_GBA->gpu.vram[chr_base_block + tid * 64 & 0xFFFF];
              uint16_t *ptr16;

              if (bGridCheck == FALSE)
                ptr16  = &  direct_write.vptr[direct_write.vpitch16 * iY * 8 + iX * 8];
              else {
                ptr16  = &  direct_write.vptr[ (direct_write.vpitch16 * iY * 9 +direct_write.vpitch16)  + iX * 9 + 1];
                uint16_t *draw_mesh = ptr16 - 1 - direct_write.vpitch16;
                for (int Id =0; Id != 2; Id++)
                  for (int x = 0; x != 10; x++)
                    draw_mesh[Id * 9 * direct_write.vpitch16 + x] =  0;

                for (int Id =0; Id != 2; Id++)
                  for (int x = 0; x != 10; x++)
                    draw_mesh[Id *  9 + x * direct_write.vpitch16] =  0;
              }

              for (int u = 0; u != 8; u++) {
                ptr16[0] = pal256[chr64b[0]];
                ptr16[1] = pal256[chr64b[1]];
                ptr16[2] = pal256[chr64b[2]];
                ptr16[3] = pal256[chr64b[3]];
                ptr16[4] = pal256[chr64b[4]];
                ptr16[5] = pal256[chr64b[5]];
                ptr16[6] = pal256[chr64b[6]];
                ptr16[7] = pal256[chr64b[7]];
                ptr16 += direct_write.vpitch16;
                chr64b += 8;
              }
            }
          }
        }
        if (bRect == FALSE)
          goto SkipGrid;
        {
          // draw scroll line.
          const uint32_t scrio_base = 0x4000010 + t_dwNametableID * 4;
          const uint16_t scrx = gba_fast_rhw (m_GBA, scrio_base) & 511;
          const uint16_t scry = gba_fast_rhw (m_GBA, scrio_base + 2) & 511;
          const int limmask_x = m_szScreen.cx - 1;
          const int limmask_y = m_szScreen.cy - 1;
          int lim_left = iElemStartX * 8;
          int lim_right = lim_left + iElemVectorX * 8;
          int lim_top = iElemStartY * 8;
          int lim_bottom = lim_top + iElemVectorY * 8;
          int init_left = scrx;
          int init_top = scry;
          int init_right = init_left + 240;
          int init_bottom = init_top + 160;

          CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_SCRAFF), _T ("scr@%d, %d"),  scrx, scry );
#undef set_rect
#define set_rect(left, top, xvec, yvec)\
          init_left = (left);\
          init_top = (top);\
          init_right = init_left + xvec;\
          init_bottom = init_top + yvec;
#undef set_rect2
#define set_rect2(left, top, right, bottom)\
          init_left = (left) + scrx;\
          init_top = (top) + scry;\
          init_right = (right) + scrx;\
          init_bottom = (bottom) + scry;
#undef draw_rect_
#define draw_rect_(/* uint16_t */ col)                                   \
      for (int Id =0; Id != 2; Id++) {                                  \
        const int vr_point = (Id ? init_bottom : init_top) & limmask_y;               \
        const int hr_point = (Id ? init_right : init_left) & limmask_x;               \
        if ( vr_point >= lim_top && vr_point < lim_bottom) {            \
          uint16_t *vptr_t = & direct_write.vptr[(vr_point - lim_top) *direct_write.vpitch16];  \
          for (int x = init_left; x != init_right; x++) {         \
            int srx = x & limmask_x;                                    \
            if (srx >= lim_left  && srx < lim_right)                    \
            { vptr_t[srx-lim_left] = col; }                             \
          }                                                             \
        }                                                               \
        if ( hr_point >= lim_left && hr_point < lim_right) {            \
          uint16_t *vptr_t = & direct_write.vptr[hr_point - lim_left];             \
          for (int y = init_top; y != init_bottom; y++) {         \
            int sry = y & limmask_y;                                    \
            if (sry >= lim_top && sry < lim_bottom)                     \
              { vptr_t[(sry-lim_top) * direct_write.vpitch16] = col; }               \
            }                                                           \
        }                                                               \
      } 

#define draw_rect_mesh(/* uint16_t */ col)                                   \
      for (int Id =0; Id != 2; Id++) {                                  \
        const int vr_point = (Id ? init_bottom : init_top) & limmask_y;               \
        const int hr_point = (Id ? init_right : init_left) & limmask_x;               \
        if ( vr_point >= lim_top && vr_point < lim_bottom) {            \
          uint16_t vbmoft = vr_point - lim_top; \
          vbmoft = vbmoft/ 8 * 9 + (vbmoft & 7) + 1;\
          uint16_t *vptr_t = & direct_write.vptr[vbmoft *direct_write.vpitch16 + 1];  \
          uint16_t init_left_ = init_left/ 8 * 9 + (init_left & 7) + 1;\
          uint16_t init_right_ = init_right/ 8 * 9 + (init_right & 7) + 1;\
          uint16_t lim_left_ = lim_left/ 8 * 9 + (lim_left & 7) + 1;\
          uint16_t lim_right_ = lim_right/ 8 * 9 + (lim_right & 7) + 1;\
          uint16_t limmod_x = limmask_x + 1; \
          limmod_x = limmask_x/ 8 * 9 + (limmask_x & 7) + 1;\
          for (int x = init_left_; x != init_right_; x++) {         \
            int srx = x % limmod_x;                                    \
            if (srx >= lim_left_  && srx < lim_right_)                    \
            { vptr_t[srx-lim_left_] = col; }                             \
          }                                                             \
        }                                                               \
        if ( hr_point >= lim_left && hr_point < lim_right) {            \
          uint16_t hbmoft = hr_point - lim_left; \
          hbmoft = hbmoft/ 8 * 9 + (hbmoft & 7) + 1;\
          uint16_t *vptr_t = & direct_write.vptr[hbmoft + direct_write.vpitch16];             \
          uint16_t init_top_ = init_top/ 8 * 9 + (init_top & 7) + 1;\
          uint16_t init_bottom_ = init_bottom/ 8 * 9 + (init_bottom & 7) + 1;\
          uint16_t lim_top_ = lim_top/ 8 * 9 + (lim_top & 7) + 1;\
          uint16_t lim_bottom_ = lim_bottom/ 8 * 9 + (lim_bottom & 7) + 1;\
          uint16_t limmod_y = limmask_y + 1; \
          limmod_y = limmask_y/ 8 * 9 + (limmask_y & 7) + 1;\
          for (int y = init_top_; y != init_bottom_; y++) {         \
            int sry = y % limmod_y;                                    \
            if (sry >= lim_top_ && sry < lim_bottom_)                     \
              { vptr_t[(sry-lim_top_) * direct_write.vpitch16] = col; }               \
            }                                                           \
        }                                                               \
      } 

#define draw_rect(/* uint16_t */ col)\
  if (bGridCheck != FALSE) {\
          draw_rect_mesh (col);\
  } else {\
          draw_rect_ (col);\
  }
          // draw scroll rect line. or affine start point
          draw_rect (m_dwClrScrAffine);

          // draw win 0/1 grid.
          for (int Id = 0; Id != 2; Id++) {
            const uint32_t winio_base = 0x4000040 + Id * 2;
            const uint16_t rl = gba_fast_rhw (m_GBA, winio_base);
            const uint16_t bt = gba_fast_rhw (m_GBA, winio_base + 4);
            const uint16_t col = Id ? m_dwClrWin1 : m_dwClrWin0;
            const uint16_t ctl = gba_fast_rhw (m_GBA, 0x4000048);
            const uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);

            if ( ! ( (master_ctl >> 13 + Id & 1)
                      && (ctl >> Id * 8 & 1 << t_dwNametableID) ))
                      continue;

            uint16_t right = rl & 255;
            uint16_t left = rl >> 8 & 255;
            uint16_t bottom = bt & 255;
            uint16_t top = bt >> 8 & 255;

            if (Id == 0)
              CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN0), _T ("win0@%d, %d, %d, %d"), left, top, right, bottom);
            else 
              CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN1), _T ("win1@%d, %d, %d, %d"),  left, top, right, bottom);

            if (right > 240)
              right = 240;
            if (bottom > 160)
              bottom = 160;
            if (left > 240)
              left = 240;
            if (top > 160)
              top = 160;
            if (left > right) {
              // h- separate
              if (top > bottom) {
                // v - separate
                std::swap (left, right);
                std::swap (top, bottom);
                set_rect2 (0, 0, left, top);
                draw_rect (col);
                set_rect2 (left, 0, 240, top);
                draw_rect (col);
                set_rect2 (0, bottom, left, 160);
                draw_rect (col);
                set_rect2 (right, bottom, 240, 160);
                draw_rect (col);
              } else {
                // v- inline
                std::swap (left, right);
                set_rect2 (0, top, right, bottom);
                draw_rect (col);
                set_rect2 (right, top, 240, bottom);
                draw_rect (col);
              }
            } else {
              // h- inline
              if (top > bottom) {
                // v - separate
                std::swap (top, bottom);
                set_rect2 (left, 0, right, top);
                draw_rect (col);
                set_rect2 (left, bottom, right, 160);
                draw_rect (col);
              } else {
                // v- inline
                set_rect2 (left, top, right, bottom);
                draw_rect (col);
              }
            }
          }
SkipGrid:
        // Alpha rect 
        if (bLockRect != FALSE)
          m_ContainerOverview.alphaRect 
                   (direct_write, CRect (ptLockStart.x, ptLockStart.y, ptLockStart.x + 8, ptLockStart.y + 8), m_dwClrShadow, 8, 31-8);
                                    
        // Present to direct3d surface. 
        m_ContainerOverview.UnlockSurface ();
        CRect rcPostRect;
        if (bGridCheck == FALSE) {
          rcPostRect.left =  m_ContainerOverview. m_ptOffset.x & 7;
          rcPostRect.top =  m_ContainerOverview. m_ptOffset.y & 7;
        } else {
          rcPostRect.left =  m_ContainerOverview. m_ptOffset.x % 9;
          rcPostRect.top =  m_ContainerOverview. m_ptOffset.y % 9;
        }
        rcPostRect.right = rcPostRect.left + rcClient.Width ();
        rcPostRect.bottom = rcPostRect.top + rcClient.Height ();
        m_ContainerOverview.blitRectFromSurface (rcPostRect);
        }
    case 3:
    case 4:
    case 5:
      default:
      break ;
    }
    return ;
NoshowServices:
    DirectWrite16 write16;
    m_ContainerOverview.LockSurface (write16);
    for (int y = 0; y != rcClient.Height (); y ++) {
      time (NULL);
      WORD clr = rand ();
      for (int x= 0; x != rcClient.Width (); x ++) {
        write16.vptr[y *write16.vpitch16 + x] =  clr;
      }
    }
    m_ContainerOverview.UnlockSurface ();
    m_ContainerOverview.blitRectFromSurface (CRect (0,0, 0 + rcClient.Width (), 0 + rcClient.Height ()));

    }
  }
 
  LRESULT OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
    BOOL bGridCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_GRID)).GetCheck () == BST_CHECKED;
    BOOL bLock =  CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_LOCK)).GetCheck () == BST_CHECKED;
    BOOL bRect =  CButton (m_ContainerInfosPane.GetDlgItem (IDCK_NT_RECT)).GetCheck () == BST_CHECKED;

    if ( ! ( (uMsg == WM_LBUTTONUP
      && bLock != FALSE)
      ||   (uMsg == WM_MOUSEMOVE
      && bLock == FALSE) ) )
    {
      bHandled = FALSE;
      return 0;
    }
    m_ptTrack.x = LOWORD (lParam);
    m_ptTrack.y = HIWORD (lParam);
    m_ptTrack += m_ContainerOverview.m_ptOffset;

    if (bGridCheck != FALSE) {
      m_ptTrack.x /= 9;
      m_ptTrack.y /= 9;
    } else {
      m_ptTrack.x /= 8;
      m_ptTrack.y /= 8;
    }

    uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);
    uint16_t bg_ctl =  gba_fast_rhw (m_GBA, 0x4000008 + (t_dwNametableID << 1));
    uint16_t nt_page = bg_ctl >> 8 & 31;
    uint16_t nt_addr = nt_page * 0x800;
    uint16_t chr_page = bg_ctl >> 2 & 3;
    uint16_t chr_addr =  chr_page * 0x4000;
    uint16_t tile_x = m_ptTrack.x;
    uint16_t tile_y = m_ptTrack.y;
    uint16_t tile_attr;
    uint16_t vmode = gba_fast_rhw (m_GBA, 0x4000000) & 7;
    uint16_t vsize = gba_fast_rhw (m_GBA, 0x4000008 + (t_dwNametableID << 1)) >> 14 & 3;
    
    CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN0), _T ("win0@null"));
    CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN1), _T ("win1@null"));
    
    for (int Id = 0; Id != 2; Id++) {
      const uint32_t winio_base = 0x4000040 + Id * 2;
      const uint16_t rl = gba_fast_rhw (m_GBA, winio_base);
      const uint16_t bt = gba_fast_rhw (m_GBA, winio_base + 4);
      const uint16_t ctl = gba_fast_rhw (m_GBA, 0x4000048);
      const uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);

      if ( ! ( (master_ctl >> 13 + Id & 1)
                && (ctl >> Id * 8 & 1 << t_dwNametableID) ))
                continue;

      uint16_t right = rl & 255;
      uint16_t left = rl >> 8 & 255;
      uint16_t bottom = bt & 255;
      uint16_t top = bt >> 8 & 255;

      if (Id == 0)
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN0), _T ("win0@%d, %d, %d, %d"), left, top, right, bottom);
      else 
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_WIN1), _T ("win1@%d, %d, %d, %d"),  left, top, right, bottom);
    }

    switch (master_ctl & 7) {
    case 1: // Mode 1
      if (t_dwNametableID < 2)
    case 0:
        {
          //  std tile.
          const uint32_t scrio_base = 0x4000010 + t_dwNametableID * 4;
          const uint16_t scrx = gba_fast_rhw (m_GBA, scrio_base) & 511;
          const uint16_t scry = gba_fast_rhw (m_GBA, scrio_base + 2) & 511;
          CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_SCRAFF), _T ("scr@%d, %d"),  scrx, scry );

          uint16_t nt_bank[4] = { 0, 0, 0, 0 };

          // Get VRAM Mapper Type.
          switch (bg_ctl & 0xC000) {
          case 0x4000:
            nt_bank[1] = 
            nt_bank[3] = 0x0800;   
            break;
          case 0x8000:  
            nt_bank[2] = 
            nt_bank[3] = 0x0800;
            break;
          case 0xC000:
            nt_bank[1] = 0x0800;
            nt_bank[2] = 0x1000;
            nt_bank[3] = 0x1800;
          default:
            break;
          }
          nt_bank[0] += nt_addr;
          nt_bank[1] += nt_addr;
          nt_bank[2] += nt_addr;
          nt_bank[3] += nt_addr;

          uintptr_t hori_bank = m_ptTrack.x >> 5 & 1;
          uintptr_t vert_bank = m_ptTrack.y >> 5 & 1;
          uintptr_t offx_page = m_ptTrack.x & 31;
          uintptr_t offy_page = m_ptTrack.y & 31;
          uintptr_t addr_base =  nt_bank[(vert_bank<<1)+hori_bank];

          nt_addr = addr_base + (offy_page << 6) + (offx_page << 1) & 0xFFFF;
          tile_attr =  * (uint16_t *) &m_GBA->gpu.vram[nt_addr];

          if (bg_ctl & 0x80) {   
            :: SetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_BANK), _T ("bank@null"));
            chr_addr = chr_addr + (tile_attr & 1023) * 64 & 0xFFFF;
          } else {
            CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_BANK), _T ("bank@%X"), tile_attr >> 12 & 15);
            chr_addr = chr_addr + (tile_attr & 1023) * 32 & 0xFFFF;
          }
          CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_HFLIP), tile_attr & 0x400 ? _T ("hflip@true") : _T("hflip@false"));
          CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_VFLIP), tile_attr & 0x800 ? _T ("vflip@true") : _T("vflip@false"));
        }
      else 
    case 2: // Mode 2 
        {
          :: SetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_HFLIP), _T ("hflip@null"));
          :: SetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_VFLIP), _T ("vflip@null"));
          :: SetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_BANK), _T ("bank@null"));

          // Render rot tile.
          uintptr_t tile_pitch = m_szScreen.cx >> 3;

          nt_addr = nt_addr + m_ptTrack.x + tile_pitch * m_ptTrack.y & 0xFFFF;
          tile_attr = m_GBA->gpu.vram[nt_addr];
          chr_addr = chr_addr + tile_attr * 64 & 0xFFFF;
        }
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_NTP), _T ("ntp@%d"), nt_page);
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_NTA), _T ("nta@%07X"), nt_addr + 0x6000000);
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_TID), _T ("tid@%03X"), tile_attr & 1023);
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_CHRAP), _T ("chrap@%d-%07X"), chr_page, chr_addr + 0x6000000);
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_X), _T ("x, y@%X, %X"), tile_x, tile_y);
        CSetWindowText (m_ContainerInfosPane.GetDlgItem (IDST_NT_Y), _T ("vmode, size@%d, %d"), vmode, vsize);
    default:
      break;
    }
    // InvalidateRect (NULL, FALSE);
    UpdateGBAItemOut (TRUE);
    return 0;
  }
  
  LRESULT OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    DWORD *pColor = NULL;
    bHandled = TRUE;
    
    switch (LOWORD (wParam)) {
    case IDBT_NT_GRID_COL: pColor = & m_dwClrGrid; break;
    case IDBT_NT_SCR_COL: pColor = & m_dwClrScrAffine; break;
    case IDBT_NT_SHADOW_COL: pColor = & m_dwClrShadow; break;
    case IDBT_NT_WIN0_COL: pColor = & m_dwClrWin0; break;
    case IDBT_NT_WIN1_COL: pColor = & m_dwClrWin1; break;
    case IDCK_NT_GRID:
    case IDCK_NT_QFLUSH:
    case IDCK_NT_LOCK:
    case IDCK_NT_RECT:  
      break;
    default:
      bHandled = FALSE; 
      return 0;
    }
    if (pColor != NULL) {
      CColorDialog col_dlg (Col_DirectDraw16_2GDI32 (* pColor));
      if (col_dlg.DoModal () == IDOK) {
        * pColor =  Col_GDI32_2DirectDraw16 ( col_dlg.GetColor ());
      }
    }
    m_ContainerOverview.FillColor (0);
    UpdateGBAItemOut (TRUE, TRUE);
    return 0;
  }

  DECLARE_WND_CLASS_FX (_T ("CViewNametable"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, IDI_NAMETABLE)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP ()

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
    m_AttachMenu.EnableMenuItem (t_iMenuId, MF_ENABLED | MF_BYCOMMAND );
    ShowWindow (SW_HIDE);
    // Post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }
  
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rc;
    GetClientRect (& rc);
    m_GuiSWinSplitter_View.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rcClient;
    GetClientRect (rcClient);
    m_GuiSWinSplitter_View.Create (m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TRANSPARENT);

    m_ContainerOverview.Create (m_GuiSWinSplitter_View, rcDefault, _T ("null"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TRANSPARENT);
    m_ContainerInfosPane.Create (m_GuiSWinSplitter_View, rcDefault);
    m_ContainerInfosPane.ShowWindow (SW_SHOWNORMAL);
    m_GuiSWinSplitter_View.SetSplitterPanes (m_ContainerOverview, m_ContainerInfosPane);
    
    m_ContainerOverview.ShowWindow (SW_SHOWNORMAL);
    m_ContainerOverview.DoSize (256, 256);
    m_ContainerOverview.ResizeClient (256, 256);
    m_ContainerOverview.SetScrollSize (256, 256);
    
    ResizeClient (284, 520);
    
    m_GuiSWinSplitter_View.ResizeClient (284, 520);
    m_GuiSWinSplitter_View.SetSplitterPos (272);

    m_ContainerInfosPane.ModifyStyleEx (0, WS_EX_TRANSPARENT | WS_EX_COMPOSITED);
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (t_iMenuId, MF_DISABLED | MF_BYCOMMAND );
    
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, gba *attachGBA) {
    m_GBA = attachGBA;
    Create (parent, CRect (0,0, 320, 360), _T ("Nametable"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  WS_EX_TRANSPARENT);
  }
};

# endif 