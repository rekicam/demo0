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

// FIXME: Very bad code, only it does work.
// TODO: Some color line button logic.
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVIEWOAM_INCLUED_HEADER_CXX_)
#  define _CVIEWOAM_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <tchar.h>
# include "..\resource.h"
# include "..\GBA\gba.h"
# include <d3d9.h>
# include <algorithm>

template <typename T> static __forceinline 
void GetOamDim (uint16_t *oamBank, T &dim_x, T &dim_y) {

  switch ((oamBank[1] & 0xC000) | (oamBank[0] >> 2 & 0x3000)) {
  case 0x0000: dim_x = 8; dim_y = 8; break;
  case 0x1000: dim_x =16; dim_y = 8; break;
  case 0x2000: dim_x = 8; dim_y =16; break;
  case 0x4000: dim_x =16; dim_y =16; break;
  case 0x5000: dim_x =32; dim_y = 8; break;
  case 0x6000: dim_x = 8; dim_y =32; break;
  case 0x8000: dim_x =32; dim_y =32; break;
  case 0x9000: dim_x =32; dim_y =16; break;
  case 0xA000: dim_x =16; dim_y =32; break;
  case 0xC000: dim_x =64; dim_y =64; break;
  case 0xD000: dim_x =64; dim_y =32; break;
  case 0xE000: dim_x =32; dim_y =64; break;
  default:
    dim_x = 8;
    dim_y = 8;
    DEBUG_BREAK ();
    break;
  }
}

template <class T, DWORD t_dwWidth, DWORD t_dwHeight>
struct CViewOamOverview :  
  public CDirectWrite16WindowREF < T, ATL::CWindow, ATL::CControlWinTraits, t_dwWidth, t_dwHeight, TRUE >{
  
  static const int mesh_edge_line = 0xffff; 
  static const int shadow_edge_line = 0xff;
  static const int lock_edge_line = 0xff00;
  static const short squmesh_size = 32;

  DWORD m_dwClrMesh;
  DWORD m_dwClrShadow;
  DWORD m_dwClrLock;
  CPoint m_ptLock;
  gba *m_GBA;

  // Constructor
  CViewOamOverview (T &attachThis_): 
    CDirectWrite16WindowREF < T, ATL::CWindow, ATL::CControlWinTraits, t_dwWidth, t_dwHeight, TRUE > (attachThis_),
    m_dwClrMesh (mesh_edge_line),
    m_dwClrShadow (shadow_edge_line),
    m_dwClrLock (lock_edge_line),
    m_ptLock (0, 0)
  { }
  // Destructor
  ~CViewOamOverview (void) {}

  void AdjustTrackPos (CPoint &pt) {
    static const int iVec = squmesh_size + 1;
    pt += m_ptOffset;

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

  void UpdateGBAItemOut (void) {

    CPoint ptTrack (-1,  -1);
    CRect rcClient;
    static const uintptr_t lim = squmesh_size + 1;
    CPoint pt = m_ptOffset;
    CPoint pt_mod;
    CSize vec;
    GetClientRect (rcClient);

    if (rcClient.Width () <= 0 
      || rcClient.Height () <= 0)
      return ;

    uint16_t tile_pitch;
    uint16_t *pal256 = (uint16_t *) & m_GBA->gpu.palette2[512];

    if (m_GBA->gpu.ctl.blk & 0x40)
      tile_pitch = 0;
    else 
      tile_pitch = 32;

    DirectWrite16 direct_write;
    LockSurface (direct_write);

    pt.x = m_ptOffset.x / lim;
    pt.y = m_ptOffset.y / lim;
    pt_mod.x = m_ptOffset.x % lim;
    pt_mod.y = m_ptOffset.y % lim;
    vec.cx = rcClient.Width () / lim + 2;
    vec.cy = rcClient.Height () / lim + 2;

    if ( (pt.x + vec.cx) > 16) 
      vec.cx = 16 - pt.x;
    if ( (pt.y + vec.cy) > 8) 
      vec.cy = 8 - pt.y;

    uint16_t pixel[64*64];
    for (int y = 0; y != vec.cy; y++) {
      for (int x = 0; x != vec.cx; x++) {
        int x0 = pt.x + x;
        int y0 = pt.y + y;
        int slot = x0 + y0 * 16;
        int dim_x = 0;
        int dim_y = 0;
        int tile_id;
        int tile_hvec = 1;
        
        if (x0 == m_ptLock.x 
          && y0 == m_ptLock.y)
        {
          ptTrack.x = x;
          ptTrack.y = y;
        }

        uint16_t *draw_ptr=  & direct_write.vptr [x * lim + y * lim * direct_write.vpitch16 + 1+direct_write.vpitch16];
        // Fetch sprite.
        uint16_t *oamblk = (uint16_t *)& m_GBA->gpu.oam[slot *8];
        if ((oamblk[0] & 0x300) == 0x200
          || (oamblk[0] & 0xC00) == 0xC00) {
          // draw mesh grid. 
          for (int y2 = 0; y2 != squmesh_size; y2++) {
            for (int x2 = 0; x2 != squmesh_size; x2++) {
              draw_ptr[y2*direct_write.vpitch16+x2] = 0;
            }
          }
          for (int u = 0; u != squmesh_size; u++) 
            draw_ptr[u*direct_write.vpitch16+u] = m_dwClrMesh;
          drawHorzLine (direct_write, y * lim, x * lim, lim + 1, m_dwClrMesh);
          drawHorzLine (direct_write, y * lim + lim, x * lim, lim + 1, m_dwClrMesh);
          drawVertLine (direct_write, x * lim, y * lim, lim + 1, m_dwClrMesh);
          drawVertLine (direct_write, x * lim + lim, y * lim, lim + 1, m_dwClrMesh);
          continue ;
        } else {
          const unsigned short mclr = (oamblk[0] & 0x800) ? m_dwClrShadow : m_dwClrMesh;
          drawHorzLine (direct_write, y * lim, x * lim, lim + 1, mclr);
          drawHorzLine (direct_write, y * lim + lim, x * lim, lim + 1, mclr);
          drawVertLine (direct_write, x * lim, y * lim, lim + 1, mclr);
          drawVertLine (direct_write, x * lim + lim, y * lim, lim + 1, mclr);

          GetOamDim<int> (oamblk, dim_x, dim_y);
          uint16_t *pal16 = & pal256[(oamblk[2] >> 12) << 4];
          tile_id=  oamblk[2] & 1023;

          if (oamblk[0] & 0x2000) { 
             if ((tile_pitch & 32) == 0)
               tile_pitch = dim_x>> 2; 
             /* tile_id &= 1022; */
             tile_hvec = 2;
          } else  if ((tile_pitch & 32) == 0) 
               tile_pitch = dim_x>> 3; 

          // Render tile.
          for (int y2 = 0; y2 != dim_y >> 3; y2++) {
            for (int x2 = 0; x2 != dim_x >> 3; x2++) {
              int tid_cur = tile_id + y2 * tile_pitch + x2 * tile_hvec;
              uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid_cur*32];
              uint16_t *ptr16 = & pixel[64 *y2 * 8 + x2* 8];
              if (oamblk[0] & 0x2000) { 
                 for (int Id = 0; Id != 8; Id++) {
                    ptr16[0] = pal256[chr[0]];
                    ptr16[1] = pal256[chr[1]];
                    ptr16[2] = pal256[chr[2]];
                    ptr16[3] = pal256[chr[3]];
                    ptr16[4] = pal256[chr[4]];
                    ptr16[5] = pal256[chr[5]];
                    ptr16[6] = pal256[chr[6]];
                    ptr16[7] = pal256[chr[7]];
                    ptr16 += 64;
                    chr += 8;
                 }
              } else {
                 for (int Id = 0; Id != 8; Id++) {
                    ptr16[0] = pal16[chr[0] >> 0 & 15];
                    ptr16[1] = pal16[chr[0] >> 4 & 15];
                    ptr16[2] = pal16[chr[1] >> 0 & 15];
                    ptr16[3] = pal16[chr[1] >> 4 & 15];
                    ptr16[4] = pal16[chr[2] >> 0 & 15];
                    ptr16[5] = pal16[chr[2] >> 4 & 15];
                    ptr16[6] = pal16[chr[3] >> 0 & 15];
                    ptr16[7] = pal16[chr[3] >> 4 & 15];
                    ptr16 += 64;
                    chr += 4;
                 }
              }
            }
          }
          for (int y2 = 0; y2 != squmesh_size; y2++) {
            int y3 = y2 * dim_y/ squmesh_size;
            for (int x2 = 0; x2 != squmesh_size; x2++) {
              int x3 = x2 * dim_x /   squmesh_size;
              draw_ptr[y2*direct_write.vpitch16+x2] = pixel[x3+y3*64];
            }
          }
        }
      }
    }

    if (ptTrack.x >= 0) {
      drawHorzLine (direct_write,  ptTrack.y * lim, ptTrack.x * lim, lim + 1, m_dwClrLock);
      drawHorzLine (direct_write, ptTrack.y * lim + lim, ptTrack.x * lim, lim + 1, m_dwClrLock);
      drawVertLine (direct_write, ptTrack.x * lim, ptTrack.y * lim, lim + 1, m_dwClrLock);
      drawVertLine (direct_write, ptTrack.x * lim + lim, ptTrack.y * lim, lim + 1, m_dwClrLock);
    }

    UnlockSurface ();
    CRect rcPostRect;
    rcPostRect.left = pt_mod.x;
    rcPostRect.top = pt_mod.y;
    rcPostRect.right = rcPostRect.left + rcClient.Width ();
    rcPostRect.bottom = rcPostRect.top + rcClient.Height ();
    blitRectFromSurface (rcPostRect);
  }

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
    UpdateGBAItemOut ();
  }
};

template <class T, DWORD t_dwWidth, DWORD t_dwHeight>
struct CViewOamRaw :  
  public CDirectWrite16WindowREF < T, ATL::CWindow, ATL::CControlWinTraits, t_dwWidth, t_dwHeight, TRUE >{

  int m_iSlot;
  gba *m_GBA;
  CPoint m_ptTrack;

  CViewOamRaw (T &attachThis_): m_iSlot (-1), m_ptTrack (0, 0),
  CDirectWrite16WindowREF < T, ATL::CWindow, ATL::CControlWinTraits, t_dwWidth, t_dwHeight, TRUE > (attachThis_)
  {}
  ~CViewOamRaw (void) {}

  void UpdateGBAItemOut (void) {

    if (!(m_iSlot >= 0
      && m_iSlot <= 127))
      return ;
    else 
      FillColor (0);

    CRect rcClient;
    GetClientRect (rcClient);

    if (rcClient.Width () <= 0 
      || rcClient.Height () <= 0)
      return ;

    uint16_t tile_pitch;
    uint16_t *pal256 = (uint16_t *) & m_GBA->gpu.palette2[512];

    if (m_GBA->gpu.ctl.blk & 0x40)
      tile_pitch = 0;
    else 
      tile_pitch = 32;

    DirectWrite16 direct_write;
    LockSurface (direct_write);

    int dim_x = 0;
    int dim_y = 0;
    int tile_id;
    int tile_hvec = 1;
    uint16_t *oamblk = (uint16_t *)& m_GBA->gpu.oam[m_iSlot *8];

    if ((oamblk[0] & 0x300) == 0x200
      || (oamblk[0] & 0xC00) == 0xC00) {
    } else {
      GetOamDim<int> (oamblk, dim_x, dim_y);
      uint16_t *pal16 = & pal256[(oamblk[2] >> 12) << 4];
      tile_id=  oamblk[2] & 1023;

      if (oamblk[0] & 0x2000) { 
          if ((tile_pitch & 32) == 0)
            tile_pitch = dim_x>> 2; 
          /* tile_id &= 1022; */
          tile_hvec = 2;
      } else  if ((tile_pitch & 32) == 0) 
            tile_pitch = dim_x>> 3; 

      // Render tile.
      int xTrack = m_ptTrack.x / 8;
      int yTrack = m_ptTrack.y / 8;
      if (xTrack < 0)
        xTrack = 0;
      if (yTrack < 0)
        yTrack = 0;



      for (int y2 = 0; y2 != dim_y >> 3; y2++) {
        for (int x2 = 0; x2 != dim_x >> 3; x2++) {
          int tid_cur = tile_id + y2 * tile_pitch + x2 * tile_hvec;
          uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid_cur*32];
          uint16_t *ptr16 = & direct_write.vptr[direct_write.vpitch16 *y2 * 8 + x2* 8];
          if (y2 == m_ptTrack.y / 8
            && x2 == m_ptTrack.x / 8) {
            // Alpha rect
            if (oamblk[0] & 0x2000) { 
                for (int Id = 0; Id != 8; Id++) {
                  ptr16[0] = alpha16  (pal256[chr[0]], 0x1f, 12, 31-12);
                  ptr16[1] = alpha16  (pal256[chr[1]], 0x1f, 12, 31-12);
                  ptr16[2] = alpha16  (pal256[chr[2]], 0x1f, 12, 31-12);
                  ptr16[3] = alpha16  (pal256[chr[3]], 0x1f, 12, 31-12);
                  ptr16[4] = alpha16  (pal256[chr[4]], 0x1f, 12, 31-12);
                  ptr16[5] = alpha16  (pal256[chr[5]], 0x1f, 12, 31-12);
                  ptr16[6] = alpha16  (pal256[chr[6]], 0x1f, 12, 31-12);
                  ptr16[7] = alpha16  (pal256[chr[7]], 0x1f, 12, 31-12);
                  ptr16 += direct_write.vpitch16;
                  chr += 8;
                }
            } else {
                for (int Id = 0; Id != 8; Id++) {
                  ptr16[0] = alpha16  (pal16[chr[0] >> 0 & 15], 0x1f, 12, 31-12);
                  ptr16[1] = alpha16  (pal16[chr[0] >> 4 & 15], 0x1f, 12, 31-12);
                  ptr16[2] = alpha16  (pal16[chr[1] >> 0 & 15], 0x1f, 12, 31-12);
                  ptr16[3] = alpha16  (pal16[chr[1] >> 4 & 15], 0x1f, 12, 31-12);
                  ptr16[4] = alpha16  (pal16[chr[2] >> 0 & 15], 0x1f, 12, 31-12);
                  ptr16[5] = alpha16  (pal16[chr[2] >> 4 & 15], 0x1f, 12, 31-12);
                  ptr16[6] = alpha16  (pal16[chr[3] >> 0 & 15], 0x1f, 12, 31-12);
                  ptr16[7] = alpha16  (pal16[chr[3] >> 4 & 15], 0x1f, 12, 31-12);
                  ptr16 += direct_write.vpitch16;
                  chr += 4;
                }
            }
            continue ;
          }

          if (oamblk[0] & 0x2000) { 
              for (int Id = 0; Id != 8; Id++) {
                ptr16[0] = pal256[chr[0]];
                ptr16[1] = pal256[chr[1]];
                ptr16[2] = pal256[chr[2]];
                ptr16[3] = pal256[chr[3]];
                ptr16[4] = pal256[chr[4]];
                ptr16[5] = pal256[chr[5]];
                ptr16[6] = pal256[chr[6]];
                ptr16[7] = pal256[chr[7]];
                ptr16 += direct_write.vpitch16;
                chr += 8;
              }
          } else {
              for (int Id = 0; Id != 8; Id++) {
                ptr16[0] = pal16[chr[0] >> 0 & 15];
                ptr16[1] = pal16[chr[0] >> 4 & 15];
                ptr16[2] = pal16[chr[1] >> 0 & 15];
                ptr16[3] = pal16[chr[1] >> 4 & 15];
                ptr16[4] = pal16[chr[2] >> 0 & 15];
                ptr16[5] = pal16[chr[2] >> 4 & 15];
                ptr16[6] = pal16[chr[3] >> 0 & 15];
                ptr16[7] = pal16[chr[3] >> 4 & 15];
                ptr16 += direct_write.vpitch16;
                chr += 4;
              }
          }
        }
      }
    }
    UnlockSurface ();
    CRect rcPostRect;
    rcPostRect.left = m_ptOffset.x;
    rcPostRect.top = m_ptOffset.y;
    rcPostRect.right = rcPostRect.left + rcClient.Width ();
    rcPostRect.bottom = rcPostRect.top + rcClient.Height ();
    blitRectFromSurface (rcPostRect);
  }

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
    UpdateGBAItemOut ();
  }
};

template <DWORD t_dwWidth = 240, DWORD t_dwHeight = 160, BOOL t_bShadowRender = FALSE>
struct CViewOamShadow :  
  public CDirectWrite16WindowImpl < CViewOamShadow<t_dwWidth, t_dwHeight, t_bShadowRender>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwWidth, 
                                    t_dwHeight, TRUE >{

  static const short win0_rect_line = 0xff;
  static const short win1_rect_line = 0x00ff;
  static const short attach_mesh_line = -1;
  static const short shadow_bin_rect = -1;
  static const short shadow_bin_clr_rect = 0;

  typedef CDirectWrite16WindowImpl < CViewOamShadow< t_dwWidth, t_dwHeight, t_bShadowRender>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwWidth, 
                                    t_dwHeight, TRUE> baseClass;
  typedef CViewOamShadow thisClass;
  
  DECLARE_WND_CLASS(_T ("CViewOamShadow"))

  BEGIN_MSG_MAP(thisClass)
    CHAIN_MSG_MAP(baseClass)
  END_MSG_MAP()

  int m_iSlot;
  gba *m_GBA;
  
  DWORD m_dwClrWin0;
  DWORD m_dwClrWin1;

  BOOL m_bShadowBinrary;

    CViewOamShadow (void): m_bShadowBinrary (FALSE) {}
  ~CViewOamShadow (void) {}

  void DrawAttachScrRect (DirectWrite16 &direct_write) {

      CRect rcClient;
      GetClientRect (& rcClient);

      if (rcClient.Width() <0
        || rcClient.Height () <0)
        return ;

      int iElemVectorX = rcClient.Width ();
      int iElemVectorY = rcClient.Height ();
      int iElemStartX =  m_ptOffset.x;
      int iElemStartY = m_ptOffset.y;

      if ( (iElemStartX + iElemVectorX) > 240) 
        iElemVectorX = 240 - iElemStartX;
      if ( (iElemStartY + iElemVectorY) > 160) 
        iElemVectorY = 160 - iElemStartY;

      int lim_left = iElemStartX;
      int lim_right = lim_left + iElemVectorX;
      int lim_top = iElemStartY;
      int lim_bottom = lim_top + iElemVectorY;
      int init_left;
      int init_top;
      int init_right;
      int init_bottom;

#undef set_rect2
#define set_rect2(left, top, right, bottom)\
          init_left = (left);\
          init_top = (top);\
          init_right = (right);\
          init_bottom = (bottom);
#undef draw_rect_
#define draw_rect_(/* uint16_t */ col)                                   \
      for (int Id =0; Id != 2; Id++) {                                  \
        const int vr_point = (Id ? (init_bottom - 1) : init_top);               \
        const int hr_point = (Id ? (init_right - 1) : init_left);               \
        if ( vr_point >= lim_top && vr_point < lim_bottom) {            \
          uint16_t *vptr_t = & direct_write.vptr[(vr_point) *direct_write.vpitch16];  \
          for (int x = init_left; x != init_right; x++) {         \
            int srx = x % 240;                                    \
            if (srx >= lim_left  && srx < lim_right)                    \
            { vptr_t[srx] = col; }                             \
          }                                                             \
        }                                                               \
        if ( hr_point >= lim_left && hr_point < lim_right) {            \
          uint16_t *vptr_t = & direct_write.vptr[hr_point ];             \
          for (int y = init_top; y != init_bottom; y++) {         \
            int sry = y % 160;                                    \
            if (sry >= lim_top && sry < lim_bottom)                     \
              { vptr_t[(sry) * direct_write.vpitch16] = col; }               \
            }                                                           \
        }                                                               \
      } 

      for (int Id = 0; Id != 2; Id++) {
        const uint32_t winio_base = 0x4000040 + Id * 2;
        const uint16_t rl = gba_fast_rhw (m_GBA, winio_base);
        const uint16_t bt = gba_fast_rhw (m_GBA, winio_base + 4);
        const uint16_t col = Id ? win1_rect_line : win0_rect_line;
        const uint16_t ctl = gba_fast_rhw (m_GBA, 0x4000048);
        const uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);

        if ( ! ( (master_ctl >> 13 + Id & 1)
                  && (ctl >> Id * 8 & 1 << 4) ))
                  continue;

        uint16_t right = rl & 255;
        uint16_t left = rl >> 8 & 255;
        uint16_t bottom = bt & 255;
        uint16_t top = bt >> 8 & 255;

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
            draw_rect_ (col);
            set_rect2 (left, 0, 240, top);
            draw_rect_ (col);
            set_rect2 (0, bottom, left, 160);
            draw_rect_ (col);
            set_rect2 (right, bottom, 240, 160);
            draw_rect_ (col);
          } else {
            // v- inline
            std::swap (left, right);
            set_rect2 (0, top, right, bottom);
            draw_rect_ (col);
            set_rect2 (right, top, 240, bottom);
            draw_rect_ (col);
          }
        } else {
          // h- inline
          if (top > bottom) {
            // v - separate
            std::swap (top, bottom);
            set_rect2 (left, 0, right, top);
            draw_rect_ (col);
            set_rect2 (left, bottom, right, 160);
            draw_rect_ (col);
          } else {
            // v- inline
            set_rect2 (left, top, right, bottom);
            draw_rect_ (col);
          }
        }
      }
  }

  void DrawAttachSlot (DirectWrite16 &dwrite) {

    if (m_iSlot >= 0 && m_iSlot <= 127) {
      intptr_t dim_x;
      intptr_t dim_y;
      int32_t x_coo;
      int32_t y_coo;
      uint16_t* oam = (uint16_t *)& m_GBA->gpu.oam[m_iSlot<<3];

      if (t_bShadowRender == FALSE) {
        if (((oam[0] & 0x300) == 0x200) 
          || (oam[0] & 0x800))
          return;
      } else {
        if (((oam[0] & 0x300 ) == 0x200) 
          || ((oam[0] & 0xC00) != 0x800))
          return;
      }
      GetOamDim<intptr_t> (oam, dim_x, dim_y);
      x_coo = oam[1] & 511; 
      y_coo = oam[0] & 255; 

      if ( (oam[0] & 0x300) == 0x300) {
        dim_x <<= 1;
        dim_y <<= 1;
      }
      // draw horz line. 
      uint16_t vec_lut[2] = { y_coo & 255, y_coo + dim_y - 1 & 255 };

      for (int Id = 0; Id != 2; Id++) {
        int vec = vec_lut[Id];
        if (vec < 160)  {
          uint16_t *vptr16 = & dwrite.vptr[dwrite.vpitch16 *vec];
          for (int Id = 0; Id != dim_x; Id++) {
            int xpos = x_coo + Id & 511;
            if (xpos < 256)
              vptr16 [xpos] = attach_mesh_line;
          }
        }
      }

      // draw vert.
      vec_lut[0] = x_coo & 511;
      vec_lut[1] = x_coo + dim_x - 1 & 511;
      for (int Id = 0; Id != 2; Id++) {
        int vec = vec_lut[Id];
        if (vec < 256)  {
          uint16_t *vptr16 = & dwrite.vptr[vec];
          for (int Id = 0; Id != dim_y; Id++) {
            int ypos = y_coo + Id & 255;
            if (ypos < 160)
              vptr16 [ypos * dwrite.vpitch16] = attach_mesh_line;
          }
        }
      }
    }
  }

  void DrawSprite (DirectWrite16 &dwrite, int layerLevel) {

    intptr_t cnt; 
    intptr_t dim_x;
    intptr_t dim_y;
    int32_t x_coo;
    int32_t y_coo;
    intptr_t tile_id;
    int32_t tile_pitch;
    uint16_t *pal16;
    uint16_t *pal256 = (uint16_t *) & m_GBA->gpu.palette2[512];
    uint8_t *oamb = & m_GBA->gpu.oam[0];
    intptr_t tid_init;
    intptr_t tid_vphase;

    const BOOL bUseBinColor =  t_bShadowRender && m_bShadowBinrary;
    if (m_GBA->gpu.ctl.blk & 0x40) 
      tile_pitch = 0;
    else 
      tile_pitch = 32;

    for (cnt = 127; cnt != (intptr_t) -1; cnt--) {  

      uint16_t* oam = (uint16_t *)& oamb[cnt<<3];
      if (t_bShadowRender == FALSE) {
        if (((oam[0] & 0x300 ) == 0x200) 
          || (oam[0] & 0x800) ||  (oam[2] & 0xC00) != layerLevel)
          continue;
      } else {
        if (((oam[0] & 0x300 ) == 0x200) 
          ||((oam[0] & 0xC00) != 0x800) || (oam[2] & 0xC00) != layerLevel)
          continue;
      }
      GetOamDim<intptr_t> (oam, dim_x, dim_y);

      x_coo = oam[1] & 511; 
      y_coo = oam[0] & 255; 
      tile_id = oam[2] & 1023;
      pal16 = & pal256[ (oam[2] >> 12) << 4];

      if (oam[0] & 0x2000) { 
         if ((tile_pitch & 32) == 0)
           tile_pitch = dim_x>> 2; 
         /* tile_id &= 1022; */
      } else  if ((tile_pitch & 32) == 0)
           tile_pitch = dim_x>> 3; 

      tid_init = tile_id;
      tid_vphase = tile_pitch;
      if (oam[0] & 0x100) {

        intptr_t affine_bx;
        intptr_t affine_by;
        intptr_t affine_dx;
        intptr_t affine_dy;
        intptr_t affine_dmx;
        intptr_t affine_dmy;
        intptr_t affine_check_left;
        intptr_t affine_check_right;
        intptr_t affine_check_top;
        intptr_t affine_check_bottom;
        int16_t *affbank;

        // Get affine params 
        affbank = (int16_t *)& oamb[(oam[1] >> 9 & 31) << 5];
        affine_dx =affbank[3] ;
        affine_dmx =affbank[3+4] ;
        affine_dy =affbank[3+8] ;
        affine_dmy =affbank[3+12] ;

        if (oam[0] & 0x200) {
          // double size sprite
          affine_bx = -dim_x  * affine_dx + -dim_y * affine_dmx;
          affine_by = -dim_x  * affine_dy + -dim_y * affine_dmy;
          affine_check_left = -dim_x / 2;
          affine_check_right = affine_check_left + dim_x;
          affine_check_top = -dim_y / 2;
          affine_check_bottom = affine_check_top + dim_y;
          dim_x <<= 1;
          dim_y <<= 1;
        } else {
          // std size sprite. 
          affine_bx = -dim_x/2  * affine_dx + -dim_y/2 * affine_dmx;
          affine_by = -dim_x/2  * affine_dy + -dim_y/2 * affine_dmy;
          affine_check_left = -dim_x / 2;
          affine_check_right = affine_check_left + dim_x;
          affine_check_top = -dim_y / 2;
          affine_check_bottom = affine_check_top + dim_y;
        }
        if (oam[0] & 0x2000) {
          for (int y2 = 0; y2 != dim_y; y2++) {
            intptr_t affine_x_loop = affine_bx;
            intptr_t affine_y_loop = affine_by;

            for (int x2 = 0; x2 != dim_x; x2++) {
              intptr_t xpos = affine_x_loop >> 8;
              intptr_t ypos = affine_y_loop >> 8;
              // check range. 
              if (xpos >= affine_check_left 
                && ypos >= affine_check_top 
                && xpos < affine_check_right
                && ypos < affine_check_bottom)
              {
                intptr_t tile_x = xpos - affine_check_left;
                intptr_t tile_y = ypos - affine_check_top;
                intptr_t tile_mod8_x = tile_x & 7;
                intptr_t tile_mod8_y = tile_y & 7;
                // check range again.
                int xpos_ = x_coo + x2;
                int ypos_ = y_coo + y2;
                xpos_ &= 511;
                ypos_ &= 255;

                if (xpos_ < 240
                  && ypos_ < 160)
                {
                  tile_x >>= 3;
                  tile_y >>= 3;

                  int tid = tile_id + tile_x * 2 + tile_y * tile_pitch;
                  uint8_t *chr=  & m_GBA->gpu.vram[0x10000+tid*32];
                  uint8_t pal_idx = chr[tile_mod8_x+tile_mod8_y*8];

                  if (pal_idx) 
                    dwrite.vptr[ypos_ * dwrite.vpitch16 + xpos_] = bUseBinColor ? shadow_bin_rect : pal256[pal_idx];
                }
              }
              affine_x_loop += affine_dx;
              affine_y_loop += affine_dy;
            }
            affine_bx += affine_dmx;
            affine_by += affine_dmy;
          }
        } else {
          for (int y2 = 0; y2 != dim_y; y2++) {
            intptr_t affine_x_loop = affine_bx;
            intptr_t affine_y_loop = affine_by;

            for (int x2 = 0; x2 != dim_x; x2++) {
              intptr_t xpos = affine_x_loop >> 8;
              intptr_t ypos = affine_y_loop >> 8;
              // check range. 
              if (xpos >= affine_check_left 
                && ypos >= affine_check_top 
                && xpos < affine_check_right
                && ypos < affine_check_bottom)
              {
                intptr_t tile_x = xpos - affine_check_left;
                intptr_t tile_y = ypos - affine_check_top;
                intptr_t tile_mod8_x = tile_x & 7;
                intptr_t tile_mod8_y = tile_y & 7;
                // check range again.
                int xpos_ = x_coo + x2;
                int ypos_ = y_coo + y2;
                xpos_ &= 511;
                ypos_ &= 255;

                if (xpos_ < 240
                  && ypos_ < 160)
                {
                  tile_x >>= 3;
                  tile_y >>= 3;

                  int tid = tile_id + tile_x  + tile_y * tile_pitch;
                  uint8_t *chr=  & m_GBA->gpu.vram[0x10000+tid*32];
                  uint8_t pal_idx = chr[tile_mod8_x/2+tile_mod8_y*4];
                  if (tile_mod8_x & 1)
                    pal_idx = pal_idx>> 4;
                  else 
                    pal_idx = pal_idx & 15;

                  if (pal_idx) 
                    dwrite.vptr[ypos_ * dwrite.vpitch16 + xpos_] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx];
                }
              }
              affine_x_loop += affine_dx;
              affine_y_loop += affine_dy;
            }
            affine_bx += affine_dmx;
            affine_by += affine_dmy;
          }
        }
      } else {
        if (x_coo >= 240 && (x_coo + dim_x) <= 512)
          continue ;
        if (y_coo >= 160 && (y_coo + dim_y) <= 256)
          continue;
        if (oam[0] & 0x2000) {
          // Palette 256
          switch (oam[1] & 0x3000) {
          case 0x0000:
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2 * 2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[x3];
                      if ( xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x1000:
            //  h-swap.
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2 * 2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[7-x3];
                      if ( xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x2000:
             // v-swap
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2 * 2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7 - y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[x3];
                      if ( xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;         
          case 0x3000:
            // v-swap, h-swap.
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2 * 2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[7-x3];
                      if (  xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;  
          default:
            ATLASSERT (FALSE);
            break;
          }
        } else {
          // Palette 16
          switch (oam[1] & 0x3000) {
          case 0x0000:
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[x3];
                      uint8_t pal_idx = pal_mixer & 15;
                      uint8_t pal_idx2= pal_mixer >> 4 & 15;
                      if ( xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx]; }
                      if (xloc_cur2 >= 0 
                        && xloc_cur2 < 240
                        && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x1000:
            //  h-swap.
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[3-x3];
                      uint8_t pal_idx2 = pal_mixer & 15;
                      uint8_t pal_idx= pal_mixer >> 4 & 15;
                      if (  xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx]; }
                      if (xloc_cur2 < 240 && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x2000:
             // v-swap
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[x3];
                      uint8_t pal_idx = pal_mixer & 15;
                      uint8_t pal_idx2= pal_mixer >> 4 & 15;
                      if ( xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx]; }
                      if (xloc_cur2 >= 0 
                        && xloc_cur2 < 240
                        && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;
          case 0x3000:
            // v-swap, h-swap.
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[3-x3];
                      uint8_t pal_idx2 = pal_mixer & 15;
                      uint8_t pal_idx= pal_mixer >> 4 & 15;
                      if (  xloc_cur < 240
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx]; }
                      if (xloc_cur2 >= 0 
                        && xloc_cur2 < 240
                        && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = bUseBinColor ? shadow_bin_rect :pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;
          default:
            ATLASSERT (FALSE);
            break;
          }
        }
      }
    }
  }

  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE) {
    // return ;
    // Only draw oam pixel.
    FillColor (m_GBA->gpu.palette2[512]);

    DirectWrite16 direct_write;
    LockSurface (direct_write);

    for (int Id = 3; Id != -1; Id--) 
      DrawSprite (direct_write, Id << 10);

    // Draw mesh.

    DrawAttachScrRect (direct_write);
    DrawAttachSlot (direct_write);
   
    UnlockSurface ();
    blitRectFromSurface (CRect (m_ptOffset.x, m_ptOffset.y, m_ptOffset.x + t_dwWidth, m_ptOffset.y + t_dwHeight));
  }

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
        // drawALLItem (GetScrollPos (SB_HORZ), GetScrollPos (SB_VERT));
    FillColor (0);
    UpdateGBAItemOut (TRUE);
    // drawALLItem (m_ptOffset.x, m_ptOffset.y);
  }
};


template <DWORD t_dwWidth = 128, DWORD t_dwHeight = 128>
struct CViewOam_T4 :  
  public CDirectWrite16WindowImpl < CViewOam_T4<t_dwWidth, t_dwHeight>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwWidth, 
                                    t_dwHeight, TRUE >{

  typedef CDirectWrite16WindowImpl < CViewOam_T4< t_dwWidth, t_dwHeight>,
                                    ATL::CWindow, 
                                    ATL::CControlWinTraits,  
                                    t_dwWidth, 
                                    t_dwHeight, TRUE> baseClass;
  typedef CViewOam_T4 thisClass;
  
  DECLARE_WND_CLASS(_T ("CViewOam_T4"))

  BEGIN_MSG_MAP(thisClass)
    CHAIN_MSG_MAP(baseClass)
  END_MSG_MAP()

  int m_iSlot;
  gba *m_GBA;
    CViewOam_T4 (void) {}
  ~CViewOam_T4 (void) {}

  void DrawSprite (DirectWrite16 &dwrite) {

    if (!(m_iSlot >= 0 
       && m_iSlot <= 127))
       return ;

    intptr_t dim_x;
    intptr_t dim_y;
    int32_t x_coo;
    int32_t y_coo;
    intptr_t tile_id;
    int32_t tile_pitch;
    uint16_t *pal16;
    uint16_t *pal256 = (uint16_t *) & m_GBA->gpu.palette2[512];
    uint8_t *oamb = & m_GBA->gpu.oam[0];
    intptr_t tid_init;
    intptr_t tid_vphase;

    if (m_GBA->gpu.ctl.blk & 0x40) 
      tile_pitch = 0;
    else 
      tile_pitch = 32;

      uint16_t* oam = (uint16_t *)& oamb[m_iSlot<<3];
      if (       ((oam[0] & 0x300 ) == 0x200)  ||  ((oam[0] & 0xC00) == 0xC00))
        return;

      GetOamDim<intptr_t> (oam, dim_x, dim_y);

      x_coo = 0;
      y_coo = 0; 
      tile_id = oam[2] & 1023;
      pal16 = & pal256[ (oam[2] >> 12) << 4];

      if (oam[0] & 0x2000) { 
         if ((tile_pitch & 32) == 0)
           tile_pitch = dim_x>> 2; 
         /* tile_id &= 1022; */
      } else  if ((tile_pitch & 32) == 0)
           tile_pitch = dim_x>> 3; 

      tid_init = tile_id;
      tid_vphase = tile_pitch;
      if (oam[0] & 0x100) {

        intptr_t affine_bx;
        intptr_t affine_by;
        intptr_t affine_dx;
        intptr_t affine_dy;
        intptr_t affine_dmx;
        intptr_t affine_dmy;
        intptr_t affine_check_left;
        intptr_t affine_check_right;
        intptr_t affine_check_top;
        intptr_t affine_check_bottom;
        int16_t *affbank;

        // Get affine params 
        affbank = (int16_t *)& oamb[(oam[1] >> 9 & 31) << 5];
        affine_dx =affbank[3] ;
        affine_dmx =affbank[3+4] ;
        affine_dy =affbank[3+8] ;
        affine_dmy =affbank[3+12] ;

        if (oam[0] & 0x200) {
          // double size sprite
          affine_bx = -dim_x  * affine_dx + -dim_y * affine_dmx;
          affine_by = -dim_x  * affine_dy + -dim_y * affine_dmy;
          affine_check_left = -dim_x / 2;
          affine_check_right = affine_check_left + dim_x;
          affine_check_top = -dim_y / 2;
          affine_check_bottom = affine_check_top + dim_y;
          dim_x <<= 1;
          dim_y <<= 1;
        } else {
          // std size sprite. 
          affine_bx = -dim_x/2  * affine_dx + -dim_y/2 * affine_dmx;
          affine_by = -dim_x/2  * affine_dy + -dim_y/2 * affine_dmy;
          affine_check_left = -dim_x / 2;
          affine_check_right = affine_check_left + dim_x;
          affine_check_top = -dim_y / 2;
          affine_check_bottom = affine_check_top + dim_y;
        }
        if (oam[0] & 0x2000) {
          for (int y2 = 0; y2 != dim_y; y2++) {
            intptr_t affine_x_loop = affine_bx;
            intptr_t affine_y_loop = affine_by;

            for (int x2 = 0; x2 != dim_x; x2++) {
              intptr_t xpos = affine_x_loop >> 8;
              intptr_t ypos = affine_y_loop >> 8;
              // check range. 
              if (xpos >= affine_check_left 
                && ypos >= affine_check_top 
                && xpos < affine_check_right
                && ypos < affine_check_bottom)
              {
                intptr_t tile_x = xpos - affine_check_left;
                intptr_t tile_y = ypos - affine_check_top;
                intptr_t tile_mod8_x = tile_x & 7;
                intptr_t tile_mod8_y = tile_y & 7;
                // check range again.
                int xpos_ = x_coo + x2;
                int ypos_ = y_coo + y2;
                xpos_ &= 511;
                ypos_ &= 255;

                if (xpos_ < 128
                  && ypos_ < 128)
                {
                  tile_x >>= 3;
                  tile_y >>= 3;

                  int tid = tile_id + tile_x * 2 + tile_y * tile_pitch;
                  uint8_t *chr=  & m_GBA->gpu.vram[0x10000+tid*32];
                  uint8_t pal_idx = chr[tile_mod8_x+tile_mod8_y*8];

                  if (pal_idx) 
                    dwrite.vptr[ypos_ * dwrite.vpitch16 + xpos_] = pal256[pal_idx];
                }
              }
              affine_x_loop += affine_dx;
              affine_y_loop += affine_dy;
            }
            affine_bx += affine_dmx;
            affine_by += affine_dmy;
          }
        } else {
          for (int y2 = 0; y2 != dim_y; y2++) {
            intptr_t affine_x_loop = affine_bx;
            intptr_t affine_y_loop = affine_by;

            for (int x2 = 0; x2 != dim_x; x2++) {
              intptr_t xpos = affine_x_loop >> 8;
              intptr_t ypos = affine_y_loop >> 8;
              // check range. 
              if (xpos >= affine_check_left 
                && ypos >= affine_check_top 
                && xpos < affine_check_right
                && ypos < affine_check_bottom)
              {
                intptr_t tile_x = xpos - affine_check_left;
                intptr_t tile_y = ypos - affine_check_top;
                intptr_t tile_mod8_x = tile_x & 7;
                intptr_t tile_mod8_y = tile_y & 7;
                // check range again.
                int xpos_ = x_coo + x2;
                int ypos_ = y_coo + y2;
                xpos_ &= 511;
                ypos_ &= 255;

                if (xpos_ < 128
                  && ypos_ < 128)
                {
                  tile_x >>= 3;
                  tile_y >>= 3;

                  int tid = tile_id + tile_x  + tile_y * tile_pitch;
                  uint8_t *chr=  & m_GBA->gpu.vram[0x10000+tid*32];
                  uint8_t pal_idx = chr[tile_mod8_x/2+tile_mod8_y*4];
                  if (tile_mod8_x & 1)
                    pal_idx = pal_idx>> 4;
                  else 
                    pal_idx = pal_idx & 15;

                  if (pal_idx) 
                    dwrite.vptr[ypos_ * dwrite.vpitch16 + xpos_] = pal16[pal_idx];
                }
              }
              affine_x_loop += affine_dx;
              affine_y_loop += affine_dy;
            }
            affine_bx += affine_dmx;
            affine_by += affine_dmy;
          }
        }
      } else {
        if (oam[0] & 0x2000) {
          // Palette 256
          switch (oam[1] & 0x3000) {
          case 0x0000:
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2 * 2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 128) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[x3];
                      if ( xloc_cur < 128
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x1000:
            //  h-swap.
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2 * 2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 128) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[7-x3];
                      if (  xloc_cur < 128
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x2000:
             // v-swap
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2 * 2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 128) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7 - y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[x3];
                      if (  xloc_cur < 128
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;         
          case 0x3000:
            // v-swap, h-swap.
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2 * 2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 3];
                    for (int x3 = 0; x3 != 8; x3++) {
                      int xloc_cur = xloc + x3 & 511;
                      uint8_t pal_idx = chr_y[7-x3];
                      if ( xloc_cur < 128
                        && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal256[pal_idx]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;  
          default:
            ATLASSERT (FALSE);
            break;
          }
        } else {
          // Palette 16
          switch (oam[1] & 0x3000) {
          case 0x0000:
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if (yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[x3];
                      uint8_t pal_idx = pal_mixer & 15;
                      uint8_t pal_idx2= pal_mixer >> 4 & 15;
                      if ( xloc_cur < 128 && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal16[pal_idx]; }
                      if ( xloc_cur2 < 128 && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x1000:
            //  h-swap.
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 128) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[3-x3];
                      uint8_t pal_idx2 = pal_mixer & 15;
                      uint8_t pal_idx= pal_mixer >> 4 & 15;
                      if (xloc_cur < 128 && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal16[pal_idx]; }
                      if (xloc_cur2 < 128 && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id += tile_pitch;
            }
            break;
          case 0x2000:
             // v-swap
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = 0; x2 != dim_x >> 3; x2++) {
                int tid = tile_id + x2;
                int xloc = x2 * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[x3];
                      uint8_t pal_idx = pal_mixer & 15;
                      uint8_t pal_idx2= pal_mixer >> 4 & 15;
                      if ( xloc_cur < 240 && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal16[pal_idx]; }
                      if (xloc_cur2 < 240 && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;
          case 0x3000:
            // v-swap, h-swap.
            tile_id = tile_id + ((dim_y >> 3) - 1) * tile_pitch;
            for (int y2 = 0; y2 != dim_y >> 3; y2++) {
              for (int x2 = dim_x/ 8 - 1; x2 != -1; x2--) {
                int tid = tile_id + x2;
                int xloc = (dim_x/ 8 - 1 - x2) * 8 + x_coo;
                int yloc = y2 * 8 + y_coo;
                uint8_t *chr = & m_GBA->gpu.vram[0x10000+tid * 32];
                for (int y3 = 0; y3 != 8; y3++) {
                  int yloc_cur = yloc + y3 & 255;
                  if ( yloc_cur < 160) {
                    uint16_t *vptr16_y = & dwrite.vptr[yloc_cur * dwrite.vpitch16];
                    uint8_t *chr_y = & chr[7-y3 << 2];
                    for (int x3 = 0; x3 != 4; x3++) {
                      int xloc_cur = xloc + x3 * 2 & 511;
                      int xloc_cur2 = xloc + x3 * 2 + 1 & 511;
                      uint8_t pal_mixer = chr_y[3-x3];
                      uint8_t pal_idx2 = pal_mixer & 15;
                      uint8_t pal_idx= pal_mixer >> 4 & 15;
                      if (xloc_cur < 240 && pal_idx) 
                      {  vptr16_y[xloc_cur] = pal16[pal_idx]; }
                      if (xloc_cur2 < 240 && pal_idx2) 
                      {  vptr16_y[xloc_cur2] = pal16[pal_idx2]; }
                    }
                  }
                }
              }
              tile_id -= tile_pitch;
            }
            break;
          default:
            ATLASSERT (FALSE);
            break;
          }
        }
      }
  }

  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE) {
    // return ;
    // Only draw oam pixel.
    FillColor (m_GBA->gpu.palette2[512]);

    DirectWrite16 direct_write;
    LockSurface (direct_write);

    DrawSprite (direct_write);

    UnlockSurface ();
    blitRectFromSurface (CRect (m_ptOffset.x, m_ptOffset.y, m_ptOffset.x + t_dwWidth, m_ptOffset.y + t_dwHeight));
  }

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
    UpdateGBAItemOut (TRUE);
  }
};

template <class TRefto>
struct CViewOamDialogInfos : public CDialogImpl<CViewOamDialogInfos<TRefto>>
                           , public CScrollImpl<CViewOamDialogInfos<TRefto>>
                           , public CDynamicMessageMap<CViewOamDialogInfos<TRefto>, TRefto>
{
  enum { IDD = IDD_GPU_OAM };

  DECLARE_WND_CLASS(_T ("CViewOamDialogInfos"))
  SET_DYNA_MSG_MAP2(CScrollImpl<CViewOamDialogInfos<TRefto>>)

    CViewOamDialogInfos (TRefto *TThis_) :
       CDynamicMessageMap<CViewOamDialogInfos<TRefto>, TRefto> (this, TThis_),
    m_GuiLV_Elem (TThis_) {}

  // CViewOamDialogInfos:
  void Collect_SubclassItemHandle (void) {
    m_GuiLV_Elem.SubclassWindow (GetDlgItem (IDLV_OAM_ELEM));

    m_GuiST_SpSlot = GetDlgItem (IDST_OAM_SP_SLOT);
    m_GuiST_OamAddr = GetDlgItem (IDST_OAM_OAM_ADDR);
    m_GuiST_SlotPos = GetDlgItem (IDST_OAM_SLOT_POS);
    m_GuiST_Size = GetDlgItem (IDST_OAM_SIZE);
    m_GuiST_SpPos = GetDlgItem (IDST_OAM_SP_POS);
    m_GuiST_Pri = GetDlgItem (IDST_OAM_PRI);
    m_GuiST_Tid = GetDlgItem (IDST_OAM_TID);
    m_GuiST_Mode = GetDlgItem (IDST_OAM_MODE);
    m_GuiST_ChrAddr = GetDlgItem (IDST_OAM_CHR_ADDR);
    m_GuiST_TMapper = GetDlgItem (IDST_OAM_TMAPPER);
    m_GuiST_Raw = GetDlgItem (IDST_OAM_RAW);
    m_GuiST_Win0 = GetDlgItem (IDST_OAM_WIN0);
    m_GuiST_Win1 = GetDlgItem (IDST_OAM_WIN1);
    m_GuiST_Palbank = GetDlgItem (IDST_OAM_PALBANK);
    m_GuiST_PA = GetDlgItem (IDST_OAM_PA);
    m_GuiST_PB = GetDlgItem (IDST_OAM_PB);
    m_GuiST_PC = GetDlgItem (IDST_OAM_PC);
    m_GuiST_PD = GetDlgItem (IDST_OAM_PD);

    m_GuiST_L2_InTile = GetDlgItem (IDST_OAM_L2_INTILE);
    m_GuiST_L2_InOam = GetDlgItem (IDST_OAM_L2_INOAM);
    m_GuiST_L2_InGloabl = GetDlgItem (IDST_OAM_L2_INGLO);
    m_GuiST_L2_Chr = GetDlgItem (IDST_OAM_L2_CHR);
    m_GuiST_L2_Pixel = GetDlgItem (IDST_OAM_L2_PIXEL);
    m_GuiST_L2_BGR = GetDlgItem (IDST_OAM_L2_BGR);
    // LVS_EX_DOUBLEBUFFER
    ModifyStyleEx (0, WS_EX_TRANSPARENT | WS_EX_COMPOSITED);
    // m_GuiLV_Elem.ModifyStyleEx (0, WS_EX_TRANSPARENT | WS_EX_COMPOSITED);
    // DWORD dwSourceStyle = m_GuiLV_Elem.GetExtendedListViewStyle ();

    // m_GuiLV_Elem.SetExtendedListViewStyle (dwSourceStyle | LVS_EX_DOUBLEBUFFER);
    // LVS_EX_COLUMNOVERFLOW


  }

  CStatic m_GuiST_SpSlot;
  CStatic m_GuiST_OamAddr;
  CStatic m_GuiST_SlotPos;
  CStatic m_GuiST_Size;
  CStatic m_GuiST_SpPos;
  CStatic m_GuiST_Pri;
  CStatic m_GuiST_Tid;
  CStatic m_GuiST_Mode;
  CStatic m_GuiST_ChrAddr;
  CStatic m_GuiST_TMapper;
  CStatic m_GuiST_Raw;
  CStatic m_GuiST_Win0;
  CStatic m_GuiST_Win1;
  CStatic m_GuiST_Palbank;
  CStatic m_GuiST_PA;
  CStatic m_GuiST_PB;
  CStatic m_GuiST_PC;
  CStatic m_GuiST_PD;
  CStatic m_GuiST_L2_InTile;
  CStatic m_GuiST_L2_InOam;
  CStatic m_GuiST_L2_InGloabl;
  CStatic m_GuiST_L2_Pixel;
  CStatic m_GuiST_L2_Chr;
  CStatic m_GuiST_L2_BGR;
  CListCtrlREF<TRefto> m_GuiLV_Elem;

  gba *m_GBA;

  float GBAInt16_2Flt (int16_t val) {
    int32_t vcalc = val;
    vcalc <<= 8;
    float fcalc = (float) vcalc;
    return fcalc/ 256.0 / 256.0;
  }

  void UpdateOutput (int iSlot) {

    if (iSlot >= 0
      && iSlot <= 127)
    {
      uint16_t *bank = (uint16_t *) & m_GBA->gpu.oam[iSlot << 3];
      uint16_t dim_x;
      uint16_t dim_y;
      CSetWindowText (m_GuiST_SpSlot, _T ("sp-slot@0x%02X,%d"), iSlot, iSlot);
      CSetWindowText (m_GuiST_SpPos, _T ("sp-pos@%d, %d"), bank[1] & 511, bank[0] & 255);
      CSetWindowText (m_GuiST_SlotPos, _T ("slot-pos@%d, %d"), iSlot & 15, iSlot >> 4);
      CSetWindowText (m_GuiST_Tid, _T ("tid@0x%03X,%d"), bank[2] & 1023, bank[2] & 1023, bank[2] & 1023, bank[2] & 1023);
      CSetWindowText (m_GuiST_ChrAddr, _T ("caddr@0x%05X"), (bank[2] & 1023) *32 + 0x10000);
      CSetWindowText (m_GuiST_Raw, _T ("raw@0x%04X, 0x%04X, 0x%04X"), bank[0], bank[1], bank[2]);
      CSetWindowText (m_GuiST_OamAddr, _T ("oaddr@0x%03X"), iSlot << 3);

      GetOamDim<uint16_t> (bank, dim_x, dim_y);
      CSetWindowText (m_GuiST_Size, _T ("size@%d, %d"), dim_x, dim_y);
      CSetWindowText (m_GuiST_Pri, _T ("pri@%d"), bank[2] >> 10 & 3);

      switch (bank[0] & 0xC00) {
      case 0x000:
        CSetWindowText (m_GuiST_Mode, _T ("mode@STD"));
        break;
      case 0x400:
        CSetWindowText (m_GuiST_Mode, _T ("mode@BLEND"));
        break;
      case 0x800:
        CSetWindowText (m_GuiST_Mode, _T ("mode@OBJWIN"));
        break;
      case 0xC00:
        CSetWindowText (m_GuiST_Mode, _T ("mode@DIS/DOUBLE"));
        break;
      default:
        ATLASSERT (FALSE);
      }
      if (m_GBA->gpu.ctl.blk & 0x40) 
        CSetWindowText (m_GuiST_TMapper, _T ("tmap@CONTINUOUS"));
      else 
        CSetWindowText (m_GuiST_TMapper, _T ("tmap@PITCH"));

      CButton (GetDlgItem (IDCK_OAM_HFLIP)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDCK_OAM_VFLIP)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDCK_OAM_MOSAIC)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDRD_OAM_8BIT)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDRD_OAM_4BIT)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDCK_OAM_DMAP)).SetCheck (BST_UNCHECKED);
      CButton (GetDlgItem (IDCK_OAM_AFFINE)).SetCheck (BST_UNCHECKED);

      if (bank[0] & 0x1000)
        CButton (GetDlgItem (IDCK_OAM_MOSAIC)).SetCheck (BST_CHECKED);
      if (bank[0] & 0x2000) {
        :: CheckDlgButton (*this, IDRD_OAM_8BIT, BST_CHECKED);
        CSetWindowText (m_GuiST_Palbank, _T ("palbank@nullptr"));
      } else {
        :: CheckDlgButton (*this, IDRD_OAM_4BIT, BST_CHECKED);
        CSetWindowText (m_GuiST_Palbank, _T ("palbank@%X"), bank[2] >> 12 & 15);
      }
      if (bank[0] & 0x100) {
        CButton (GetDlgItem (IDCK_OAM_AFFINE)).SetCheck (BST_CHECKED);

        if (bank[0] & 0x200) {
          CButton (GetDlgItem (IDCK_OAM_DMAP)).SetCheck (BST_CHECKED);
        } else {
          CButton (GetDlgItem (IDCK_OAM_DMAP)).SetCheck (BST_UNCHECKED);
        }
        int16_t *affbank = (int16_t *)& m_GBA->gpu.oam[(bank[1] >> 9 & 31) << 5];
        int16_t affine_dx =affbank[3] ;
        int16_t affine_dmx =affbank[3+4] ;
        int16_t affine_dy =affbank[3+8] ;
        int16_t affine_dmy =affbank[3+12] ;

        CSetWindowText (m_GuiST_PA, _T ("pa@%X, %f"), affine_dx, GBAInt16_2Flt (affine_dx));
        CSetWindowText (m_GuiST_PB, _T ("pb@%X, %f"), affine_dmx, GBAInt16_2Flt (affine_dmx) );
        CSetWindowText (m_GuiST_PC, _T ("pc@%X, %f"), affine_dy, GBAInt16_2Flt (affine_dy));
        CSetWindowText (m_GuiST_PD, _T ("pd@%X, %f"), affine_dmy, GBAInt16_2Flt (affine_dmy));
      } else {
        CButton (GetDlgItem (IDCK_OAM_AFFINE)).SetCheck (BST_UNCHECKED);
        CButton (GetDlgItem (IDCK_OAM_DMAP)).SetCheck (BST_UNCHECKED);

        m_GuiST_PA.SetWindowText (_T ("pa@nullptr"));
        m_GuiST_PB.SetWindowText (_T ("pb@nullptr"));
        m_GuiST_PC.SetWindowText (_T ("pc@nullptr"));
        m_GuiST_PD.SetWindowText (_T ("pd@nullptr"));

        if (bank[1] & 0x1000)
          CButton (GetDlgItem (IDCK_OAM_HFLIP)).SetCheck (BST_CHECKED);
        if (bank[1] & 0x2000)
          CButton (GetDlgItem (IDCK_OAM_VFLIP)).SetCheck (BST_CHECKED);
      }
      for (int Id = 0; Id != 2; Id++) {
        const uint32_t winio_base = 0x4000040 + Id * 2;
        const uint16_t rl = gba_fast_rhw (m_GBA, winio_base);
        const uint16_t bt = gba_fast_rhw (m_GBA, winio_base + 4);
        const uint16_t ctl = gba_fast_rhw (m_GBA, 0x4000048);
        const uint16_t master_ctl = gba_fast_rhw (m_GBA, 0x4000000);

        if ( ! ( (master_ctl >> 13 + Id & 1)
                  && (ctl >> Id * 8 & 1 << 4) ))
        {
          if (Id == 0)
            m_GuiST_Win0.SetWindowText (_T ("win0@nullptr"));
          else 
            m_GuiST_Win1.SetWindowText (_T ("win1@nullptr"));
        }
        else 
        {
          uint16_t right = rl & 255;
          uint16_t left = rl >> 8 & 255;
          uint16_t bottom = bt & 255;
          uint16_t top = bt >> 8 & 255;

          if (Id == 0)
            CSetWindowText (m_GuiST_Win0, _T ("win0@%d,%d,%d,%d"), left, right, top, bottom);
          else 
            CSetWindowText (m_GuiST_Win1, _T ("win1@%d,%d,%d,%d"), left, right, top, bottom);
        }
      }
    }
  }

  void DoPaint(CDCHandle /*dc*/) {}
};


template <int t_iMenuId>
class CViewOam : public dockwins::CDockingTabChild<CViewOam<t_iMenuId>> {
public:
  CPoint m_ptTrack;
  DWORD m_dwClrMesh;
  DWORD m_dwClrTrack;

  CMenuHandle m_AttachMenu;
  CViewOamOverview<CViewOam<t_iMenuId>, CViewOamOverview<CViewOam, 0, 0>::squmesh_size * 16 + 17, CViewOamOverview<CViewOam, 0, 0>::squmesh_size * 8 + 9> m_ContainerOverview;
  // slot buffer 
  CViewOamDialogInfos<CViewOam<t_iMenuId>> m_ContainerInfosPane; // attr pane.
  CViewOamShadow<240, 160> m_ContainerWidgets3; // screen buffer
  CViewOamShadow<240, 160, TRUE> m_ContainerWidgets3_5; // shadow buffer
  CViewOamRaw<CViewOam<t_iMenuId>, 64, 64> m_ContainerOamRaw; // single std draw buffer
  CViewOam_T4<> m_ContainerWidgets5; // affine draw buffer 

// ---------------------------------------------------------------------------------
// Sprite slot                                   | 
// ----------------------------------------|  
//                       |                          |
//                       | Common pixel   | Attr pane
// Screen buffer  |--------------------|
//   ---------------  |                       |
// Shadow buffer |  Affine pixel  |
// ----------------------------------------------------------------

  CSplitterWindowT<true> m_GuiSWinSplitter_Level; // xx | Attr pane. 
  CSplitterWindowT<false> m_GuiSWinSplitter_Level2; //  Sprite slot
                                                                                    // -------------
                                                                                    //       xx 
  CSplitterWindowT<true> m_GuiSWinSplitter_Level2_5; 
  CSplitterWindowT<false> m_GuiSWinSplitter_Level3; // Screen buffer | xx 
  CSplitterWindowT<false> m_GuiSWinSplitter_Level4; //  Common pixel
                                                                                    // -------------
                                                                                    //       Affine pixel 
  typedef    dockwins::CDockingTabChild<CViewOam<t_iMenuId>> baseClass;
  typedef CViewOam  thisClass;

  // CViewOam (void) : m_ContainerOverview (this), m_ContainerInfosPane (this) {}
 CViewOam (void) : m_ContainerInfosPane (this), m_ContainerOverview (*this), m_ContainerOamRaw (*this), m_ptTrack (0, 0),
  m_dwClrMesh (0), m_dwClrTrack (0xffffff) {}
 ~CViewOam (void) {}

  LRESULT OnResponseOamRaw  (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
    
    BOOL bLockCheck;

    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      bLockCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_LOCK2)).GetCheck () == BST_CHECKED;

      if ( (bLockCheck
        && uMsg == WM_LBUTTONUP)
        ||  (bLockCheck == FALSE
        && uMsg == WM_MOUSEMOVE) )
      {
        // Update track pos.
        m_ContainerOamRaw.m_ptTrack.x = LOWORD (lParam); 
        m_ContainerOamRaw.m_ptTrack.y = HIWORD (lParam);
        m_ContainerOamRaw.m_ptTrack += m_ContainerOamRaw.m_ptOffset;

        UpdateGBAItemOut (TRUE);
      }
    default:
      bHandled = FALSE;
      return 0;
    }
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnResponseOamOverView (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
    
    BOOL bLockCheck;

    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      bLockCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_LOCK)).GetCheck () == BST_CHECKED;

      if ( (bLockCheck
        && uMsg == WM_LBUTTONUP)
        ||  (bLockCheck == FALSE
        && uMsg == WM_MOUSEMOVE) )
      {
        // Update track pos.
        m_ContainerOverview.m_ptLock.x = LOWORD (lParam); 
        m_ContainerOverview.m_ptLock.y = HIWORD (lParam);
        m_ContainerOverview.AdjustTrackPos (m_ContainerOverview.m_ptLock);

        int iSlot = m_ContainerOverview.m_ptLock.x + m_ContainerOverview.m_ptLock.y * 16;
        if (iSlot > 127)
          iSlot = 127;

        m_ContainerOamRaw.m_iSlot =
        m_ContainerWidgets5.m_iSlot = 
        m_ContainerOamRaw.m_iSlot = 
        m_ContainerWidgets3_5.m_iSlot =
        m_ContainerWidgets3.m_iSlot = iSlot;

        UpdateGBAItemOut (TRUE);
      }
    default:
      bHandled = FALSE;
      return 0;
    }
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 

    CRect rcClient;
    static const uintptr_t lim = m_ContainerOverview.squmesh_size + 1;
    CPoint pt;
    CSize vec;
    m_ContainerOverview.GetClientRect (rcClient);

    if (rcClient.Width () <= 0 
      || rcClient.Height () <= 0)
      return 0;

    pt.x = (m_ContainerOverview.m_ptOffset.x + LOWORD(lParam)) / lim;
    pt.y = (m_ContainerOverview.m_ptOffset.y + HIWORD(lParam)) / lim;

    int iSlot = pt.x + pt.y * 16;
    if (iSlot > 127)
      iSlot = 127;

    m_ContainerOamRaw.m_iSlot =
    m_ContainerWidgets5.m_iSlot = 
    m_ContainerOamRaw.m_iSlot = 
    m_ContainerWidgets3_5.m_iSlot =
    m_ContainerWidgets3.m_iSlot = iSlot;
    m_ContainerInfosPane.UpdateOutput (iSlot);

    return 0;
  }
  DECLARE_WND_CLASS_FX (_T ("CViewOam"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, IDI_OAM)

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
    m_GuiSWinSplitter_Level.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 0;
  }
  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rcClient;
    GetClientRect (rcClient);

    static const DWORD commonSpliterStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    m_GuiSWinSplitter_Level.Create (m_hWnd, rcClient, _T (""), commonSpliterStyle, WS_EX_TRANSPARENT);
    m_GuiSWinSplitter_Level2.Create (m_GuiSWinSplitter_Level, rcClient, _T (""), commonSpliterStyle, WS_EX_TRANSPARENT);
    m_GuiSWinSplitter_Level2_5.Create (m_GuiSWinSplitter_Level2, rcClient, _T (""), commonSpliterStyle, WS_EX_TRANSPARENT); 
    m_GuiSWinSplitter_Level3.Create (m_GuiSWinSplitter_Level2_5, rcClient, _T (""), commonSpliterStyle, WS_EX_TRANSPARENT); 
    m_GuiSWinSplitter_Level4.Create (m_GuiSWinSplitter_Level2_5, rcClient, _T (""), commonSpliterStyle, WS_EX_TRANSPARENT); 

    static const DWORD commonPaneStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    m_ContainerInfosPane.Create (m_GuiSWinSplitter_Level, rcDefault);
    m_ContainerOverview.Create (m_GuiSWinSplitter_Level2, rcDefault, _T (""), commonPaneStyle, WS_EX_TRANSPARENT);
    m_ContainerWidgets3.Create (m_GuiSWinSplitter_Level3, rcDefault, _T (""), commonPaneStyle, WS_EX_TRANSPARENT);
    m_ContainerWidgets3_5.Create (m_GuiSWinSplitter_Level3, rcDefault, _T (""), commonPaneStyle, WS_EX_TRANSPARENT);
    m_ContainerOamRaw.Create (m_GuiSWinSplitter_Level4, rcDefault, _T (""), commonPaneStyle, WS_EX_TRANSPARENT);
    m_ContainerOamRaw.SetCallbackFilter (& CViewOam::OnResponseOamRaw);
    m_ContainerWidgets5.Create (m_GuiSWinSplitter_Level4, rcDefault, _T (""), commonPaneStyle, WS_EX_TRANSPARENT);
    m_ContainerInfosPane.ShowWindow (SW_SHOWNORMAL);
    m_ContainerInfosPane.Collect_SubclassItemHandle ();
    m_ContainerOverview.SetCallbackFilter (& CViewOam::OnResponseOamOverView);

    m_ContainerInfosPane.GetClientRect (& rcClient);
    m_ContainerInfosPane.SetScrollSize (rcClient.Width (), rcClient.Height ());

    m_ContainerInfosPane.add_msg_map (  WM_COMMAND, &CViewOam::OnButtonCommand);
    // m_ContainerInfosPane.add_msg_map (  WM_CTLCOLORSTATIC, &CViewOam::OnTransStatic);
    // m_ContainerInfosPane.add_msg_map (  WM_CTLCOLORDLG, &CViewOam::OnTransDlg);

    m_ContainerInfosPane.m_GuiLV_Elem.add_msg_map (  WM_MOUSEMOVE, &CViewOam::OnListViewMouseMessage);
    m_ContainerInfosPane.m_GuiLV_Elem.add_msg_map (  WM_LBUTTONUP, &CViewOam::OnListViewMouseMessage);
    m_ContainerInfosPane.m_GuiLV_Elem.add_msg_map (  WM_PAINT, &CViewOam::OnPaintListView);


    // OnTransDlg
    // set pane. 
    m_GuiSWinSplitter_Level.SetSplitterPanes (m_GuiSWinSplitter_Level2, m_ContainerInfosPane);
    m_GuiSWinSplitter_Level2.SetSplitterPanes (m_ContainerOverview, m_GuiSWinSplitter_Level2_5);
    m_GuiSWinSplitter_Level2_5.SetSplitterPanes (m_GuiSWinSplitter_Level3, m_GuiSWinSplitter_Level4);
    m_GuiSWinSplitter_Level3.SetSplitterPanes (m_ContainerWidgets3, m_ContainerWidgets3_5);
    m_GuiSWinSplitter_Level4.SetSplitterPanes (m_ContainerOamRaw, m_ContainerWidgets5);

    ResizeClient (720, 620);

    m_GuiSWinSplitter_Level.SetSplitterPos (720 - 272);
    m_GuiSWinSplitter_Level2.SetSplitterPos (250);
    m_GuiSWinSplitter_Level2_5.SetSplitterPos (250);
    m_GuiSWinSplitter_Level3.SetSplitterPos (186);
    m_GuiSWinSplitter_Level4.SetSplitterPos (72);
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (t_iMenuId, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE) {
    if (isClientRectEmpty (*this) != FALSE)
      return ;

    if (bForceUpdate
      || CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_QFLUSH)).GetCheck () == BST_CHECKED)
    {
      if (CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_SHADOW_BIN)).GetCheck () == BST_CHECKED) 
        m_ContainerWidgets3_5.m_bShadowBinrary = TRUE;
      else 
        m_ContainerWidgets3_5.m_bShadowBinrary = FALSE;
      m_ContainerOverview.UpdateGBAItemOut ();
      m_ContainerWidgets3.UpdateGBAItemOut ();
      m_ContainerWidgets3_5.UpdateGBAItemOut ();
      m_ContainerOamRaw.UpdateGBAItemOut ();
      m_ContainerWidgets5.UpdateGBAItemOut ();
      m_ContainerInfosPane.UpdateOutput (m_ContainerWidgets5.m_iSlot);
      m_ContainerInfosPane.m_GuiLV_Elem.InvalidateRect (NULL, TRUE);
    }
  }

  LRESULT OnListViewMouseMessage (CDnMsg<CViewOam>* pMsg, BOOL& bHandled) {
    BOOL bLockCheck;
    switch (pMsg->msg) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
      bLockCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_LOCK3)).GetCheck () == BST_CHECKED;

      if ( (bLockCheck
        && pMsg->msg == WM_LBUTTONUP)
        ||  (bLockCheck == FALSE
        && pMsg->msg == WM_MOUSEMOVE) )
      {
        // Update track pos.
        m_ptTrack.x = LOWORD (pMsg->lparam); 
        m_ptTrack.y = HIWORD (pMsg->lparam);
        //  m_ptTrack += m_ContainerOamRaw.m_ptOffset;

        UpdateGBAItemOut (TRUE);
      }

    default:
      bHandled = FALSE;
      return 0;
    }
    // Update Item.
    UpdateGBAItemOut (TRUE);
    return 0;
  }
  LRESULT OnTransStatic (CDnMsg<CViewOam>* pMsg, BOOL& bHandled) {
    bHandled = TRUE;

    CDCHandle handle = (HDC) pMsg->wparam;
    handle.SetBkMode (TRANSPARENT);
    return (LRESULT)(HBRUSH)::GetStockObject(NULL_BRUSH);

  }
  LRESULT OnPaintListView  (CDnMsg<CViewOam>* pMsg, BOOL& bHandled) {
    bHandled = TRUE;
    CPaintDC dc (pMsg->from);
  //  dc.FillSolidRect (0, 0, 48, 48, 0);
      // Update pane and pixel output 
      int iSlot = m_ContainerWidgets5.m_iSlot;
      if (iSlot > 127)
        iSlot = 127;
      if (iSlot < 0)
        iSlot = 0;

      int iInPosX = m_ContainerOamRaw.m_ptTrack.x;
      int iInPosY = m_ContainerOamRaw.m_ptTrack.y;
      int iOamElemX;
      int iOamElemY;

      uint16_t *oam = (uint16_t *)& m_ContainerOamRaw.m_GBA->gpu.oam[iSlot<<3];

      GetOamDim<int> (oam, iOamElemX, iOamElemY);

      if (iInPosX < 0)
        iInPosX = 0;
      if (iInPosY < 0)
        iInPosY = 0;
      if (iInPosX >= iOamElemX)
        iInPosX = iOamElemX - 1;
      if (iInPosY >= iOamElemY)
        iInPosY = iOamElemY - 1;
      iInPosX >>= 3;
      iInPosY >>= 3;

      // size := 11
      // Pos := Pos in tile range 
      // Pos2 := Pos in tile range + global offset 
      // Chr := current chr addr 
      // Pixel := palette bank 
      // 
      int iMiniX = m_ptTrack.x / 11;
      int iMiniY = m_ptTrack.y / 11;
      int iPitch;
      if (iMiniX < 0)
        iMiniX = 0;
      if (iMiniX >= 8)
        iMiniX = 7;
      if (iMiniY < 0)
        iMiniY = 0;
      if (iMiniY >= 8)
        iMiniY = 7;

      CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_InTile, _T ("InTile@%d, %d"), iMiniX, iMiniY);
      CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_InOam, _T ("InOam@%d, %d, %d, %d"), iInPosX, iInPosY, iInPosX * 8, iInPosY * 8);
      CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_InGloabl, _T ("InGlobal@%d, %d"),  (oam[1] & 511) +  iInPosX * 8 + iMiniX, (oam[0] & 255) + iInPosY * 8 + iMiniY);

      int itemPos = iInPosX; 
      int iTid = oam[2] & 1023;
      if (m_ContainerInfosPane.m_GBA->gpu.ctl.blk & 0x40) {
        if (oam[0] & 0x2000)
        iPitch = iOamElemX / 8 * 2;
        else 
          iPitch = iOamElemX/ 8;
      } else {
        iPitch = 32;
      }
      
      // ListView pixel.
      // 11 - 10
      
      dc.FillSolidRect (0, 0, 89, 89, m_dwClrMesh);
      BOOL bCheck = CButton (m_ContainerInfosPane.GetDlgItem (IDCK_OAM_ELEM_BIN)).GetCheck () == BST_CHECKED;
      
      uint32_t chr_addr;
      if (oam[0] & 0x2000) {
        int iTidCur = iTid + iPitch * iInPosY + iInPosX * 2;
        uint32_t *palbank = (uint32_t *) & m_ContainerInfosPane.m_GBA->gpu.palette3[512];
        uint16_t *palbank2 = (uint16_t *) & m_ContainerInfosPane.m_GBA->gpu.palette[512];
        chr_addr = 0x10000+iTidCur * 32;
        uint8_t *pixel = & m_ContainerInfosPane.m_GBA->gpu.vram[chr_addr];
        // Draw pixel . 8 bit
        for (int y = 0; y != 8; y++) {
          for (int x = 0; x != 8; x++) {
            int pix = palbank[pixel[y*8+x]];
            int x2 = x * 11 + 1;
            int y2 = y * 11 + 1;
            if (bCheck)
              if (pix)
                dc.FillSolidRect (x2, y2, 10, 10, 0);
              else 
                dc.FillSolidRect (x2, y2, 10, 10, 0xffffff);
            else 
            dc.FillSolidRect (x2, y2, 10, 10, pix);
          }
        }
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_Chr, _T ("Chr@%05X"), chr_addr + iMiniY * 8 + iMiniX);
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_Pixel, _T ("Pix@%02X"), pixel[iMiniY * 8 + iMiniX]);
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_BGR, _T ("BGR@%04X"), palbank2[pixel[iMiniY * 8 + iMiniX]]);
      } else {
        int iTidCur = iTid + iPitch * iInPosY + iInPosX;
        chr_addr = 0x10000+iTidCur * 32;
        uint8_t *pixel = & m_ContainerInfosPane.m_GBA->gpu.vram[chr_addr];
        uint32_t *palbank = (uint32_t *) & m_ContainerInfosPane.m_GBA->gpu.palette3[512 + (oam[2] >> 12 & 15) * 32];
        uint16_t *palbank2 = (uint16_t *) & m_ContainerInfosPane.m_GBA->gpu.palette[512 + (oam[2] >> 12 & 15) * 32];
        // Draw pixel . 4 bit
        for (int y = 0; y != 8; y++) {
          for (int x = 0; x != 4; x++) {
            int pix = pixel[y*4+x];
            int x2 = x * 22 + 1;
            int y2 = y * 11 + 1;
            if (bCheck != FALSE) {
              if (pix &15)

            dc.FillSolidRect (x2, y2, 10, 10, 0);
              else 
                dc.FillSolidRect (x2, y2, 10, 10, 0xffffff);
              if (pix >> 4 &15)

           dc.FillSolidRect (x2 + 11, y2, 10, 10, 0);
              else 
                dc.FillSolidRect (x2 + 11, y2, 10, 10, 0xffffff);
            } else {
            dc.FillSolidRect (x2, y2, 10, 10, palbank[pix &15]);
            dc.FillSolidRect (x2 + 11, y2, 10, 10, palbank[pix >> 4 &15]);
            }
          }
        }
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_Chr, _T ("Chr@%05X:%s"),
          chr_addr + iMiniY * 4 + iMiniX / 2,
            (iMiniX & 1) ? _T ("HI") : _T ("LO") );
        int t = pixel[iMiniY * 4 + iMiniX/2];
        if (iMiniX & 1)
          t = t >> 4 & 15;
        else 
          t &= 15;
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_Pixel, _T ("Pix@%01X"), t);
        CSetWindowText (m_ContainerInfosPane.m_GuiST_L2_BGR, _T ("BGR@%04X"), palbank2[t]);
      }
      int x2 = iMiniX * 11;
      int y2 = iMiniY * 11;
      
      dc.FrameRect (
        & CRect (x2, y2, x2 + 12, y2 + 12),
          CBrush (CreateSolidBrush (m_dwClrTrack)));
    return 0;

   // return (LRESULT)(HBRUSH)::GetStockObject(WHITE
    //  _BRUSH);

  }
  LRESULT OnTransDlg (CDnMsg<CViewOam>* pMsg, BOOL& bHandled) {
    bHandled = FALSE;

    CDCHandle handle = (HDC) pMsg->wparam;
    // handle.SetBkMode (TRANSPARENT);
    return (LRESULT)(HBRUSH)::GetStockObject(WHITE_BRUSH);

  }
  LRESULT OnButtonCommand (CDnMsg<CViewOam>* pMsg, BOOL& bHandled) {
    PDWORD pdwCol = NULL;
    switch (LOWORD(pMsg->wparam)) {
    case IDBT_OAM_OVERVIEW_GRID_COL:
      pdwCol = & m_ContainerOverview.m_dwClrMesh;
      break;
    case IDBT_OAM_OVERVIEW_LOCK_COL:
      pdwCol = & m_ContainerOverview.m_dwClrLock;
      break;
    case IDBT_OAM_OVERVIEW_SHADOW_COL:
      pdwCol = & m_ContainerOverview.m_dwClrShadow;
      break;
    case IDBT_OAM_BT_ELEM_GRID_COL:
      pdwCol = & m_dwClrMesh;
      break;
    case IDBT_OAM_BT_ELEM_LOCK_COL:
      pdwCol = & m_dwClrTrack;
      break;
    default:
      bHandled = FALSE;
      return 0;
    }
    ATLASSERT (pdwCol != NULL);
    CColorDialog col_dlg (*pdwCol);
    if (col_dlg.DoModal () == IDOK)
      * pdwCol = Col_GDI32_2DirectDraw16 (col_dlg.GetColor ());
    if (LOWORD(pMsg->wparam) == 
      IDBT_OAM_BT_ELEM_GRID_COL 
      || LOWORD(pMsg->wparam) == 
      IDBT_OAM_BT_ELEM_LOCK_COL)
      * pdwCol = col_dlg.GetColor ();
    // Update Item.
    UpdateGBAItemOut (TRUE);
    return 0;
  }

  void kCreate (HWND parent, gba *attachGBA) {
    m_ContainerOverview.m_GBA = 
    m_ContainerInfosPane.m_GBA =
    m_ContainerWidgets3_5.m_GBA = 
    m_ContainerWidgets3.m_GBA =
    m_ContainerOamRaw.m_GBA = 
    m_ContainerWidgets5.m_GBA = attachGBA;
    Create (parent, CRect (0,0, 320, 240), _T ("Oam"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  WS_EX_TRANSPARENT);
  }
};

# endif 