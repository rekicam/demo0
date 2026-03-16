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

// TODO: More case, and repeat code .
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CVIEWTILE_INCLUED_HEADER_CXX_)
#  define _CVIEWTILE_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <tchar.h>
# include "..\resource.h"

template <DWORD t_dwBaseAddr> /* t_dwBaseAddr := 0~0x17FFF */
struct CViewTile_T : public CDialogImpl<CViewTile_T<t_dwBaseAddr>>
                           , public CScrollImpl<CViewTile_T<t_dwBaseAddr>>
{
  enum { IDD = IDD_GPU_CHRBASE };

  typedef CDialogImpl<CViewTile_T<t_dwBaseAddr>> baseClass;
  typedef CScrollImpl<CViewTile_T<t_dwBaseAddr>> baseClass2;
  typedef CViewTile_T thisClass;
  
  // https://stackoverflow.com/questions/4057948/initializing-a-member-array-in-constructor-initializer
  CListCtrlREF<CViewTile_T> *m_GuiLV_ChrGroup [4];
  CDC m_MemDibDraw[4];
  HGDIOBJ m_SaveOldGDIObj[4];
  DWORD *m_MemDibBuffer[4];
  CPoint m_ptTrack;
  CPoint m_ptTrack2;
  PBYTE m_pbAttachChr;
  BOOL m_bAttach8Bit;
  DWORD m_dwAttachAddr;
  DWORD m_dwItemAddr;
  DWORD m_dwItemAddr2;
  DWORD m_dwItemTID;
  DWORD m_dwItemPos;
  DWORD m_dwItemPix;
  DWORD m_dwItemBGR;

  CListCtrlREF<CViewTile_T> m_GuiLV_ChrCanvas;
  CListCtrlREF<CViewTile_T> m_GuiLV_ChrCanvas2;
  CListCtrlREF<CViewTile_T> m_GuiLV_ChrCanvas3;
  CListCtrlREF<CViewTile_T> m_GuiLV_ChrCanvas4;
  HWND m_GuiLV_AttachCursor;

  CStatic m_GuiST_Addr;
  CStatic m_GuiST_Index;
  CStatic m_GuiST_Addr2;
  CStatic m_GuiST_Tid;
  CStatic m_GuiST_Pos;
  CStatic m_GuiST_Pix;
  CStatic m_GuiST_BGR;

  CButton m_GuiRD_4BitPAL;
  CButton m_GuiBT_Grid;
  CButton m_GuiBT_QFlush;
  CButton m_GuiBT_Offset1;
  CButton m_GuiBT_Lock;
  CButton m_GuiBT_Lock2;
  CListCtrlREF<CViewTile_T<t_dwBaseAddr>> m_GuiLV_Pixout;
  CTrackBarCtrl m_GuiSI_Bank;

  gba *m_GBA;

  CViewTile_T (void): m_GuiLV_ChrCanvas (this),
  m_GuiLV_ChrCanvas2 (this),
  m_GuiLV_ChrCanvas3 (this),
  m_GuiLV_ChrCanvas4 (this),
  m_GuiLV_Pixout (this), 
  m_GuiLV_AttachCursor (NULL),
  m_ptTrack (0, 0),
  m_ptTrack2 (0, 0),
  m_bAttach8Bit (FALSE),
  m_pbAttachChr (NULL),
  m_dwAttachAddr (0)
  {
    m_GuiLV_ChrGroup[0] =  &m_GuiLV_ChrCanvas;
    m_GuiLV_ChrGroup[1] =  &m_GuiLV_ChrCanvas2;
    m_GuiLV_ChrGroup[2] =  &m_GuiLV_ChrCanvas3;
    m_GuiLV_ChrGroup[3] =  &m_GuiLV_ChrCanvas4;
  }
  ~CViewTile_T (void) {
    for (int Id = 0; Id != 4; Id ++) {
      CDC & attachItem = m_MemDibDraw[Id];
      HBITMAP Bitmap =  attachItem.SelectBitmap ((HBITMAP) m_SaveOldGDIObj[Id]);
      ATLASSUME (Bitmap != NULL && (Bitmap != HGDI_ERROR));
      :: DeleteObject (Bitmap);
    }
  }

  // Override DoPaint
  void DoPaint (CDCHandle dc) {
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 

    for (int Id = 0; Id != 4; Id++) {
      static const int ID_table[] = { IDLV_TILE_CHRTABLE0,  IDLV_TILE_CHRTABLE1, IDLV_TILE_CHRTABLE2, IDLV_TILE_CHRTABLE3};
      auto &attachListDraw = *m_GuiLV_ChrGroup[Id];
      attachListDraw.SubclassWindow (GetDlgItem (ID_table[Id]));

      auto &attachDC = m_MemDibDraw[Id];
      
      auto &attachOldGDI = m_SaveOldGDIObj[Id];
      auto tempDC = attachListDraw.GetDC ();

      attachDC.CreateCompatibleDC (tempDC);
      
      BITMAPINFO bmpinfos;
      ZeroMemory (& bmpinfos, sizeof (bmpinfos));
	    bmpinfos.bmiHeader.biSize        =  sizeof (bmpinfos);
	    bmpinfos.bmiHeader.biWidth       =  288;
	    bmpinfos.bmiHeader.biHeight      = -108; // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
	    bmpinfos.bmiHeader.biPlanes      =  1;
	    bmpinfos.bmiHeader.biBitCount    = 32;
	    bmpinfos.bmiHeader.biCompression =  BI_RGB;

	    auto  bitmap = :: CreateDIBSection	(attachDC, & bmpinfos, 
										                DIB_RGB_COLORS, (void* *)& m_MemDibBuffer[Id], NULL, 0);
      ATLASSUME (bitmap != NULL);

      attachOldGDI = attachDC.SelectBitmap (bitmap);
      ATLASSUME (attachOldGDI != NULL && (attachOldGDI != HGDI_ERROR));

      attachListDraw.ReleaseDC (tempDC);
      attachListDraw.add_msg_map (WM_MOUSEMOVE, & CViewTile_T::OnMouseMove);
      attachListDraw.add_msg_map (WM_LBUTTONUP, & CViewTile_T::OnMouseMove);
      attachListDraw.add_msg_map (WM_PAINT, & CViewTile_T::OnPaint); 


    }

    m_GuiST_Addr = GetDlgItem (IDST_CHR_ADDR);
    m_GuiST_Index = GetDlgItem (IDST_CHR_TID);
    m_GuiST_Addr2 = GetDlgItem (IDST_CHR_ADDR2);
    m_GuiST_Pos = GetDlgItem (IDST_CHR_POS);
    m_GuiST_Pix= GetDlgItem (IDST_CHR_PIXEL);
    m_GuiST_BGR = GetDlgItem (IDST_CHR_BGR);


    // m_GuiST_RGB_MAG = GetDlgItem (IDST_TILE_PIXELOUT);
    m_GuiRD_4BitPAL = GetDlgItem (IDRD_CHR_4BIT);
    m_GuiBT_Grid = GetDlgItem (IDCK_CHR_GRID);
    m_GuiBT_QFlush = GetDlgItem (IDCK_CHR_QFLUSH);
    m_GuiSI_Bank = (GetDlgItem (IDSI_TILE_4BITPALBANK));
    m_GuiLV_Pixout.SubclassWindow (GetDlgItem (IDLV_CHR_PIXELOUT));
    CheckDlgButton (IDRD_CHR_4BIT, BST_CHECKED);

    m_GuiLV_Pixout.add_msg_map (WM_MOUSEMOVE, & CViewTile_T::OnMouseMoveElem);
    m_GuiLV_Pixout.add_msg_map (WM_LBUTTONUP, & CViewTile_T::OnMouseMoveElem);
    bHandled = FALSE;

    m_GuiSI_Bank.SetPageSize (1);
    m_GuiSI_Bank.SetRangeMin (0);
    m_GuiSI_Bank.SetRangeMax (15);


    ModifyStyleEx (0, WS_EX_COMPOSITED);
    return 0;
  }

  void UpdateGBAItemOut (BOOL bForceUpdate = FALSE) {
    if (! (bForceUpdate 
    ||        (   IsWindowVisible () 
               && m_GuiBT_QFlush.GetCheck () == BST_CHECKED)) )
      return ;

    BOOL bGrid = m_GuiBT_Grid.GetCheck () == BST_CHECKED;
    if (m_GuiRD_4BitPAL.GetCheck () == BST_CHECKED) {
      draw4BitItem256 (0, t_dwBaseAddr, bGrid);
      draw4BitItem256 (1, t_dwBaseAddr + 0x2000, bGrid);
      // Check Tile Wrap round...
      DWORD dwAddrNext = t_dwBaseAddr + 0x4000;
      if (dwAddrNext >= 0x10000 && t_dwBaseAddr < 0x10000)
        dwAddrNext = 0;
      draw4BitItem256 (2, dwAddrNext, bGrid);
      draw4BitItem256 (3, dwAddrNext + 0x2000, bGrid);
    } else {
      DWORD dwOffset_1 = 0;
      if (t_dwBaseAddr >= 0x10000
        && CButton (GetDlgItem (IDCK_CHR_OFF1)).GetCheck () == BST_CHECKED)
        dwOffset_1 = 32;
      draw8BitItem256 (0, t_dwBaseAddr + dwOffset_1, bGrid);
      // Check Tile Wrap round...
      DWORD dwAddrNext = t_dwBaseAddr + dwOffset_1 + 0x4000 & 0xFFFF;
      DWORD dwAddrNext2 = t_dwBaseAddr + dwOffset_1 + 0x8000 & 0xFFFF;
      DWORD dwAddrNext3 = t_dwBaseAddr + dwOffset_1 + 0xC000 & 0xFFFF;
      
      if (t_dwBaseAddr < 0x10000) {
        draw8BitItem256 (1, dwAddrNext, bGrid);
        draw8BitItem256 (2, dwAddrNext2, bGrid);
        draw8BitItem256 (3, dwAddrNext3, bGrid);
      } else 
        draw8BitItem256 (1, dwAddrNext, bGrid);
    }
    // return ;
    // Draw elem pixel.
    CClientDC dc (m_GuiLV_Pixout);
#define CHECK_SQU_SIZE 12
    static const int block_size = CHECK_SQU_SIZE + 1;
    static const int full_block_size = block_size * 8 + 1;
#define COLOR_BASE 0xffffff 
    dc.FillSolidRect (0, 0, full_block_size, full_block_size, COLOR_BASE);
#define COLOR_FILL 0x0000ff
#define COLOR_TRACK 0x00ff00 
  //DWORD m_dwItemAddr2;
        
  //DWORD m_dwItemPix;
  //DWORD m_dwItemBGR;


    uint32_t *pal = (uint32_t *) & m_GBA->gpu.palette3[ (m_dwAttachAddr >= 0x10000) ? 512 : 0];
    uint16_t *pal2 =  (uint16_t *) & m_GBA->gpu.palette[ (m_dwAttachAddr >= 0x10000) ? 512 : 0];
    if (m_bAttach8Bit  != FALSE) {
      for (int y = 0; y != 8; y++) {
        for (int x = 0; x != 8; x++) {
          int pix = pal[m_pbAttachChr[y*8+x]];
          int x_pos = x * block_size + 1;
          int y_pos = y *  block_size + 1;
          dc.FillSolidRect (x_pos, y_pos, CHECK_SQU_SIZE, CHECK_SQU_SIZE, pix);
        }
      }
      m_dwItemAddr2 = m_dwItemAddr + m_ptTrack2.x + m_ptTrack2.y * 8;
      CSetWindowText (m_GuiST_Addr2, _T ("addr2@%07X"), m_dwItemAddr2);
      uint8_t val = m_pbAttachChr[m_ptTrack2.y*8+m_ptTrack2.x];
      CSetWindowText (m_GuiST_Pix, _T ("pix@%01X, %d"), val, val);
      CSetWindowText (m_GuiST_BGR, _T ("BGR@%04X"), pal2[val]);
    } else {
      int bank = m_GuiSI_Bank.GetPos ();
      pal = & pal [bank * 16];
      pal2 = & pal2 [bank * 16];

      for (int y = 0; y != 8; y++) {
        for (int x = 0; x != 4; x++) {
          int pix = m_pbAttachChr[y*4+x];
          int x_pos = x *2 * block_size + 1;
          int y_pos = y * block_size + 1;
          dc.FillSolidRect (x_pos, y_pos, CHECK_SQU_SIZE, CHECK_SQU_SIZE, pal[pix & 15]);
          dc.FillSolidRect (x_pos + block_size, y_pos, CHECK_SQU_SIZE, CHECK_SQU_SIZE, pal[pix >> 4 & 15]);
        }

      }
      m_dwItemAddr2 = m_dwItemAddr + m_ptTrack2.x / 2 + m_ptTrack2.y * 4;
      if (m_ptTrack2.x & 1)
        CSetWindowText (m_GuiST_Addr2, _T ("addr2@%07X:H"), m_dwItemAddr2);
      else 
        CSetWindowText (m_GuiST_Addr2, _T ("addr2@%07X:L"), m_dwItemAddr2);
      uint8_t val = m_pbAttachChr[m_ptTrack2.y*4+m_ptTrack2.x/2];
      if (m_ptTrack2.x & 1)
        val= val >> 4 & 15;
      else 
        val &= 15;
      CSetWindowText (m_GuiST_Pix, _T ("pix@%01X, %d"), val, val);
      CSetWindowText (m_GuiST_BGR, _T ("BGR@%04X"), pal2[val]);
    }
    CSetWindowText (m_GuiST_Addr, _T ("addr@%07X"), m_dwItemAddr);
    CSetWindowText (m_GuiST_Index, _T ("tid@%03X, %d"), m_dwItemTID, m_dwItemTID);
    CSetWindowText (m_GuiST_Pos, _T ("pos@%d, %d"), m_ptTrack2.x, m_ptTrack2.y);

    // Mark elem rect 
    int x2 = m_ptTrack2.x  * block_size;
    int y2 = m_ptTrack2.y * block_size;

      dc.FrameRect (
        & CRect (x2, y2, x2 + block_size + 1, y2 + block_size + 1),
          CBrush (CreateSolidBrush (0xff)));


  }

  void draw4BitItem256 (int iCanvasId, DWORD dwBaseAddr, BOOL bGrid) {

    auto &Canvas = *m_GuiLV_ChrGroup[iCanvasId];
    auto &mdc = m_MemDibDraw[iCanvasId];
    CClientDC dcChrCanvas (Canvas);
    auto dwPixelBuffer = m_MemDibBuffer[iCanvasId];

    int iInterval = !! bGrid;
    int iBank = m_GuiSI_Bank.GetPos ();
    uint32_t *palette;
    uint8_t *pixel = & m_GBA->gpu.vram[dwBaseAddr];

    if (t_dwBaseAddr >= 0x10000)
      palette = (uint32_t *)& m_GBA->gpu.palette4[512 + (iBank << 5)];
    else 
      palette = (uint32_t *)& m_GBA->gpu.palette4[ ((dwBaseAddr >= 0x10000) ? 512 : 0) + (iBank << 5)];

    for (int y = 0; y != 8; y++)  {
      for (int x = 0; x != 32; x++) {
        // add 32 in every x 
        // 
        uint8_t *pix = & pixel[(x<<5) + (y<<5) * 32];

        if (x == m_ptTrack.x 
          && y == m_ptTrack.y
          && m_GuiLV_AttachCursor == *m_GuiLV_ChrGroup[iCanvasId])
        {
          m_bAttach8Bit = FALSE;
          m_pbAttachChr = pix;
          m_dwAttachAddr = dwBaseAddr + (x<<5) + (y<<5) * 32;
          m_dwItemAddr = dwBaseAddr + (x<<5) + (y<<5) * 32;
  //DWORD m_dwItemAddr2;
          m_dwItemTID = iCanvasId * 256 +    x+y*32;
  //DWORD m_dwItemPos;
  //DWORD m_dwItemPix;
  //DWORD m_dwItemBGR;
        }

        // draw tile block every 32 byte 
        int xPos = x * 8 + iInterval * x;
        int yPos = y * 8 + iInterval * y;

        DWORD *pVMemory = & dwPixelBuffer[xPos + yPos * 288];
        for (int yScan = 0; yScan != 8; yScan++) {
          pVMemory[0] = palette[pix[0] >>  0 & 0xF];
          pVMemory[1] = palette[pix[0] >>  4 & 0xF];
          pVMemory[2] = palette[pix[1] >>  0 & 0xF];
          pVMemory[3] = palette[pix[1] >>  4 & 0xF];
          pVMemory[4] = palette[pix[2] >>  0 & 0xF];
          pVMemory[5] = palette[pix[2] >>  4 & 0xF];
          pVMemory[6] = palette[pix[3] >>  0 & 0xF];
          pVMemory[7] = palette[pix[3] >>  4 & 0xF];
          pVMemory += 288;
          pix += 4;
        }
      }
    }
    if (bGrid != FALSE) {
      for (int Id = 1; Id != 8; Id++) {
        int iVec = Id * 8 + Id - 1;
        static const int iVecTotal_X= (8 + 1) * 32 - 1;
        mdc.MoveTo (0, iVec, NULL);
        mdc.LineTo (iVecTotal_X, iVec );
      }
      for (int Id = 1; Id != 32; Id++) {
        int iVec = Id * 8 + Id - 1;
        static const int iVecTotal_Y = (8 + 1) * 8 - 1;
        mdc.MoveTo (iVec, 0, NULL);
        mdc.LineTo (iVec, iVecTotal_Y );
      }
      if (m_GuiLV_AttachCursor == Canvas) {
        int xPosDraw = m_ptTrack.x * 8 + m_ptTrack.x;
        int yPosDraw = m_ptTrack.y * 8 + m_ptTrack.y;
        int xDrawLength;
        int yDrawLength;

        if (m_ptTrack.x != 0) {
          xDrawLength = 10;
          xPosDraw --;
        }
        else 
          xDrawLength = 9;
        if (m_ptTrack.y != 0)  {
          yDrawLength = 10;
          yPosDraw --;
        }
        else 
          yDrawLength = 9;
        if (yDrawLength != 9) {
          mdc.FillSolidRect (xPosDraw,       yPosDraw,       xDrawLength, 1, 0x0000FF); // top - x 
          mdc.FillSolidRect (xPosDraw,       yPosDraw + 9, xDrawLength, 1, 0x0000FF); // bottom - x
        } else {
          mdc.FillSolidRect (xPosDraw,       yPosDraw + 8, xDrawLength, 1, 0x0000FF); // bottom - x
        }
        if (xDrawLength != 9) {
          mdc.FillSolidRect (xPosDraw,       yPosDraw, 1,   yDrawLength, 0x0000FF); // left - y
          mdc.FillSolidRect (xPosDraw + 9, yPosDraw, 1,   yDrawLength, 0x0000FF); // right - y
        } else {
          mdc.FillSolidRect (xPosDraw + 8, yPosDraw, 1,   yDrawLength, 0x0000FF); // right - y
        }
      }
    }
    // Blit to main.
    dcChrCanvas.BitBlt (0, 0, 288, 72, mdc, 0, 0, SRCCOPY);
  }

  void draw8BitItem256 (int iCanvasId, DWORD dwBaseAddr, BOOL bGrid) {
    auto &Canvas = *m_GuiLV_ChrGroup[iCanvasId];
    auto &mdc = m_MemDibDraw[iCanvasId];
    CClientDC dcChrCanvas (Canvas);
    auto dwPixelBuffer = m_MemDibBuffer[iCanvasId];

    int iInterval = !! bGrid;
    uint32_t *palette;
    uint8_t *pixel = & m_GBA->gpu.vram[dwBaseAddr];

    if (t_dwBaseAddr >= 0x10000)
      palette = (uint32_t *)& m_GBA->gpu.palette4[512];
    else 
      palette = (uint32_t *)& m_GBA->gpu.palette4[ (dwBaseAddr >= 0x10000) ? 512 : 0];

    for (int y = 0; y != 8; y++)  {
      for (int x = 0; x != 32; x++) {
        // add 64 in every x 
        // 
        uint8_t *pix = & pixel[(x<<6) + (y<<6) * 32];

        if (x == m_ptTrack.x 
          && y == m_ptTrack.y
          && m_GuiLV_AttachCursor == *m_GuiLV_ChrGroup[iCanvasId])
        {
          m_bAttach8Bit = TRUE;
          m_pbAttachChr = pix;
          m_dwAttachAddr = dwBaseAddr + (x<<6) + (y<<6) * 32;
          m_dwItemAddr = m_dwAttachAddr;
  //DWORD m_dwItemAddr2;
          m_dwItemTID = iCanvasId * 256 +  x+y*32;
  //DWORD m_dwItemPos;
  //DWORD m_dwItemPix;
  //DWORD m_dwItemBGR;

        }

        // draw tile block every 64 byte 
        int xPos = x * 8 + iInterval * x;
        int yPos = y * 8 + iInterval * y;

        DWORD *pVMemory = & dwPixelBuffer[xPos + yPos * 288];
        for (int yScan = 0; yScan != 8; yScan++) {
          pVMemory[0] = palette[pix[0]];
          pVMemory[1] = palette[pix[1]];
          pVMemory[2] = palette[pix[2]];
          pVMemory[3] = palette[pix[3]];
          pVMemory[4] = palette[pix[4]];
          pVMemory[5] = palette[pix[5]];
          pVMemory[6] = palette[pix[6]];
          pVMemory[7] = palette[pix[7]];
          pVMemory += 288;
          pix += 8;
        }
      }
    }

    if (bGrid != FALSE) {
      for (int Id = 1; Id != 8; Id++) {
        int iVec = Id * 8 + Id - 1;
        static const int iVecTotal_X= (8 + 1) * 32 - 1;
        mdc.MoveTo (0, iVec, NULL);
        mdc.LineTo (iVecTotal_X, iVec );
      }
      for (int Id = 1; Id != 32; Id++) {
        int iVec = Id * 8 + Id - 1;
        static const int iVecTotal_Y = (8 + 1) * 8 - 1;
        mdc.MoveTo (iVec, 0, NULL);
        mdc.LineTo (iVec, iVecTotal_Y );
      }
      if (m_GuiLV_AttachCursor == Canvas) {
        int xPosDraw = m_ptTrack.x * 8 + m_ptTrack.x;
        int yPosDraw = m_ptTrack.y * 8 + m_ptTrack.y;
        int xDrawLength;
        int yDrawLength;

        if (m_ptTrack.x != 0) {
          xDrawLength = 10;
          xPosDraw --;
        }
        else 
          xDrawLength = 9;
        if (m_ptTrack.y != 0)  {
          yDrawLength = 10;
          yPosDraw --;
        }
        else 
          yDrawLength = 9;
        if (yDrawLength != 9) {
          mdc.FillSolidRect (xPosDraw,       yPosDraw,       xDrawLength, 1, 0x0000FF); // top - x 
          mdc.FillSolidRect (xPosDraw,       yPosDraw + 9, xDrawLength, 1, 0x0000FF); // bottom - x
        } else {
          mdc.FillSolidRect (xPosDraw,       yPosDraw + 8, xDrawLength, 1, 0x0000FF); // bottom - x
        }
        if (xDrawLength != 9) {
          mdc.FillSolidRect (xPosDraw,       yPosDraw, 1,   yDrawLength, 0x0000FF); // left - y
          mdc.FillSolidRect (xPosDraw + 9, yPosDraw, 1,   yDrawLength, 0x0000FF); // right - y
        } else {
          mdc.FillSolidRect (xPosDraw + 8, yPosDraw, 1,   yDrawLength, 0x0000FF); // right - y
        }
      }
    }

    // Blit to main.
    dcChrCanvas.BitBlt (0, 0, 288, 72, mdc, 0, 0, SRCCOPY);
  }

  LRESULT OnPaint (CDnMsg<CViewTile_T>* pMsg, BOOL& bHandled) {
    CPaintDC dc (pMsg->from);
    UpdateGBAItemOut (TRUE);
    bHandled= TRUE;
    return 0;
  }

  LRESULT OnMouseMoveElem (CDnMsg<CViewTile_T>* pMsg, BOOL& bHandled) {

    BOOL bLockCheck = CButton (GetDlgItem (IDCK_CHR_LOCK2)).GetCheck () == BST_CHECKED;
    if ( ! (  (bLockCheck && 
      pMsg->msg == WM_LBUTTONUP)
      || (bLockCheck == FALSE && 
      pMsg->msg == WM_MOUSEMOVE) ))
    {
      bHandled = FALSE;
      return 1;
    }
    m_ptTrack2.x =  LOWORD (pMsg->lparam);
    m_ptTrack2.y =  HIWORD (pMsg->lparam);

    int iVec = CHECK_SQU_SIZE +  1;
    m_ptTrack2.x = m_ptTrack2.x / iVec;
    m_ptTrack2.y = m_ptTrack2.y / iVec;
    if (m_ptTrack2.x < 0)
      m_ptTrack2.x = 0;
    if (m_ptTrack2.y < 0)
      m_ptTrack2.y = 0;
    if (m_ptTrack2.x > 7)
      m_ptTrack2.x = 7;
    if (m_ptTrack2.y > 7)
      m_ptTrack2.y = 7;

    UpdateGBAItemOut (TRUE);
    return 0;
  }

  LRESULT OnMouseMove (CDnMsg<CViewTile_T>* pMsg, BOOL& bHandled) {
    if (pMsg->msg != WM_MOUSEMOVE
      && pMsg->msg != WM_LBUTTONUP)
    {
      bHandled = FALSE;
      return 1;
    }
    BOOL bLockCheck = CButton (GetDlgItem (IDCK_CHR_LOCK)).GetCheck () == BST_CHECKED;
    if ( ! (  (bLockCheck && 
      pMsg->msg == WM_LBUTTONUP)
      || (bLockCheck == FALSE && 
      pMsg->msg == WM_MOUSEMOVE) ))
    {
      bHandled = FALSE;
      return 1;
    }
    m_ptTrack.x =  LOWORD (pMsg->lparam);
    m_ptTrack.y =  HIWORD (pMsg->lparam);
    m_GuiLV_AttachCursor = pMsg->from;

    BOOL bGrid = IsDlgButtonChecked (IDCK_CHR_GRID) == BST_CHECKED;
    int iVec = 8 +  !! bGrid;
    m_ptTrack.x = m_ptTrack.x / iVec;
    m_ptTrack.y = m_ptTrack.y / iVec;
    if (m_ptTrack.x < 0)
      m_ptTrack.x = 0;
    if (m_ptTrack.y < 0)
      m_ptTrack.y = 0;
    if (m_ptTrack.x > 31)
      m_ptTrack.x = 31;
    if (m_ptTrack.y > 7)
      m_ptTrack.y = 7;

    UpdateGBAItemOut (TRUE);
    return 0;
  }
  
	LRESULT OnUpdateDraw (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
    // Clear misc mesh pixel .
    for (int Id = 0; Id != 4; Id++)
      m_MemDibDraw[Id].FillSolidRect (0, 0, 288, 72, 0);
		UpdateGBAItemOut (TRUE);
		return 0;
	}

	LRESULT OnScroll (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 
	  UpdateGBAItemOut (TRUE);
    bHandled = FALSE;
		return 0;
	}

  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER (WM_INITDIALOG, OnCreate)
    MESSAGE_HANDLER (WM_HSCROLL, OnScroll)
    MESSAGE_HANDLER (WM_VSCROLL, OnScroll)
    COMMAND_ID_HANDLER (IDCK_CHR_GRID, OnUpdateDraw)
    COMMAND_ID_HANDLER (IDRD_CHR_4BIT, OnUpdateDraw)
    COMMAND_ID_HANDLER (IDRD_CHR_8BIT, OnUpdateDraw)
    CHAIN_MSG_MAP(baseClass2)
  END_MSG_MAP()
};

template <int t_iMenuId, DWORD t_dwBaseAddr>
class CViewTile : public dockwins::CDockingTabChild<CViewTile<t_iMenuId, t_dwBaseAddr>> {
public:
  CMenuHandle m_AttachMenu;
  CViewTile_T<t_dwBaseAddr> m_ContainerOverview;

  typedef    dockwins::CDockingTabChild<CViewTile<t_iMenuId, t_dwBaseAddr>> baseClass;
  typedef CViewTile  thisClass;

  CViewTile (void) {}
 ~CViewTile (void) {}

  DECLARE_WND_CLASS_FX (_T ("CViewTile"), (CS_HREDRAW | CS_VREDRAW), COLOR_WINDOW, IDI_TILE)

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
    m_ContainerOverview.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect rcClient;
    m_ContainerOverview.Create (*this);
    m_ContainerOverview.ShowWindow (SW_SHOWNORMAL);
    m_ContainerOverview.GetClientRect (rcClient);
    ResizeClient (rcClient.Width () + 8, rcClient.Height () + 8);
    m_ContainerOverview.ResizeClient (rcClient.Width (), rcClient.Height ());
    m_ContainerOverview.SetScrollSize (rcClient.Width (), rcClient.Height ());
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (t_iMenuId, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, gba *attachGBA) {
    m_ContainerOverview.m_GBA = attachGBA;
    Create (parent, CRect (0,0, 320, 240), _T ("Tile"), WS_OVERLAPPEDWINDOW  | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  WS_EX_TRANSPARENT);
  }
};

# endif 