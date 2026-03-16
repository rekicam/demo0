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

# if !defined (_CVIEWIO_INCLUED_HEADER_CXX_)
#  define _CVIEWIO_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabChild.hxx"
# include <dockwins/TabbedFrame.h>
# include <tchar.h>
# include "..\resource.h"
# include "..\GBA\gba.h"

// Function := ARM-CPU.

// @0 := Switch GBA IO Mapper.
//
// Alt + Keyboard 1 := LCD MASTER CONTROL INFOS
// Alt + Keyboard 2 := SOUND 
// Alt + Keyboard 3 := DMA 
// Alt + Keyboard 4 := TIMER
// Alt + Keyboard 5 := COMPORT
// Alt + Keyboard 6 := CONTROLLER
// Alt + Keyboard 7 := INTERRUPT, MISC

// @1 := misc
//  
// Alt + U := Manual Update Memory
// Alt + A := Auto Update Memory fast 

typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0>    CViewIOT_Traits;

struct  CViewIO_T : public CWindowImpl <CViewIO_T, CWindow, CViewIOT_Traits>// public CMessageFilter
{
public :
  //  fixed min size
  //  some settings 
  static  const  int width = 820;
  static  const  int height = 460;
  static  const  COLORREF background_color = 0;
  static  const  int nes8bitfont_size = 9;
  static  const  DWORD font_color = 0xFFFFFF;

  typedef CWindowImpl<CViewIO_T,CWindow,CViewIOT_Traits> baseClass;
  typedef CViewIO_T  thisClass;
  
  CRichEditREF <CViewIO_T>  m_GuiRT_Output;
  struct gba *m_GBA;
  BOOL m_bAutoUpdate;
  DWORD m_CurrentRegion;

  CViewIO_T (void):m_GuiRT_Output (this)  { m_CurrentRegion = 0; m_bAutoUpdate = FALSE; }
 ~CViewIO_T (void) {}

  void UpdateGBAIO_LCD_Output (void) {
#if 0
    WORD DISPCNT = gba_fast_rhw (m_GBA, 0x4000000);
    WORD DISPSTAT = gba_fast_rhw (m_GBA, 0x4000004);
    WORD VCOUNT = gba_fast_rhw (m_GBA, 0x4000006);
    WORD WIN0H = gba_fast_rhw (m_GBA, 0x4000040);
    WORD WIN1H = gba_fast_rhw (m_GBA, 0x4000042);
    WORD WIN0V = gba_fast_rhw (m_GBA, 0x4000044);
    WORD WIN1V = gba_fast_rhw (m_GBA, 0x4000046);
    WORD WININ = gba_fast_rhw (m_GBA, 0x4000048);
    WORD WINOUT = gba_fast_rhw (m_GBA, 0x400004A);
    WORD MOSAIC = gba_fast_rhw (m_GBA, 0x400004C);
    WORD BLDCTL = gba_fast_rhw (m_GBA, 0x4000050);
    WORD BLDV = gba_fast_rhw (m_GBA, 0x4000052);
    WORD BLDY = gba_fast_rhw (m_GBA, 0x4000054);
    WORD BGCNT[4];
    WORD SCROLL_X[4];
    WORD SCROLL_Y[4];
    DWORD AFFINE_bx[2];
    DWORD AFFINE_by[2];
    WORD AFFINE_PA[2];
    WORD AFFINE_PB[2];
    WORD AFFINE_PC[2];
    WORD AFFINE_PD[2];

    BGCNT[0] = gba_fast_rhw (m_GBA, 0x4000008);
    BGCNT[1] = gba_fast_rhw (m_GBA, 0x400000A);
    BGCNT[2] = gba_fast_rhw (m_GBA, 0x400000C);
    BGCNT[3] = gba_fast_rhw (m_GBA, 0x400000E);

    SCROLL_X[0] = gba_fast_rhw (m_GBA, 0x4000010);
    SCROLL_Y[0] = gba_fast_rhw (m_GBA, 0x4000012);
    SCROLL_X[1] = gba_fast_rhw (m_GBA, 0x4000014);
    SCROLL_Y[1] = gba_fast_rhw (m_GBA, 0x4000016);
    SCROLL_X[2] = gba_fast_rhw (m_GBA, 0x4000018);
    SCROLL_Y[2] = gba_fast_rhw (m_GBA, 0x400001A);
    SCROLL_X[3] = gba_fast_rhw (m_GBA, 0x400001C);
    SCROLL_Y[3] = gba_fast_rhw (m_GBA, 0x400001E);

    AFFINE_PA[0] = gba_fast_rhw (m_GBA, 0x4000020);
    AFFINE_PB[0] = gba_fast_rhw (m_GBA, 0x4000022);
    AFFINE_PC[0] = gba_fast_rhw (m_GBA, 0x4000024);
    AFFINE_PD[0] = gba_fast_rhw (m_GBA, 0x4000026);
    AFFINE_bx[0] = gba_fast_rhw (m_GBA, 0x4000028);
    ((PWORD)AFFINE_bx)[1] = gba_fast_rhw (m_GBA, 0x400002A);
        AFFINE_by[0] = gba_fast_rhw (m_GBA, 0x400002C);
    ((PWORD)AFFINE_by)[1] = gba_fast_rhw (m_GBA, 0x400002E);
    AFFINE_PA[1] = gba_fast_rhw (m_GBA, 0x4000020+16);
    AFFINE_PB[1] = gba_fast_rhw (m_GBA, 0x4000022+16);
    AFFINE_PC[1] = gba_fast_rhw (m_GBA, 0x4000024+16);
    AFFINE_PD[1] = gba_fast_rhw (m_GBA, 0x4000026+16);
    AFFINE_bx[1] = gba_fast_rhw (m_GBA, 0x4000028+16);
    ((PWORD)& AFFINE_bx[1])[1] = gba_fast_rhw (m_GBA, 0x400002A+16);
        AFFINE_by[1] = gba_fast_rhw (m_GBA, 0x400002C+16);
    ((PWORD)& AFFINE_by[1])[1] = gba_fast_rhw (m_GBA, 0x400002E+16);

    LPCTSTR sSize[] = { _T ("256x256"), _T("512x256"), _T("256x512"), _T ("512x512") };
    LPCTSTR sBld[] = { _T ("NONE"), _T("BLD "), _T("BRI+"), _T ("BRI-") };
    LPCTSTR sFormat = (_T("+0!DISPCNT -> %04X             \t| +40!WIN0H ->%04X L:%d R:%d\t           |+208! IME ->%d           \n")
                      _T ("|___ VIDE_MODE:%d              \t| +42!WIN1H ->%04X L:%d R:%d\t           |+200! IE  ->%04X           \n")
                      _T ("|___ FRAME:%d                  \t| +44!WIN0V ->%04X T:%d B:%d\t           |+202! IF  ->%04X                       \n")
                      _T ("|___ HBLANK-FREE:%s\t\t| +46!WIN1V ->%04X T:%d B:%d               |__________________   \n")
                      _T ("|___ OAM V-MAP:%s\t\t| +48!WININ ->%04X   \t\t           |  IE IF BIOS            \n")
                      _T ("|___ FBLANK:%s\t\t\t| +4A!WINOUT->%04X   \t\t           |   %s  %s    %s  VBL           \n") 
                      _T ("|______________________________\t|_____________________________           |   %s  %s    %s  HBL            \n")
                      _T ("| BG0:%d BG1:%d BG2:%d BG3:%d  \t\t|  WIN0 WIN1 WINOBJ WINOUT               |   %s  %s    %s  VCNT            \n")
                      _T ("|______________________________\t|     %s   %s     %s     %s    BG0           |   %s  %s    %s  TM0            \n")
                      _T ("| SP:%d                        \t|     %s   %s     %s     %s    BG1           |   %s  %s    %s  TM1            \n")
                      _T ("|______________________________\t|     %s   %s     %s     %s    BG2           |   %s  %s    %s  TM2            \n")  
                      _T ("| WIN0:%d WIN1:%d WINOBJ:%d    \t\t|     %s   %s     %s     %s    BG3           |   %s  %s    %s  TM3             \n") 
                      _T ("|______________________________\t|     %s   %s     %s     %s    SP            |   %s  %s    %s  SIO            \n")
                      _T ("                               \t|     %s   %s     %s     %s    FILTER        |   %s  %s    %s  DMA0            \n")
                      _T ("+4!DISPSTAT -> %04X            \t|________________________________________|   %s  %s    %s  DMA1            \n")
                      _T ("|___ VBL_FLG:%d                \t|  T1 T2       | +50!FILTER ->%04X:= %s|   %s  %s    %s  DMA2            \n")
                      _T ("|___ HBL_FLG:%d                \t|   %s %s   BG0  | +4C!MOSAIC ->%04X       |   %s  %s    %s  DMA3            \n")
                      _T ("|___ VCOUNT_FLG:%d             \t|   %s %s   BG1  |   |___ BG H:%d           |   %s  %s    %s  KEY            \n")
                      _T ("|___ VBL_IRQ_EN:%s             |   %s %s   BG2  |   |___ BG V:%d           |   %s  %s    %s  CART            \n")
                      _T ("|___ HBL_IRQ_EN:%s             |   %s %s   BG3  |   |___ SP H:%d           |___________________\n")
                      _T ("|___ VCOUNT_IRQ_EN:%s          |   %s %s   SP   |   |___ SP V:%d           | USR_IRQ -> %08X           \n")
                      _T ("|___ VCOUNT_SET:%03d             \t|   %s %s   BD   |                         | SND_BUF -> %08X            \n")
                      _T ("                               \t|______________| BLD_T1 := %d\t           |            \n")
                      _T ("+6!VCOUNT -> %04X              \t|              | BLD_T2 := %d\t           |            \n")
                      _T ("|___ SCANLINE:%d               \t|MOECMKS Do It!| BRI_T := %d\t           |            \n")
                      _T ("|____________________________________________________________________________________              |            \n")             
                      _T ("|___                    \t\tBG0      \tBG1     \tBG2      \tBG3               |            \n")
                      _T ("|____________________________________________________________________________________              |            \n")
                      _T ("|___CHR_b                         %07X     %07X     %07X     %07X              |            \n")
                      _T ("|___NAMETABLE_b                   %07X     %07X     %07X     %07X \n")
                      _T ("|___VRAM_BANK                     %s     %s     %s     %s               |            \n")
                      _T ("|___PRI                 \t\t%d\t\t%d\t\t%d\t\t%d               |            \n")
                      _T ("|___MOSAIC              \t\t%d\t\t%d\t\t%d\t\t%d               |            \n")
                      _T ("|___COL                 \t\t%d\t\t%d\t\t%d\t\t%d               |            \n")
                      _T ("|___SCROLL-X            \t\t%d\t\t%d\t\t%d\t\t%d               |            \n")
                      _T ("|___SCROLL-Y            \t\t%d\t\t%d\t\t%d\t\t%d              |            \n")
                      _T ("|______________________A F F I N E_________________________________________              |            \n")
                      _T ("|___REF-X  \t%08X:%f  %08X:%f \n")
                      _T ("|___REF-Y  \t%08X:%f  %08X:%f \n")
                      _T ("|___PA   \t%04X:%f  %04X:%f  \n")
                      _T ("|___PB   \t%04X:%f  %04X:%f \n")
                      _T ("|___PC   \t%04X:%f  %04X:%f  \n")
                      _T ("|___PD   \t%04X:%f  %04X:%f \n\n-"));
#define GET_BIT(var, sft)\
  ((var)>>(sft) & 1)

    DWORD ARM7_ie = m_GBA->arm7.ie.blk;
    DWORD ARM7_if = m_GBA->arm7.ifs.blk;

    DWORD UsrIRQ_hander = *(PDWORD) & m_GBA->mem.IRam[0x7FFC];
    DWORD ARM7_BIOS_Int =  *(PDWORD) & m_GBA->mem.IRam[0x7FF8];
    DWORD FifoBuffer =   *(PDWORD) & m_GBA->mem.IRam[0x7FF0];
    // DWORD svcStack =   *(PDWORD) & m_GBA->IRam[0x7FA0];
#define GET_IE_IF_BIOS_INT_GP(sft)\
    GET_BIT (ARM7_ie, sft) ? _T ("+") : _T(" "), GET_BIT (ARM7_if, sft) ? _T ("+") : _T(" "), GET_BIT (ARM7_BIOS_Int, sft) ? _T ("+") : _T(" ")

    CString cstr;
    cstr.AppendFormat (sFormat, DISPCNT, WIN0H, WIN0H >> 8, WIN0H & 0xFF, (m_GBA->arm7.ime.blk & 1),
                                                  DISPCNT & 7,  WIN1H, WIN1H >> 8, WIN1H & 0xFF, m_GBA->arm7.ie.blk,
                                                  DISPCNT >> 4 & 1, WIN0V, WIN0V >> 8, WIN0V & 0xFF, m_GBA->arm7.ifs.blk,
                                                  (DISPCNT >> 5 & 1) ? _T("ENABLE "):_T("DISABLE"), WIN0V, WIN1V >> 8, WIN1V & 0xFF,
                                                  (DISPCNT >> 6 & 1) ? _T("LINEAR     "):_T("FIXED PITCH"), WININ,
                                                  (DISPCNT >> 7 & 1) ? _T("ENABLE "):_T("DISABLE"), WINOUT, GET_IE_IF_BIOS_INT_GP (0),
                                                  GET_IE_IF_BIOS_INT_GP (1),
                                                  DISPCNT >> 8 & 1, DISPCNT >> 9 & 1, DISPCNT >> 10 & 1, DISPCNT >>11 & 1, GET_IE_IF_BIOS_INT_GP (3),                     
#define PRIVATE_UNWIND(n)\
(WININ >> n & 1) ? _T ("+") :_T(" "),  (WININ >> n+8 & 1) ? _T ("+") :_T(" "),  (WINOUT >> n+ 8 & 1) ? _T ("+") :_T(" "), (WINOUT >> n & 1) ? _T ("+") :_T(" ")
                                                  PRIVATE_UNWIND (0), GET_IE_IF_BIOS_INT_GP (4),
                                                   DISPCNT >>12 & 1,PRIVATE_UNWIND (1),GET_IE_IF_BIOS_INT_GP (5),
                                                   PRIVATE_UNWIND (2),GET_IE_IF_BIOS_INT_GP (6),
                                                   DISPCNT >>13 & 1, DISPCNT >>14 & 1, DISPCNT >>15 & 1, PRIVATE_UNWIND (3),GET_IE_IF_BIOS_INT_GP (7),
                                                  PRIVATE_UNWIND (4),GET_IE_IF_BIOS_INT_GP (8),
                                                   PRIVATE_UNWIND (5),GET_IE_IF_BIOS_INT_GP (9),
#undef PRIVATE_UNWIND
#define PRIVATE_UNWIND(n)\
  (BLDCTL >>(n) & 1) ? _T ("+") : _T (" "), (BLDCTL >>((n)+8) & 1) ? _T ("+") : _T (" ")
                                                   DISPSTAT,GET_IE_IF_BIOS_INT_GP (9),
                                                   GET_BIT (DISPSTAT, 0), BLDCTL, sBld[BLDCTL>>6 & 3],GET_IE_IF_BIOS_INT_GP (10),
                                                   GET_BIT (DISPSTAT, 1), PRIVATE_UNWIND (0), MOSAIC,GET_IE_IF_BIOS_INT_GP (11),
                                                   GET_BIT (DISPSTAT, 2), PRIVATE_UNWIND (1), MOSAIC & 15,GET_IE_IF_BIOS_INT_GP (12),
                                                   GET_BIT (DISPSTAT, 3) ? _T ("ENABLE ") :_T ("DISABLE"), PRIVATE_UNWIND (2), MOSAIC >> 4& 15,GET_IE_IF_BIOS_INT_GP (13),
                                                   GET_BIT (DISPSTAT, 4) ? _T ("ENABLE ") :_T ("DISABLE"), PRIVATE_UNWIND (3), MOSAIC >> 8& 15,
                                                   GET_BIT (DISPSTAT, 5) ? _T ("ENABLE ") :_T ("DISABLE"), PRIVATE_UNWIND (4), MOSAIC >>12& 15, UsrIRQ_hander, 
                                                   DISPSTAT >> 8, PRIVATE_UNWIND (5), FifoBuffer,
                                                   BLDV & 31, 
                                                   VCOUNT, BLDV >> 8 & 31,
                                                   VCOUNT & 255, BLDY & 31,
#define GET_BIT(var, sft)\
  ((var)>>(sft) & 1)
#define CHAR_B(n)\
    0x6000000 + ((BGCNT[n] >> 2 & 3) * 0x4000)
#define NT_B(n)\
    0x6000000 + ((BGCNT[n] >> 8 & 31) * 0x800)
                                                  CHAR_B(0), CHAR_B(1), CHAR_B(2), CHAR_B(3), 
                                                  NT_B(0), NT_B(1), NT_B(2), NT_B(3), 
                                                  sSize[BGCNT[0]>>14&3], sSize[BGCNT[1]>>14&3],  sSize[BGCNT[2]>>14&3],  sSize[BGCNT[3]>>14&3],
                                                  BGCNT[0] & 3, BGCNT[1] & 3, BGCNT[2] & 3,BGCNT[3] & 3,
                                                  BGCNT[0] >> 6 & 1, BGCNT[1] >> 6 & 1, BGCNT[2] >> 6 & 1,BGCNT[3] >> 6 & 1,
                                                  (BGCNT[0] >> 7 & 1) ? 256 : 16, (BGCNT[1] >> 7 & 1) ? 256 : 16, (BGCNT[2] >> 7 & 1) ? 256 : 16,(BGCNT[3] >> 7 & 1) ? 256 : 16,
                                                  SCROLL_X[0] & 511, SCROLL_X[1] & 511,  SCROLL_X[2] & 511, SCROLL_X[3] & 511,
                                                  SCROLL_Y[0] & 511, SCROLL_Y[1] & 511,  SCROLL_Y[2] & 511, SCROLL_Y[3] & 511,
                                                  AFFINE_bx[0], GBAInt28_2Flt (AFFINE_bx[0]), AFFINE_bx[1], GBAInt28_2Flt (AFFINE_bx[1]), 
                                                  AFFINE_by[0], GBAInt28_2Flt (AFFINE_by[0]), AFFINE_by[1], GBAInt28_2Flt (AFFINE_by[1]), 
                                                  AFFINE_PA[0], GBAInt16_2Flt (AFFINE_PA[0]), AFFINE_PA[1], GBAInt16_2Flt (AFFINE_PA[1]), 
                                                  AFFINE_PB[0], GBAInt16_2Flt (AFFINE_PB[0]), AFFINE_PB[1], GBAInt16_2Flt (AFFINE_PB[1]), 
                                                  AFFINE_PC[0], GBAInt16_2Flt (AFFINE_PC[0]), AFFINE_PC[1], GBAInt16_2Flt (AFFINE_PC[1]), 
                                                  AFFINE_PD[0], GBAInt16_2Flt (AFFINE_PD[0]), AFFINE_PD[1], GBAInt16_2Flt (AFFINE_PD[1])); 

      //int line = m_GuiRT_Output.GetFirstVisibleLine ();
      CancelSomeInputStatus ();
      m_GuiRT_Output.SetWindowText (cstr);
      if (::GetFocus () == NULL)
        m_GuiRT_Output.SetFocus ();
#endif 
  }

  void UpdateGBAIO_DMA_Output (void) {
#if 0
    DWORD SRC[4];
    DWORD DST[4];
    DWORD CNT[4];
    WORD CTL[4];
    PWORD const pSRC = (PWORD)& SRC[0];
    PWORD const pDST= (PWORD)& DST[0];

    LPTSTR pDMACommon[4] = { _T ("STD"), _T ("VBL"), _T ("HBL"), _T ("") };
    LPTSTR pDMASpec[4] = { _T ("UB"), _T ("FIFO"), _T ("FIFO"), _T ("RASTER/GAMEPAK")};
    LPTSTR pDMASpecPtr[4];
    LPTSTR pTimerLut[4] = { _T ("F/1"), _T ("F/64"), _T ("F/256"), _T ("F/1024")};
    DWORD dwTimerLut[4] =  { 1, 64, 256, 1024 };

    LPCTSTR sFormat = (_T ("_____________        DMA0     DMA1     DMA2       DMA3 ______________________________\n")
          _T ("EN                   %s\t        %s\t       %s   \t      %s\n")
          _T ("SRC                  %07X\t      %07X\t     %07X\t       %07X\n")
          _T ("DST                  %07X\t      %07X\t     %07X \t      %07X \n")
          _T ("CNT                  %05X\t      %05X\t     %05X \t      %05X\n")
          _T ("SIZE                 %d\t        %d \t      %d   \t     %d\n")
          _T ("REP                  %s\t        %s \t      %s   \t     %s\n")
          _T ("SPEC                 %s\t        %s  \t     %s   \t     %s \n")
          _T ("IRQ                 %s\t        %s   \t    %s   \t     %s \n\n")
          _T ("_____________        TIMER0     TIMER1     TIMER2       TIMER3 ______________________________\n")
          _T ("EN                   %s\t        %s\t       %s   \t      %s\n")
          _T ("INIT                  %04X\t      %04X\t     %04X\t       %04X\n")
          _T ("CUR                  %04X\t      %04X\t     %04X \t      %04X \n")
          _T ("SCALER               %s\t      %s\t     %s \t      %s\n")
          _T ("CASCADE                 %s\t        %s \t      %s   \t     %s\n")
          _T ("IRQ                 %s\t        %s   \t    %s   \t     %s \n")
          _T ("HZ                 %.4fHz\t        %.4fHz \t      %.4fHz   \t     %.4fHz\n")
          _T ("CLKS/HZ     %.4fClks\t        %.4fClks \t      %.4fClks   \t     %.4fClks\n\n\n-")  

                                      );

    int iBase = 0x40000B0;
    for (int Id= 0; Id != 4; Id++) {
      const int Idt = Id << 1;
      pSRC[Idt] = gba_fast_rhw (m_GBA, iBase+0);
      pSRC[Idt+1] = gba_fast_rhw (m_GBA, iBase+2);
      pDST[Idt] = gba_fast_rhw (m_GBA, iBase+4);
      pDST[Idt+1] = gba_fast_rhw (m_GBA, iBase+6);
      CNT[Id]  = gba_fast_rhw (m_GBA, iBase+8);
      CTL[Id]  = gba_fast_rhw (m_GBA, iBase+10);
      if (Id != 3)
        CNT[Id] &= 0x3FFF;
      if (CNT[Id] == 0)
        if (Id != 3)
          CNT[Id] = 0x4000;
        else 
          CNT[Id] = 0x10000;
      else ;
      int iDmaEnum = CTL[Id] >> 12 & 3;
      if ( iDmaEnum != 3)
        pDMASpecPtr[Id] = pDMACommon[iDmaEnum];
      else if (Id != 3)
        pDMASpecPtr[Id] = pDMASpec[Id];
      else if (CTL[Id] >> 11 & 1)
        pDMASpecPtr[3] = _T ("GamePAK");
      else 
        pDMASpecPtr[3] = _T ("Raster");
      iBase += 12;
    }

#define GET_BIT(var, sft)\
  ((var)>>(sft) & 1)
#define SET_EN(n)\
  GET_BIT(CTL[n],15) ? _T ("ENABLE ") : _T ("DISABLE")
#define SET_SIZE(n)\
  GET_BIT(CTL[n],10) ? 32 : 16
#define SET_REP(n)\
  GET_BIT(CTL[n],9) ? _T ("YES") : _T ("NO")
#define SET_IRQ(n)\
  GET_BIT(CTL[n],14) ? _T ("ENABLE ") : _T ("DISABLE")

    CString cstr;
    struct timer *const pTimer = & m_GBA->timer;
    cstr.AppendFormat (sFormat, SET_EN(0), SET_EN(1), SET_EN(2), SET_EN(3),
                                                  SRC[0], SRC[1], SRC[2], SRC[3],
                                                  DST[0], DST[1], DST[2], DST[3],
                                                  CNT[0], CNT[1], CNT[2], CNT[3],
                                                  SET_SIZE(0), SET_SIZE(1), SET_SIZE(2), SET_SIZE(3),
                                                  SET_REP(0), SET_REP(1), SET_REP(2), SET_REP(3),
                                                  pDMASpecPtr[0], pDMASpecPtr[1], pDMASpecPtr[2], pDMASpecPtr[3],
                                                  SET_IRQ(0), SET_IRQ(1), SET_IRQ(2), SET_IRQ(3),
#undef SET_EN
#undef SET_IRQ                                      
#define SET_EN(n)\
  GET_BIT(pTimer->item[n].ctl.blk,7) ? _T ("ENABLE ") : _T ("DISABLE")
                                                  SET_EN(0), SET_EN(1), SET_EN(2), SET_EN(3),
#define SET_INIT(n)\
  pTimer->item[n].reload.blk
                                                  SET_INIT(0), SET_INIT(1), SET_INIT(2), SET_INIT(3),
#define SET_COUNTER(n)\
  pTimer->item[n].counter.blk
                                                  SET_COUNTER(0), SET_COUNTER(1), SET_COUNTER(2), SET_COUNTER(3),
#define SET_SCALER(n)\
 pTimerLut[pTimer->item[n].ctl.blk & 3]
                                                  SET_SCALER(0), SET_SCALER(1), SET_SCALER(2), SET_SCALER(3),
#define SET_CASCADE(n)\
  GET_BIT(pTimer->item[n].ctl.blk,2) ? _T ("ENABLE ") : _T ("DISABLE")
                                                  SET_CASCADE(0), SET_CASCADE(1), SET_CASCADE(2), SET_CASCADE(3),
#define SET_IRQ(n)\
  GET_BIT(pTimer->item[n].ctl.blk,6) ? _T ("ENABLE ") : _T ("DISABLE")
                                                  SET_IRQ(0), SET_IRQ(1), SET_IRQ(2), SET_IRQ(3),
#define SET_HZ(n)\
  16780000.0 / (((double)dwTimerLut[pTimer->item[n].ctl.blk & 3]) * (double)(0x10000 - pTimer->item[n].reload.blk))
                                                  SET_HZ(0), SET_HZ(1), SET_HZ(2), SET_HZ(3),
#define SET_HZ_CLKS(n)\
  16780000.0 /(16780000.0 / (((double)dwTimerLut[pTimer->item[n].ctl.blk & 3]) * (double)(0x10000 - pTimer->item[n].reload.blk)))
                                                  SET_HZ_CLKS(0), SET_HZ_CLKS(1), SET_HZ_CLKS(2), SET_HZ_CLKS(3)
                                                  );

      CancelSomeInputStatus ();
      m_GuiRT_Output.SetWindowText (cstr);
      if (::GetFocus () == NULL)
        m_GuiRT_Output.SetFocus ();
  }

  float GBAInt28_2Flt (int32_t val) {
    if (val & 0x8000000)
      val |= 0xF0000000;
    int64_t vcalc = val;
    vcalc <<= 8;
    float fcalc = (float) vcalc;
    return fcalc/ 256.0 / 256.0;
  }

  float GBAInt16_2Flt (int16_t val) {
    int32_t vcalc = val;
    vcalc <<= 8;
    float fcalc = (float) vcalc;
    return fcalc/ 256.0 / 256.0;
  }

 void UpdateMemoryARM7 (void) {
    UpdateOutput (m_CurrentRegion);
 }

  void UpdateOutput (DWORD dwRegion = -1) {

    CRect rc;
    int iElem;
    GetClientRect (rc);
    iElem = rc.Height ()/ nes8bitfont_size;
    if (rc.Width () <= 0 || rc.Height () <= 0 || iElem < 1)
      return ;
    else {}

    if (dwRegion != -1)
      m_CurrentRegion = dwRegion;
    else {}

    switch (m_CurrentRegion) {
    case 0:
      // Region 1 :: LCD 
      UpdateGBAIO_LCD_Output ();
      break;
    case 1:
      UpdateGBAIO_DMA_Output ();
      break;
    default:
      ATLASSERT (FALSE);
      break;
    }
#endif 
  }

  LRESULT OnSysCharMessage (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    // Mask alt failure warning sound effect
    bHandled= TRUE;
    return 0;
  }

  LRESULT OnSysKeyUpMessage (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    bHandled= TRUE;
    switch (pMsg->wparam) {
    case '1':
      m_CurrentRegion = 0; 
      break;
    case '2':
      m_CurrentRegion = 1;
      break;
    case 'A':
      if (m_bAutoUpdate != FALSE)
        ;//CViewAppendOuput (_T ("CViewIO::Auto Update Close!\n"));
      else 
        ;//CViewAppendOuput (_T ("CViewIO::Auto Update Open!\n"));
      m_bAutoUpdate = !m_bAutoUpdate;
      return 0;
    case 'U':
      break;
    default:
      return 0;
    }
    //UpdateOutput (m_CurrentRegion);
    return 0;
  }

  void TryAutoUpdate (void) {
    if (m_bAutoUpdate)
   ;  //UpdateOutput (m_CurrentRegion);
    else {}
  }

  DECLARE_WND_CLASS(_T ("CViewIO_T"))
  
  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
  END_MSG_MAP()

  LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
  { 
    if (m_GuiRT_Output.IsWindow () != FALSE)
      m_GuiRT_Output.SetFocus ();
    bHandled = FALSE;
    return 0;
  }

 void CancelSomeInputStatus (void) {
   // Cancel the current input state? I really don't know.
   // But he does work. That's enough.
   m_GuiRT_Output.EnableWindow (FALSE);
   m_GuiRT_Output.EnableWindow (TRUE);
 }

  LRESULT OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = FALSE;
    SELCHANGE *p  = (SELCHANGE *)lParam;
    if (p->nmhdr.hwndFrom == m_GuiRT_Output
      && p->nmhdr.code == EN_SELCHANGE) {
        //// for manual mode, paste mem. 
        if (m_bAutoUpdate != FALSE)
          CancelSomeInputStatus (); 
    }
    return 0;
  }

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    CRect rc;
    ::SetWindowPos(m_GuiRT_Output, NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnEraseBG (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    bHandled = FALSE;
    return 1;
  }

  LRESULT OnMouseWheelMessage (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    bHandled = TRUE;
    return 0;
  }

  LRESULT OnVScrollMessage (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    // m_GuiRT_Output.SetSelNone ();
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      // The double buffers in the system (WS_EX_COMPOSITED) have BUG and will not 
      // handle the update status of SB_THUMBTRACK in my os (win7. amd64)..
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Output.GetScrollInfo ( SB_VERT, & scrBar);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_POS;
      m_GuiRT_Output.SetScrollInfo ( SB_VERT, & scrBar, FALSE);
    }  
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnHScrollMessage (CDnMsg<CViewIO_T>* pMsg, BOOL& bHandled) {
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Output.GetScrollInfo ( SB_HORZ, & scrBar);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Output.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
    }
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
    // SetClassLongPtr (m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR) GetStockObject (NULL_BRUSH)) ;
    HINSTANCE hInst = _Module.GetModuleInstance ();
    HWND hWindow = m_hWnd;

    HFONT hFont = CreateFont (-8, 0, 0, 0, 500, FALSE, FALSE, FALSE, 0, 500, 2, 1, 1, _T ("Press Start 2P"));
    
    // Edit Group 
    m_GuiRT_Output.Create (*this, NULL, NULL, WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD  |  WS_VSCROLL | WS_HSCROLL  | ES_MULTILINE | ES_READONLY);
    m_GuiRT_Output.SetFont (hFont, FALSE);
    m_GuiRT_Output.SetBackgroundColor (background_color);
    //m_GuiRT_Output.HideSelection (TRUE, FALSE);

    m_GuiRT_Output.add_msg_map (WM_SYSKEYUP, & CViewIO_T::OnSysKeyUpMessage);
    m_GuiRT_Output.add_msg_map (WM_SYSCHAR, & CViewIO_T::OnSysCharMessage);
    m_GuiRT_Output.add_msg_map (WM_VSCROLL, & CViewIO_T::OnVScrollMessage);
    m_GuiRT_Output.add_msg_map (WM_HSCROLL, & CViewIO_T::OnHScrollMessage);
    m_GuiRT_Output.add_msg_map (WM_MOUSEWHEEL, & CViewIO_T::OnMouseWheelMessage);

    CHARFORMAT ch_format;
    ZeroMemory (&ch_format, sizeof(CHARFORMAT));
      
    m_GuiRT_Output.GetDefaultCharFormat(ch_format);
    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = font_color;

    m_GuiRT_Output.SetDefaultCharFormat(ch_format);
    // ShowARM7Disasm2 (0);
    PARAFORMAT2 pf;
    ZeroMemory(&pf, sizeof(pf));
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask |= PFM_LINESPACING;
    pf.bLineSpacingRule =5;
    pf.dyLineSpacing = 24;
    m_GuiRT_Output.SetSelAll ();
    m_GuiRT_Output.SetParaFormat(pf);
    return 0;
  }
};

class CViewIO : public dockwins::CDockingTabChild<CViewIO>
{
public:
  CMenuHandle m_AttachMenu;
  typedef    dockwins::CDockingTabChild<CViewIO> baseClass;
  typedef CViewIO  thisClass;
  CViewIO_T m_panel_arm7;

  CViewIO (void) {}
  ~CViewIO (void) {}

  DECLARE_WND_CLASS_FX(_T ("CViewIO"), 0, COLOR_WINDOW, IDI_IOMAPPER)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_SETFOCUS, OnFocus)
    CHAIN_MSG_MAP (baseClass)
    REFLECT_NOTIFICATIONS()
  END_MSG_MAP ()

  LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
  { 
    if (m_panel_arm7.m_GuiRT_Output.IsWindow () != FALSE)
      m_panel_arm7.m_GuiRT_Output.SetFocus ();
    // post to parent tab container deal, no care it.
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) 
  { 
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_IO, MF_ENABLED | MF_BYCOMMAND );
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
    ::SetWindowPos(m_panel_arm7.m_GuiRT_Output, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top ,SWP_NOZORDER | SWP_NOACTIVATE);
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) 
  {
    m_panel_arm7.Create (m_hWnd, rcDefault, _T (""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  WS_EX_COMPOSITED);
    // m_panel_arm7.UpdateOutput (0);
    //ResizeClient (CViewIO_T ::width, CViewIO_T::height);
    
    return 0;
  }

  LRESULT OnTestCreate (HWND parent, HMENU menu) {
    m_AttachMenu = menu;
    m_AttachMenu.EnableMenuItem (ID_DEBUGGER_IO, MF_DISABLED | MF_BYCOMMAND );
    ShowWindow (SW_SHOWNORMAL);
    return  0;
  }

  void kCreate (HWND parent, struct gba *dev) {
    m_panel_arm7.m_GBA = dev;
    Create (parent, CRect (0,0, CViewIO_T ::width, CViewIO_T::height), _T ("IO"), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,  WS_EX_TRANSPARENT);
    m_panel_arm7.UpdateGBAIO_LCD_Output ();

  //   Win32_ImpulseBoy::IBG_ViewIO = this;
  }
};
// TODO: lr35902 (z80)


# endif 