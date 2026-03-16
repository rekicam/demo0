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

# include "stdwtl.h"
# include <thread>
# include "CGameVideo.hxx"
# include "CGameAudio.hxx"
# include "CGameInput.hxx"
# include "CFileDialogFilter.hxx"
# include "CKeySettings.hxx"
# include <atlutil.h>
# include <atlfile.h>
# include <dockwins/TabDockingBox.h>
# include "CVC7LikeExCaption.hxx"
# include "CDynamicMessageMap.hxx"
#define GBA_PROTOTYPE_IMPORT
# include "..\gba\gba.h"
# include "..\GBA\cpu.inl"
# include "CViewComment.hxx"
# include "CViewOutput.hxx"
# include "CViewBreakpoint.hxx"
# include "CViewIO.hxx"
# include "CViewWatch.hxx"
# include "CViewPalette.hxx"
# include "CViewTile.hxx"
# include "CViewNametable.hxx"
# include "CViewOam.hxx"

class CDebugger;
class CGPUViewer;

/*
template <bool t_bVertical, class T>
struct CSplitterWindowREF : public CSplitterWindowT<t_bVertical>,
                                              CDynamicMessageMap<CSplitterWindowT<t_bVertical>, T> {
public:
  CSplitterWindowREF (T *TThis_):CDynamicMessageMap<CSplitterWindowT<t_bVertical>>, T> (this, TThis_) {}
  ~CSplitterWindowREF (void) {}
  typedef CWindowImpl<CRichEditREF<T, TWinTraits>> baseClass;
  typedef CRichEditREF thisClass;
  
  DECLARE_WND_CLASS(_T ("CSplitterWindowREF"))
  SET_DYNA_MSG_MAP()
};
*/

// Object :: Controller_Mapper
struct Controller_Mapper {

  BYTE up;
  BYTE down;
  BYTE left;
  BYTE right;
  BYTE weak_left;
  BYTE weak_right;
  BYTE b;
  BYTE a;
  BYTE select;
  BYTE start;

  Controller_Mapper (void) {
    setDefault ();
  }

  void setDefault (void) {
    up = DIK_W;
    down = DIK_S;
    left = DIK_A;
    right = DIK_D;
    b = DIK_J;
    a = DIK_K;
    weak_left = DIK_U;
    weak_right = DIK_I;
    select = DIK_SPACE;
    start = DIK_RETURN;
  }

  void GetMask (BYTE *set) {
    set[0] = up;
    set[1] = down;
    set[2] = left;
    set[3] = right;
    set[4] = weak_left;
    set[5] = weak_right;
    set[6] = b;
    set[7] = a;
    set[8] = select;
    set[9] = start;
  }

  void SetMask (BYTE *set) {
    up = set[0];
    down = set[1];
    left = set[2];
    right = set[3];
    weak_left = set[4];
    weak_right = set[5];
    b = set[6];
    a = set[7];
    select = set[8];
    start = set[9];
  }
};

// < FORWARD - CMainFrame >========================================================
class CMainFrame : public CFrameWindowImpl <CMainFrame>, public CIdleHandler {
public:
  typedef CFrameWindowImpl <CMainFrame> baseClass;
  typedef CMainFrame thisClass;

  template <int t_iStep>
  LRESULT OnResizeClient (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    ResizeClient (t_iStep * 240, t_iStep * 160);
    CenterWindow ();
    return 0;
  }

  DECLARE_FRAME_WND_CLASS (_T ("CMainFrame"), IDR_MAINFRAME)

  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_DROPFILES, OnDropFile)
    COMMAND_ID_HANDLER (ID_SIZE_X1, OnResizeClient <1>)
    COMMAND_ID_HANDLER (ID_SIZE_X2, OnResizeClient <2>)
    COMMAND_ID_HANDLER (ID_SIZE_X3, OnResizeClient <3>)
    COMMAND_ID_HANDLER (ID_SIZE_X4, OnResizeClient <4>)
    COMMAND_ID_HANDLER (ID_DIRECTX_BILINEAR, OnDirect3D_Bilinear)
    COMMAND_ID_HANDLER (ID_DIRECTX_NORMAL, OnDirect3D_Std)
    COMMAND_ID_HANDLER (ID_DIRECTX_FAST, OnDirect3D_Fast)
    COMMAND_ID_HANDLER (ID_GRAPHICS_OPENGL, OnGL_1)
    COMMAND_ID_HANDLER (ID_GRAPHICS_GDI, OnGDI_DIB);
    COMMAND_ID_HANDLER (ID_FILTER_NONE, OnFilter_NO)
    COMMAND_ID_HANDLER (ID_FILTER_LINEAR, OnFilter_Interp)
    COMMAND_ID_HANDLER (ID_FILTER_LINEAR2, OnFilter_Interp_2)
    COMMAND_ID_HANDLER (ID_FILTER_SCANLINE, OnFilter_Scanline)
    COMMAND_ID_HANDLER (ID_FILE_LOADROM, OnLoadRom)
    COMMAND_ID_HANDLER (ID_SETTINGS_CONTROLLER, OnKeyPadSettings)
    COMMAND_ID_HANDLER (ID_SETTINGS_RUN, OnRunEmulator);
    COMMAND_ID_HANDLER (ID_MODE_FORGEGROUND, OnKeyModeForge)
    COMMAND_ID_HANDLER (ID_MODE_BACKGROUND, OnKeyModeBack)
    COMMAND_ID_HANDLER (ID_ADV_DEBUGGER, OnOpenDebuggerWindow)
    COMMAND_ID_HANDLER (ID_ADV_GPUVIEWER, OnOpenGPUWindow)
    CHAIN_MSG_MAP(baseClass)
  END_MSG_MAP()

  BOOL OnIdle (void);
  CMainFrame (void): m_GameVideo (NULL), m_bPause (TRUE), m_bLoadRom (FALSE), m_i64TimeStart (0) {}
  ~CMainFrame (void) { Detach (); }

  static void callc GBA_Controller_Callback  (struct gba *gba, void *buf);
  static void callc GBA_Video_Callback  (struct gba *gba, ppu_framebuffer *buf);
  static void callc GBA_APU_SwapBuffer  (struct gba *gba, apu_framebuffer *buf);
  static void callc GBA_VSync  (struct gba *gba);
  static void callc GBA_Breakpoint_Callback  (struct gba *gba, breakpoint *bp);

  int ResetRom (CString *strPathRom);
  int SaveBattery (void);
  int ReadBattery (CString *strPathRomSave);

  LRESULT OnRunEmulator (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnPauseEmulator (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnLoadRom (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnKeyModeForge(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnKeyModeBack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnKeyPadSettings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnOpenDebuggerWindow(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnOpenGPUWindow (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnDirect3D_Fast (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnDirect3D_Std (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);

  // Graphics && Filter 
  LRESULT CMainFrame::OnDirect3D_Bilinear (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnGL_1 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnGDI_DIB (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnFilter_NO (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnFilter_Interp (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) ;
  LRESULT CMainFrame::OnFilter_Interp_2 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT CMainFrame::OnFilter_Scanline (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);

  // LRESULT OnViewComment(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  // LRESULT OnViewOutput(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
  LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnDropFile (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  void resetInternalObject (void) {}
  void emulatorEntry (void) {}

  class CDebugger *m_Debugger;
  class CGPUViewer *m_GPUViewer;

  class CGameVideoRoot *m_GameVideo;
  class CGameAudioSample16_44100Hz_2 *m_GameAudio;
  class CGameInput m_GameInput;
  struct Controller_Mapper m_Keypad_Settings;

  struct gba *m_GBA;

  BOOL m_bPause;
  BOOL m_bLoadRom;
  CString m_PathROM;
  CString m_PathSave;

  __int64 m_i64TimeStart;

};

// < FORWARD - CDebugger >========================================================
class CDebugger : public dockwins::CDockingFrameImpl<CDebugger, CWindow, dockwins::CVC7LikeDockingFrameTraits>, CIdleHandler, CMessageFilter {
public: 
  // Some debugger's global settings..
  static const DWORD disasm_main_bg_clr = 0;
  static const DWORD disasm_main_text_clr = 0xFF1e;
  static const DWORD disasm_pchit_bg_clr = 0xb45a6e;
  static const DWORD disasm_breakpoint_bg_clr = 0xab99bd;
  static const DWORD register_main_bg_clr = 0x6e4e29;
  static const DWORD register_main_text_clr = 0xFFFFFF;
  static const DWORD register_auto_update_main_bg_clr = 0x6e4e29;
  static const DWORD register_auto_update_main_text_clr = 0xFFFFFF;
  static const DWORD register_old_tonew_update_clr = 0xFF0000;
  static const DWORD memory_main_bg_clr = 0;
  static const DWORD memory_main_text_clr = 0xFFFFFF;
  static const DWORD memory_auto_update_main_bg_clr = 0;
  static const DWORD memory_auto_update_main_text_clr = 0xFF1e;
  static const DWORD stack_main_bg_clr = 0x6e4e29;
  static const DWORD stack_main_text_clr = 0xFFFFFF;
  static const DWORD stack_auto_update_main_bg_clr = 0x000000;
  static const DWORD stack_auto_update_main_text_clr = 0xFFFFFF;
  static const DWORD stack_shadow_init_clr = 0xFFFFFF;
  static const DWORD stack_shadow_push_clr = 0x008dfa;
  static const DWORD stack_shadow_pop_clr = 0xc6c6c6;
  static const DWORD stack_shadow_lr_bg_clr = 0xf0c810;
  static  const  int nes8bitfont_size = 9;

  // const DWORD 
  void SetDebuggerInterrupt (BOOL bInterrupt) {
    m_AttachFrame->m_bPause  = bInterrupt;
    if (bInterrupt)
      SetWindowText (_T ("AGB Debugger @ Interrupt"));
    else 
      SetWindowText (_T ("AGB Debugger @ Run"));
  }

public:
  typedef dockwins::CDockingFrameImpl<CDebugger, CWindow, dockwins::CVC7LikeDockingFrameTraits> baseClass;
  typedef CDebugger thisClass;

  DECLARE_FRAME_WND_CLASS(_T("CDebugger"), IDR_DEBUGGER)
  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_SIZE, OnSize)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    COMMAND_ID_HANDLER (ID_MAIN_STARTDEBUGGERMODE, OnStartDebug)
    COMMAND_ID_HANDLER (ID_MAIN_CLOSEDEBUGGERMODE, OnCloseDebug)
    COMMAND_ID_HANDLER (ID_DEBUGGER_COMMENT, OnViewComment)
    COMMAND_ID_HANDLER (ID_DEBUGGER_OUTPUT, OnViewOutput)
    COMMAND_ID_HANDLER (ID_DEBUGGER_BREAKPOINT, OnViewBreakpoint)
    COMMAND_ID_HANDLER (ID_DEBUGGER_IO, OnViewIO)
    COMMAND_ID_HANDLER (ID_DEBUGGER_WATCH, OnViewWatch)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP()

  CDebugger (struct gba *attachGBA, CMainFrame *attachFrame) : m_AttachFrame (attachFrame), m_GuiRT_Disasm (this),
  m_GuiRT_Register (this),
  m_GuiRT_Memory (this),
  m_GuiRT_Stack (this),
  m_ViewMemoryInfos (attachGBA),
  m_GBA (attachGBA),
  m_ViewRegisterInfos (attachGBA),
  m_ViewDisasmInfos (attachGBA),
  m_ViewStackInfos (attachGBA), 
  m_bMasterUpdate (FALSE)
  {}
  ~CDebugger (void) {   Detach (); }
  
  LRESULT OnSysCharMessage (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    // Mask alt failure warning sound effect
    bHandled= TRUE;
    return 0;
  }
  LRESULT CDebugger::OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = FALSE;
    return 0;
  }
  LRESULT CDebugger::OnClose (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    ShowWindow (SW_HIDE);
    bHandled = TRUE;
    return 0;
  }
  BOOL OnIdle (void) { 
    
    if (m_bMasterUpdate != FALSE) {
      m_ViewWatch.UpdateWatchInfos ();
      m_ViewIO.m_panel_arm7.TryAutoUpdate ();
      OnUpdateMemoryWork ();
      OnUpdateRegisterWork ();
    }
    return FALSE; 
  } 

  BOOL PreTranslateMessage(MSG* pMsg)
  {
    return FALSE;
  }

  LRESULT OnViewMemoryMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
      bHandled = FALSE;
      return 0;
  }

  LRESULT CDebugger::OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    //  m_ViewComment.kCreate 

    CRect rcClient;
    // Init docksetting.
    InitializeDockingFrame();
    m_ViewComment.kCreate (*this);
    m_ViewOutput.kCreate (*this);
    m_ViewBreakpoint.kCreate (*this, m_GBA);
    m_ViewIO.kCreate (*this, m_GBA);
    m_ViewWatch.kCreate (*this, m_GBA);

    // Init splitter pane. 
   m_hWndClient = m_SplitterVert.Create (*this, CRect (0, 0, 640, 320), NULL,
          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN , WS_EX_COMPOSITED | WS_EX_TRANSPARENT);
   m_SplitterRightHori.Create (m_SplitterVert, rcDefault, NULL,
          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_COMPOSITED | WS_EX_TRANSPARENT);
   m_SplitterRightHori_Vert.Create (m_SplitterRightHori, rcDefault, NULL,
          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_COMPOSITED | WS_EX_TRANSPARENT);
    // Init rich edit 
    m_GuiRT_Disasm.Create (m_SplitterVert, rcDefault, NULL, ES_WANTRETURN | ES_MULTILINE |  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_AUTOHSCROLL);
    m_GuiRT_Register.Create (m_SplitterRightHori_Vert, rcDefault, NULL, ES_WANTRETURN | ES_MULTILINE |  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL);
    m_GuiRT_Memory.Create (m_SplitterRightHori, rcDefault, NULL, ES_WANTRETURN | ES_MULTILINE |  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL);
    m_GuiRT_Stack.Create (m_SplitterRightHori_Vert, rcDefault, NULL, ES_WANTRETURN | ES_MULTILINE |  WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_AUTOHSCROLL);

    // Set base init text...
    m_GuiRT_Disasm.SetBackgroundColor (disasm_main_bg_clr);
    m_GuiRT_Register.SetBackgroundColor (register_main_bg_clr);
    m_GuiRT_Memory.SetBackgroundColor (memory_main_bg_clr);
    m_GuiRT_Stack.SetBackgroundColor (stack_main_bg_clr);
    // Set base font.
    HFONT hFont = CreateFont(-8, 0, 0, 0, 500, FALSE, FALSE, FALSE, 0, 500, 2, 1, 1, _T ("Press Start 2P"));
    m_GuiRT_Disasm.SetFont (hFont);
    m_GuiRT_Register.SetFont (hFont);
    m_GuiRT_Memory.SetFont (hFont);
    m_GuiRT_Stack.SetFont (hFont);
    // Disasm Window Base Settings  ----------------------------------------
    CHARFORMAT ch_format;
    ZeroMemory (&ch_format, sizeof(CHARFORMAT));

    m_GuiRT_Disasm.GetDefaultCharFormat(ch_format);
    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = disasm_main_text_clr;

    m_GuiRT_Disasm.SetDefaultCharFormat(ch_format);

    PARAFORMAT2 pf;
    ZeroMemory(& pf, sizeof (pf));
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask |= PFM_LINESPACING;
    pf.bLineSpacingRule =5;
    pf.dyLineSpacing = 24;
    m_GuiRT_Disasm.SetSelAll ();
    m_GuiRT_Disasm.SetParaFormat(pf);
    m_GuiRT_Disasm.SetSelNone ();

    // Register Window Base Settings -------------------------------
    m_GuiRT_Register.GetDefaultCharFormat(ch_format);
    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = register_main_text_clr;

    m_GuiRT_Register.SetDefaultCharFormat(ch_format);

    ZeroMemory(& pf, sizeof (pf));
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask |= PFM_LINESPACING;
    pf.bLineSpacingRule =5;
    pf.dyLineSpacing = 24;
    m_GuiRT_Register.SetSelAll ();
    m_GuiRT_Register.SetParaFormat(pf);
    m_GuiRT_Register.SetSelNone ();

    // Stack Window Base Settings -------------------------------------
    m_GuiRT_Stack.GetDefaultCharFormat(ch_format);
    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = stack_main_text_clr;

    m_GuiRT_Stack.SetDefaultCharFormat(ch_format);

    ZeroMemory(& pf, sizeof (pf));
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask |= PFM_LINESPACING;
    pf.bLineSpacingRule =5;
    pf.dyLineSpacing = 24;
    m_GuiRT_Stack.SetSelAll ();
    m_GuiRT_Stack.SetParaFormat(pf);
    m_GuiRT_Stack.SetSelNone ();

    // Memory Window Base Settings ------------------------
    m_GuiRT_Memory.GetDefaultCharFormat(ch_format);
    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = memory_main_text_clr;

    m_GuiRT_Memory.SetDefaultCharFormat(ch_format);

    ZeroMemory(& pf, sizeof (pf));
    pf.cbSize = sizeof(PARAFORMAT2);
    pf.dwMask |= PFM_LINESPACING;
    pf.bLineSpacingRule =5;
    pf.dyLineSpacing = 24;
    m_GuiRT_Memory.SetSelAll ();
    m_GuiRT_Memory.SetParaFormat(pf);
    m_GuiRT_Memory.SetSelNone ();

    m_GuiRT_Disasm.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);

    m_GuiRT_Memory.add_msg_map (WM_SYSKEYUP, & CDebugger::OnSysKeyUp_ViewMemory);
    m_GuiRT_Memory.add_msg_map (WM_SIZE, & CDebugger::OnSize_ViewMemory);
    m_GuiRT_Memory.add_msg_map (WM_HSCROLL, & CDebugger::OnHScroll_ViewMemory);
    m_GuiRT_Memory.add_msg_map (WM_VSCROLL, & CDebugger::OnVScroll_ViewMemory);
    m_GuiRT_Memory.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);
    m_GuiRT_Memory.add_msg_map (WM_MOUSEWHEEL, & CDebugger::OnMouseWheel_ViewMemory);
    m_GuiRT_Memory.add_msg_map (OCM_NOTIFY, & CDebugger::OnRefNotify_ViewMemory);

    m_GuiRT_Register.add_msg_map (WM_SYSKEYUP, & CDebugger::OnSysKeyUp_ViewRegister);
    m_GuiRT_Register.add_msg_map (WM_HSCROLL, & CDebugger::OnHScroll_ViewRegister);
    m_GuiRT_Register.add_msg_map (WM_VSCROLL, & CDebugger::OnVScroll_ViewRegister);
    m_GuiRT_Register.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);
    m_GuiRT_Register.add_msg_map (WM_MOUSEWHEEL, & CDebugger::OnMouseWheel_ViewRegister);
    
    // m_GuiRT_Disasm.add_msg_map (WM_SYSKEYUP, & CDebugger::OnSysKeyUp_ViewDisasm);
    m_GuiRT_Disasm.add_msg_map (WM_HSCROLL, & CDebugger::OnHScroll_ViewDisasm);
    m_GuiRT_Disasm.add_msg_map (WM_VSCROLL, & CDebugger::OnVScroll_ViewDisasm);
    m_GuiRT_Disasm.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);
    m_GuiRT_Disasm.add_msg_map (WM_MOUSEWHEEL, & CDebugger::OnMouseWheel_ViewRegister);
    m_GuiRT_Disasm.add_msg_map (WM_SIZE, & CDebugger::OnSize_ViewDisasm);
    m_GuiRT_Disasm.add_msg_map (WM_SYSKEYUP, & CDebugger::OnSysKeyUp_ViewDisasm);
    m_GuiRT_Disasm.add_msg_map (WM_KEYUP, & CDebugger::OnKeyUp_ViewDisasm);

    // m_GuiRT_Stack.add_msg_map (WM_HSCROLL, & CDebugger::OnHScroll_ViewDisasm);
    m_GuiRT_Stack.add_msg_map (WM_VSCROLL, & CDebugger::OnVScroll_ViewStack);
    //m_GuiRT_Stack.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);
    m_GuiRT_Stack.add_msg_map (WM_MOUSEWHEEL, & CDebugger::OnMouseWheel_ViewRegister);
    m_GuiRT_Stack.add_msg_map (WM_SIZE, & CDebugger::OnSize_ViewStack);
    //m_GuiRT_Stack.add_msg_map (WM_SYSKEYUP, & CDebugger::OnSysKeyUp_ViewDisasm);
   // m_GuiRT_Stack.add_msg_map (WM_KEYUP, & CDebugger::OnKeyUp_ViewDisasm);


    m_GuiRT_Stack.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);
    m_GuiRT_Register.add_msg_map (WM_SYSCHAR, & CDebugger::OnSysCharMessage);

    // Adjust Splitter pane ...
    m_SplitterVert.SetSplitterPanes (m_GuiRT_Disasm, m_SplitterRightHori);
    m_SplitterVert.SetSplitterPos (320);
    m_SplitterRightHori.SetSplitterPos (160);
    m_SplitterRightHori.SetSplitterPanes (m_SplitterRightHori_Vert, m_GuiRT_Memory);
    m_SplitterRightHori_Vert.SetSplitterPos (160);
    m_SplitterRightHori_Vert.SetSplitterPanes (m_GuiRT_Register, m_GuiRT_Stack);

    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_DEBUGGER)), TRUE);
    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_DEBUGGER)), FALSE);

    CMessageLoop *msgLoop = _Module.GetMessageLoop ();
    msgLoop->AddIdleHandler (this);
    msgLoop->AddMessageFilter (this);

    return 0;
  }
  LRESULT OnStartDebug (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    gba_setdebug (m_GBA, true);
    m_bMasterUpdate = TRUE;
    return 0;
  }
  LRESULT OnCloseDebug (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    gba_setdebug (m_GBA, false);
    m_bMasterUpdate = FALSE;
    return 0;
  }

  LRESULT OnViewComment (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewComment.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewOutput (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewOutput.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewBreakpoint (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewBreakpoint.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewIO (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewIO.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewWatch (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewWatch.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  __forceinline
  int GetiElemSize (HWND hWindow) {
    CRect rc;
    :: GetClientRect (hWindow, rc);
    return rc.Height () / nes8bitfont_size;
  }

  static int GetRegionAddressStart (DWORD dwRegion) {
    static  const int address_Bank[] = {
      0x0000000,
      0x2000000,
      0x3000000,
      0x5000000,
      0x6000000,
      0x7000000,
      0x8000000,
      0x9000000,
      0xE000000
    };
    ATLASSERT (dwRegion < sizeof (address_Bank)/sizeof (address_Bank[0]));
    return address_Bank[dwRegion];
  }

  static int GetRegionSize (DWORD dwRegion) {
    static const int size_Bank[] = {
      0x0004000,
      0x0040000,
      0x0008000,
      0x0000400,
      0x0018000,
      0x0000400,
      0x1000000,
      0x1000000,
      0x0010000,
    };
    ATLASSERT (dwRegion < sizeof (size_Bank)/sizeof (size_Bank[0]));
    return size_Bank[dwRegion];
  }

  static void CancelSomeInputStatus (CWindow &window) {
   // Cancel the current input state? I really don't know.
   // But he does work. That's enough.
   window.EnableWindow (FALSE);
   window.EnableWindow (TRUE);
  }

  // <View-StackWork > =======================================================================
  struct ViewStackWorkInfos {
   ViewStackWorkInfos (struct gba *attachGBA): 
       GBA (attachGBA), 
               bAutoUpdate (FALSE),
          dwMiddleAddress  (0) {}
   ~ViewStackWorkInfos (void) {}
   struct gba *GBA;
   BOOL bAutoUpdate;
   DWORD dwMiddleAddress; 
   DWORD dwLeftTop;
  } m_ViewStackInfos;

  void OnUpdateStackWork (BOOL bForceUpdate = FALSE, BOOL bFollowSP = FALSE) {
    if (m_ViewStackInfos.bAutoUpdate != FALSE || bForceUpdate) {
      SCROLLINFO scrBarH;
      SCROLLINFO scrBarV;
      
      scrBarH.cbSize = 
      scrBarV.cbSize = sizeof (SCROLLBARINFO);
      scrBarH.fMask =
      scrBarV.fMask = SIF_ALL;

      m_GuiRT_Stack.GetScrollInfo ( SB_HORZ, & scrBarH);
      m_GuiRT_Stack.GetScrollInfo ( SB_VERT, & scrBarV);
      if (bFollowSP == FALSE) {
        UpdateStackOutput (m_ViewStackInfos.dwMiddleAddress);
        m_GuiRT_Stack.SetScrollInfo ( SB_HORZ, & scrBarH, TRUE);
        m_GuiRT_Stack.SetScrollInfo ( SB_VERT, & scrBarV, TRUE);
      } else 
        UpdateStackOutput (m_ViewStackInfos.GBA->arm7.regs[13].blk & -4);
        scrBarV.nPos = (m_ViewStackInfos.dwLeftTop & -4) >> 2;
        m_GuiRT_Stack.SetScrollInfo ( SB_HORZ, & scrBarH, TRUE);
        m_GuiRT_Stack.SetScrollInfo ( SB_VERT, & scrBarV, TRUE);
    }
  }

  void UpdateStackOutput (DWORD dwMiddleStart) {
    int iElem = GetiElemSize (m_GuiRT_Stack);
    if (iElem <= 0)
      return ;
    else {}

    if (m_AttachFrame->m_bLoadRom == FALSE) {
      m_GuiRT_Stack.SetWindowText (_T ("Oops! Nothing to do in stack window"));
      return ; // nothing to do.
    }

     __declspec (align (32))    uint8_t bLRShadowGroup[0x200];
    uint8_t *pShadowLR = & bLRShadowGroup[0];
    if (iElem <= sizeof (bLRShadowGroup)/ sizeof ( bLRShadowGroup[0]) )
      ZeroMemory (bLRShadowGroup, sizeof (bLRShadowGroup));
    else {
      pShadowLR = (uint8_t *)malloc (sizeof (uint8_t) *iElem);
      ZeroMemory (pShadowLR, sizeof (uint8_t) *iElem);
    }

    // Calc left-top address.
    dwMiddleStart &= 0x7FFC;
    int iHalfOffset = iElem / 2 * 4;
    if ( ((int) dwMiddleStart) - iHalfOffset < 0)
      dwMiddleStart = 0;
    else if (dwMiddleStart + iHalfOffset >= 0x8000)
      dwMiddleStart = 0x8000 - iHalfOffset;
    else 
      dwMiddleStart = dwMiddleStart;
    int iLefTopAddress = dwMiddleStart - iHalfOffset;
    m_ViewStackInfos.dwMiddleAddress = dwMiddleStart;

    CString StringContent;
    struct stack_trace_chunk *pStackTraceInfos =  & m_ViewStackInfos.GBA->dbg.stack_trace[iLefTopAddress >> 2];
    uint32_t *pStackMem = (uint32_t *)& m_ViewStackInfos.GBA->mem.IRam[iLefTopAddress & 0x7FFC];
    DWORD dwStack = m_ViewStackInfos.GBA->arm7.regs[13].blk & -4;

    if (iLefTopAddress < 0)
      iLefTopAddress = 0;

    m_ViewStackInfos.dwLeftTop = iLefTopAddress;
    for (int Id = 0; Id != iElem; Id++) {
      // // 1234567->1234567 TRIGGER(A:%%R_ID)->1234567  PUSH
      const DWORD addr = iLefTopAddress + 0x3000000;
      StringContent.AppendFormat (_T ("%07X->%08X TRIGGER(%s:%02d)->%07X %s %s\n"), 
             addr,
             pStackMem[0],
             pStackTraceInfos->thumb ? _T ("T") : _T ("A"),
             pStackTraceInfos->r_id,
             pStackTraceInfos->do_addr,
             pStackTraceInfos->status ?   ( (pStackTraceInfos->status == 1) ? _T ("PUSH") : _T ("POP "))  : _T ("INIT"),
             addr == dwStack ? _T ("!!!") : _T (""));

      pShadowLR [Id] = pStackTraceInfos->status;
      if (pStackTraceInfos->r_id == 14)
        pShadowLR [Id] |= 0x80;
      else {}
      pStackTraceInfos++;
      pStackMem ++;
      iLefTopAddress += 4;
    }
    m_GuiRT_Stack.SetWindowText (StringContent);

    // Shadow color .
    for (int Id = 0; Id != iElem; Id++) {
      uint8_t content = pShadowLR[Id];
      DWORD dwMask = CFM_COLOR;
      DWORD text_clr;
      if (content & 0x80)
        dwMask |= CFM_BACKCOLOR;
      content &= 0x7F;
      if (content == 0)
        text_clr = stack_shadow_init_clr;
      else if (content == 1)
        text_clr = stack_shadow_push_clr;
      else 
        text_clr = stack_shadow_pop_clr;
      SetUpdateColor (& m_GuiRT_Stack, Id, dwMask, stack_shadow_lr_bg_clr, text_clr, FALSE);
    }
    m_GuiRT_Stack.SetSelNone ();
    if (pShadowLR != & bLRShadowGroup[0])
      free (pShadowLR);
  }

  LRESULT OnSize_ViewStack (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    CRect rc;
    bHandled = TRUE;
    m_GuiRT_Stack.GetClientRect (rc);
    int iElem = GetiElemSize (m_GuiRT_Stack);
    if (rc.Width  () <= 0 
      || rc.Height () <= 0
      || iElem <= 0)
      return 1;
    // m_GuiRT_Stack.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);

    SCROLLINFO sbarinfos2;
    sbarinfos2.cbSize = sizeof (SCROLLBARINFO);
    sbarinfos2.fMask = SIF_ALL;
    m_GuiRT_Stack.GetScrollInfo (SB_VERT, & sbarinfos2);
    sbarinfos2.nPage = iElem;
    sbarinfos2.nMax = 0x8000 >> 2;  
    sbarinfos2.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    UpdateStackOutput (m_ViewStackInfos.dwMiddleAddress);
    sbarinfos2.nPos = m_ViewStackInfos.dwMiddleAddress >> 2;
    m_GuiRT_Stack.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    m_GuiRT_Stack.SetScrollInfo (SB_VERT, & sbarinfos2);
    return 0;
  }

  LRESULT OnVScroll_ViewStack (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    int iElem = GetiElemSize (m_GuiRT_Stack);
    if (iElem <= 0)
      return 0;
    else {}

    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    DWORD dwShowPos;
    SCROLLINFO scrBar;

    // m_GuiRT_Disassembly.line
    scrBar.cbSize = sizeof (SCROLLBARINFO);
    scrBar.fMask = SIF_ALL;

    m_GuiRT_Stack.GetScrollInfo ( SB_VERT, & scrBar);
    scrBar.nPage = iElem;
    scrBar.nMax = 0x8000 >> 2;

    // scrBar.nPos = scrBar.nTrackPos;
    scrBar.fMask = SIF_TRACKPOS | SIF_POS | SIF_RANGE | SIF_PAGE;
    dwShowPos = scrBar.nTrackPos;

    switch (dwMainSBCode) {
    case SB_THUMBTRACK:
      scrBar.nPos = scrBar.nTrackPos;
      break;
    case SB_THUMBPOSITION: 
      scrBar.nPos = scrBar.nTrackPos;
      break;

    case SB_ENDSCROLL: 
      dwShowPos = scrBar.nPos;
      break;

    case  SB_LINEUP:
      dwShowPos = scrBar.nPos;

      if (dwShowPos != 0) 
        dwShowPos--;
      scrBar.nPos = dwShowPos;
      break;

    case  SB_LINEDOWN:        
      dwShowPos = ++scrBar.nPos;
      scrBar.nPos = dwShowPos;
      break;
    case  SB_PAGEUP:    
      dwShowPos = scrBar.nPos;

      if (dwShowPos >= iElem) 
        dwShowPos-= iElem;
      scrBar.nPos = dwShowPos;
      break;
    case SB_PAGEDOWN:        
      dwShowPos = scrBar.nPos + iElem;
      scrBar.nPos = dwShowPos;
      break;
    case  SB_TOP:      
    case  SB_BOTTOM:   
      break;
    default:
      break;
    }
    // Show Stack text.
    UpdateStackOutput ( (dwShowPos + iElem/ 2) * 4 );
    // m_GuiRT_Stack.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    m_GuiRT_Stack.SetScrollInfo ( SB_VERT, & scrBar, TRUE);
    return 0;
  }

  // <View-MemoryWork > =======================================================================
  struct ViewMemoryWorkInfos {
   ViewMemoryWorkInfos (struct gba *attachGBA): 
       GBA (attachGBA), 
               bAutoUpdate (FALSE),
          dwCurrentRegion (0),
          dwCurrentAdressStart  (0),
          dwCurrentFormat (0) {}
   ~ViewMemoryWorkInfos (void) {}
   int GetRegionAddressStart (void) { return CDebugger:: GetRegionAddressStart (dwCurrentRegion); }
   int GetRegionSize (void) { return CDebugger:: GetRegionSize (dwCurrentRegion); }
   struct gba *GBA;
   BOOL bAutoUpdate;
   DWORD dwCurrentRegion;
   DWORD dwCurrentAdressStart; // !!!: offset in current region!
   DWORD dwCurrentFormat;
  } m_ViewMemoryInfos;

  // iCodeRegion := -1 (keep old)
  void ResetMemoryRegion (int iCodeRegion = -1, BOOL bSetPos = FALSE, int iSetPos = -1) {
    SCROLLINFO scrBar;
    int iElem = GetiElemSize (m_GuiRT_Memory);
    if (iElem <= 0)
      return ;
    else {}

    scrBar.cbSize = sizeof (SCROLLBARINFO);
    scrBar.fMask = SIF_ALL;
    m_GuiRT_Memory.GetScrollInfo (SB_VERT, & scrBar);

    scrBar.cbSize = sizeof (SCROLLBARINFO);
    scrBar.nMin = 0;
    scrBar.nPos =0;
    scrBar.nPage = iElem - 2;
    scrBar.fMask = SIF_ALL;

    if (iCodeRegion != -1)
      m_ViewMemoryInfos.dwCurrentRegion = iCodeRegion;
    else {}

    scrBar.nMax = m_ViewMemoryInfos.GetRegionSize () >> 4;
    if (bSetPos != FALSE)
      scrBar.nPos = iSetPos;
    else {}
    m_GuiRT_Memory.SetScrollInfo (SB_VERT, & scrBar);
  }

  LRESULT OnSize_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    CRect rc;
    bHandled = TRUE;
    m_GuiRT_Memory.GetClientRect (rc);
    int iElem = GetiElemSize (m_GuiRT_Memory);
    if (rc.Width  () <= 0 
      || rc.Height () <= 0
      || iElem <= 2)
      return 1;
    // m_GuiRT_Memory.SetWindowPos(NULL, rc.left, rc.top, rc.Width (), rc.Height () ,SWP_NOZORDER | SWP_NOACTIVATE);

    SCROLLINFO sbarinfos2;
    sbarinfos2.cbSize = sizeof (SCROLLBARINFO);
    sbarinfos2.fMask = SIF_ALL;
    m_GuiRT_Memory.GetScrollInfo (SB_VERT, & sbarinfos2);
    sbarinfos2.nPage = iElem - 2;
    sbarinfos2.nMax = m_ViewMemoryInfos.GetRegionSize () >> 4;
    sbarinfos2.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    m_ViewMemoryInfos.dwCurrentAdressStart = sbarinfos2.nPos;
    UpdateMemoryOutput (m_ViewMemoryInfos.GetRegionAddressStart () +sbarinfos2.nPos *16, m_ViewMemoryInfos.dwCurrentFormat);
    m_GuiRT_Memory.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    m_GuiRT_Memory.SetScrollInfo (SB_VERT, & sbarinfos2);
    return 0;
  }

  LRESULT OnVScroll_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    int iElem = GetiElemSize (m_GuiRT_Memory);
    if (iElem <= 2)
      return 0;
    else {}

    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    DWORD dwShowPos;
    SCROLLINFO scrBar;

    // m_GuiRT_Disassembly.line
    scrBar.cbSize = sizeof (SCROLLBARINFO);
    scrBar.fMask = SIF_ALL;

    m_GuiRT_Memory.GetScrollInfo ( SB_VERT, & scrBar);
    scrBar.nPage = iElem - 2;
    scrBar.nMax = m_ViewMemoryInfos.GetRegionSize () >> 4;

    // scrBar.nPos = scrBar.nTrackPos;
    scrBar.fMask = SIF_TRACKPOS | SIF_POS | SIF_RANGE | SIF_PAGE;
    dwShowPos = scrBar.nTrackPos;
    bHandled = TRUE;

    switch (dwMainSBCode) {
    case SB_THUMBTRACK:
      scrBar.nPos = scrBar.nTrackPos;
      break;
    case SB_THUMBPOSITION: 
      scrBar.nPos = scrBar.nTrackPos;
      break;

    case SB_ENDSCROLL: 
      dwShowPos = scrBar.nPos;
      break;

    case  SB_LINEUP:
      dwShowPos = scrBar.nPos;

      if (dwShowPos != 0) 
        dwShowPos--;
      scrBar.nPos = dwShowPos;
      break;

    case  SB_LINEDOWN:        
      dwShowPos = ++scrBar.nPos;
      scrBar.nPos = dwShowPos;
      break;
    case  SB_PAGEUP:    
      dwShowPos = scrBar.nPos;

      if (dwShowPos >= iElem) 
        dwShowPos-= iElem;
      scrBar.nPos = dwShowPos;
      break;
    case SB_PAGEDOWN:        
      dwShowPos = scrBar.nPos + iElem;
      scrBar.nPos = dwShowPos;
      break;
    case  SB_TOP:      
    case  SB_BOTTOM:   
      break;
    default:
      break;
    }
    // Show Memory text.
    
    m_ViewMemoryInfos.dwCurrentAdressStart = dwShowPos;
    UpdateMemoryOutput (m_ViewMemoryInfos.GetRegionAddressStart () + dwShowPos * 16, m_ViewMemoryInfos.dwCurrentFormat);
    // m_GuiRT_Memory.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    m_GuiRT_Memory.SetScrollInfo ( SB_VERT, & scrBar, TRUE);
    return 0;
  }

  LRESULT OnHScroll_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Memory.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Memory.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
      m_GuiRT_Memory.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
    } else if (dwMainSBCode == SB_ENDSCROLL) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Memory.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      CancelSomeInputStatus (m_GuiRT_Memory);
      m_GuiRT_Memory.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
      m_GuiRT_Memory.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    }
    return 0;
  }

  LRESULT OnMouseWheel_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    return 0;
  }
  LRESULT OnRefNotify_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    return 0;
  }
  LRESULT OnSysKeyUp_ViewMemory (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    bHandled= TRUE;
    switch (pMsg->wparam) {
    case '1':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 0; 
      break;
    case '2':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 1; 
      break;
    case '3':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 2; 
      break;
    case '4':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 3; 
      break;
    case '5':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 4; 
      break;
    case '6':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 5; 
      break;
    case '7':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 6; 
      break;
    case '8':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 7; 
      break;
    case '9':
      m_ViewMemoryInfos.dwCurrentAdressStart = 0;
      m_ViewMemoryInfos.dwCurrentRegion = 8; 
      break;
    case 'b':
    case 'B':
      m_ViewMemoryInfos.dwCurrentFormat = 0;
      break;
    case 'h':
    case 'H':
      m_ViewMemoryInfos.dwCurrentFormat = 1;
      break;
    case 'w':
    case 'W':
      m_ViewMemoryInfos.dwCurrentFormat = 2;
     break;
    case 'a':
    case 'A': {
      CHARFORMAT ch_format;
      ZeroMemory (&ch_format, sizeof(CHARFORMAT));
      m_GuiRT_Disasm.GetDefaultCharFormat(ch_format);
      ch_format.cbSize = sizeof(ch_format);
      ch_format.dwEffects &= ~CFE_AUTOCOLOR;
      ch_format.dwMask |= CFM_COLOR;

      if (m_ViewMemoryInfos.bAutoUpdate != FALSE) {
        ch_format.crTextColor = memory_main_text_clr;
        m_GuiRT_Memory.SetBackgroundColor (memory_main_bg_clr);
        m_ViewOutput.AppendText (_T ("Memory::Auto Update Close!\n"));
      } else {
        ch_format.crTextColor = memory_auto_update_main_text_clr;
        m_GuiRT_Memory.SetBackgroundColor (memory_auto_update_main_bg_clr);
        m_ViewOutput.AppendText (_T ("Memory::Auto Update Open!\n"));
        CancelSomeInputStatus (m_GuiRT_Memory);
        m_GuiRT_Memory.SetFocus ();
      }
      m_ViewMemoryInfos.bAutoUpdate = !m_ViewMemoryInfos.bAutoUpdate;
      m_GuiRT_Memory.SetDefaultCharFormat(ch_format);
      }
      return 0;
    case 'u':
    case 'U':
      break;
    default:
      return 0;
    }
    UpdateMemoryOutput (m_ViewMemoryInfos.dwCurrentAdressStart * 16+ m_ViewMemoryInfos.GetRegionAddressStart (), m_ViewMemoryInfos.dwCurrentFormat);
    ResetMemoryRegion (m_ViewMemoryInfos.dwCurrentRegion, TRUE, m_ViewMemoryInfos.dwCurrentAdressStart);
    return 0;
  }
  __forceinline
  char asciiFilter_charA (char ch) {
    signed char ch_t = ch;
    if (ch_t < 32)
      return ' ';
    else 
      return ch_t;
  }
  __forceinline
  wchar_t asciiFilter_charW (wchar_t ch) {
    signed short ch_t = ch;
    if (ch_t >= 32 && ch_t <= 127)
      return ch_t;
    else 
      return L' ';
  }
  // dwOutputFormat:: 0~2 byte | halfword | word
  void UpdateMemoryOutput (DWORD dwAddressStart, DWORD dwOutputFormat) {
    int iElem = GetiElemSize (m_GuiRT_Memory);
    if (iElem <= 0)
      return ;
    else {}
    if (m_AttachFrame->m_bLoadRom == FALSE) {
      m_GuiRT_Memory.SetWindowText (_T ("Oops! Nothing to do in memory window"));
      return ; // nothing to do.
    }
    union {
      UCHAR block_8[16];
      WORD block_16[8];
      DWORD block_32[4];
      float block_f32[4];
    } uMemAccessBlock;

    DWORD dwAddressLoop = dwAddressStart & 0xFFFFFFF;
    switch (dwOutputFormat) {
    case 0: 
      {
      CStringA str;
      str.Append ("         00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 0 1 2 3 4 5 6 7 8 9 A B C D E F\n"
                  "----------------------------------------------------------------------------------------\n");
      for (int Id = 2; Id < iElem; Id++) {
        uMemAccessBlock.block_16[0] = gba_fast_rhw (m_GBA, dwAddressLoop + 0);
        uMemAccessBlock.block_16[1] = gba_fast_rhw (m_GBA, dwAddressLoop + 2);
        uMemAccessBlock.block_16[2] = gba_fast_rhw (m_GBA, dwAddressLoop + 4);
        uMemAccessBlock.block_16[3] = gba_fast_rhw (m_GBA, dwAddressLoop + 6);
        uMemAccessBlock.block_16[4] = gba_fast_rhw (m_GBA, dwAddressLoop + 8);
        uMemAccessBlock.block_16[5] = gba_fast_rhw (m_GBA, dwAddressLoop + 10);
        uMemAccessBlock.block_16[6] = gba_fast_rhw (m_GBA, dwAddressLoop + 12);
        uMemAccessBlock.block_16[7] = gba_fast_rhw (m_GBA, dwAddressLoop + 14);

        str.AppendFormat ("%07X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X"
          " %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n", 
          dwAddressLoop,
          uMemAccessBlock.block_8[0],
          uMemAccessBlock.block_8[1],
          uMemAccessBlock.block_8[2],
          uMemAccessBlock.block_8[3],
          uMemAccessBlock.block_8[4],
          uMemAccessBlock.block_8[5],
          uMemAccessBlock.block_8[6],
          uMemAccessBlock.block_8[7],
          uMemAccessBlock.block_8[8],
          uMemAccessBlock.block_8[9],
          uMemAccessBlock.block_8[10],
          uMemAccessBlock.block_8[11],
          uMemAccessBlock.block_8[12],
          uMemAccessBlock.block_8[13],
          uMemAccessBlock.block_8[14],
          uMemAccessBlock.block_8[15],
          asciiFilter_charA (uMemAccessBlock.block_8[0]),
          asciiFilter_charA (uMemAccessBlock.block_8[1]),
          asciiFilter_charA (uMemAccessBlock.block_8[2]),
          asciiFilter_charA (uMemAccessBlock.block_8[3]),
          asciiFilter_charA (uMemAccessBlock.block_8[4]),
          asciiFilter_charA (uMemAccessBlock.block_8[5]),
          asciiFilter_charA (uMemAccessBlock.block_8[6]),
          asciiFilter_charA (uMemAccessBlock.block_8[7]),
          asciiFilter_charA (uMemAccessBlock.block_8[8]),
          asciiFilter_charA (uMemAccessBlock.block_8[9]),
          asciiFilter_charA (uMemAccessBlock.block_8[10]),
          asciiFilter_charA (uMemAccessBlock.block_8[11]),
          asciiFilter_charA (uMemAccessBlock.block_8[12]),
          asciiFilter_charA (uMemAccessBlock.block_8[13]),
          asciiFilter_charA (uMemAccessBlock.block_8[14]),
          asciiFilter_charA (uMemAccessBlock.block_8[15]) );
        dwAddressLoop += 16;
      }
#ifdef _UNICODE
      m_GuiRT_Memory.SetWindowText (CString (str));
#else 
      m_GuiRT_Memory.SetWindowText (str);
#endif 
      }
      break;
    case 1:
      {
      CStringW str;
      str.Append (L"         0000 0002 0004 0006 0008 000A 000C 000E 0 1 2 3 4 5 6 7 -\n"
                  L"--------------------------------------------------------------------\n");
      for (int Id = 2; Id < iElem; Id++) {
        uMemAccessBlock.block_16[0] = gba_fast_rhw (m_GBA, dwAddressLoop + 0);
        uMemAccessBlock.block_16[1] = gba_fast_rhw (m_GBA, dwAddressLoop + 2);
        uMemAccessBlock.block_16[2] = gba_fast_rhw (m_GBA, dwAddressLoop + 4);
        uMemAccessBlock.block_16[3] = gba_fast_rhw (m_GBA, dwAddressLoop + 6);
        uMemAccessBlock.block_16[4] = gba_fast_rhw (m_GBA, dwAddressLoop + 8);
        uMemAccessBlock.block_16[5] = gba_fast_rhw (m_GBA, dwAddressLoop + 10);
        uMemAccessBlock.block_16[6] = gba_fast_rhw (m_GBA, dwAddressLoop + 12);
        uMemAccessBlock.block_16[7] = gba_fast_rhw (m_GBA, dwAddressLoop + 14);

        str.AppendFormat (L"%07X: %04X %04X %04X %04X %04X %04X %04X %04X"
          L" %c %c %c %c %c %c %c %c \n", 
          dwAddressLoop,
          uMemAccessBlock.block_16[0],
          uMemAccessBlock.block_16[1],
          uMemAccessBlock.block_16[2],
          uMemAccessBlock.block_16[3],
          uMemAccessBlock.block_16[4],
          uMemAccessBlock.block_16[5],
          uMemAccessBlock.block_16[6],
          uMemAccessBlock.block_16[7],
          asciiFilter_charW (uMemAccessBlock.block_16[0]),
          asciiFilter_charW (uMemAccessBlock.block_16[1]),
          asciiFilter_charW (uMemAccessBlock.block_16[2]),
          asciiFilter_charW (uMemAccessBlock.block_16[3]),
          asciiFilter_charW (uMemAccessBlock.block_16[4]),
          asciiFilter_charW (uMemAccessBlock.block_16[5]),
          asciiFilter_charW (uMemAccessBlock.block_16[6]),
          asciiFilter_charW (uMemAccessBlock.block_16[7]) );
        dwAddressLoop += 16;
      }
#ifdef _UNICODE
      m_GuiRT_Memory.SetWindowText (str);
#else 
      m_GuiRT_Memory.SetWindowText (CString (str));
#endif 
      }
      break;
    case 2:
      {
      CString str;
      str.Append (_T ("         00000000 00000004 00000008 0000000C    ------ F  L  O  A  T ------\n")
                  _T ("------------------------------------------------------------------------------\n"));
      for (int Id = 2; Id < iElem; Id++) {
        uMemAccessBlock.block_16[0] = gba_fast_rhw (m_GBA, dwAddressLoop + 0);
        uMemAccessBlock.block_16[1] = gba_fast_rhw (m_GBA, dwAddressLoop + 2);
        uMemAccessBlock.block_16[2] = gba_fast_rhw (m_GBA, dwAddressLoop + 4);
        uMemAccessBlock.block_16[3] = gba_fast_rhw (m_GBA, dwAddressLoop + 6);
        uMemAccessBlock.block_16[4] = gba_fast_rhw (m_GBA, dwAddressLoop + 8);
        uMemAccessBlock.block_16[5] = gba_fast_rhw (m_GBA, dwAddressLoop + 10);
        uMemAccessBlock.block_16[6] = gba_fast_rhw (m_GBA, dwAddressLoop + 12);
        uMemAccessBlock.block_16[7] = gba_fast_rhw (m_GBA, dwAddressLoop + 14);

        str.AppendFormat (_T ("%07X: %08X %08X %08X %08X")
          _T (" %f %f %f %f\n"), 
          dwAddressLoop,
          uMemAccessBlock.block_32[0],
          uMemAccessBlock.block_32[1],
          uMemAccessBlock.block_32[2],
          uMemAccessBlock.block_32[3],
          uMemAccessBlock.block_f32[0],
          uMemAccessBlock.block_f32[1],
          uMemAccessBlock.block_f32[2],
          uMemAccessBlock.block_f32[3]);
        dwAddressLoop += 16;
      }
      m_GuiRT_Memory.SetWindowText (str);
      }
      break;
    default:
      ATLASSERT (FALSE);
      break;
    }
  }

  //  Window - Memory Work.
  void OnUpdateMemoryWork (BOOL bForceUpdate = FALSE) {
    if (m_ViewMemoryInfos.bAutoUpdate != FALSE || bForceUpdate) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Memory.GetScrollInfo ( SB_VERT, & scrBar);
      UpdateMemoryOutput (m_ViewMemoryInfos.dwCurrentAdressStart * 16+ m_ViewMemoryInfos.GetRegionAddressStart (), m_ViewMemoryInfos.dwCurrentFormat);
      m_GuiRT_Memory.SetScrollInfo ( SB_VERT, & scrBar, FALSE);
    }
  }
  // <View-RegisterWork > =======================================================================
  struct ViewRegisterWorkInfos {
   ViewRegisterWorkInfos (struct gba *attachGBA): 
       GBA (attachGBA), 
       bAutoUpdate (FALSE) { ZeroMemory (& _swap_arm7, sizeof (_swap_arm7)); }
   ~ViewRegisterWorkInfos (void) {}
   struct gba *GBA;
   BOOL bAutoUpdate;
   struct arm7 _swap_arm7;
  } m_ViewRegisterInfos;

  void SetUpdateColor (int line, int charStart, int num, DWORD clr) {
    int n = m_GuiRT_Register.LineIndex (line);
    CHARRANGE ch_range;
    ch_range.cpMin = charStart + n;
    ch_range.cpMax = ch_range.cpMin + num;

    CHARFORMAT ch_format;
    ZeroMemory (& ch_format, sizeof (ch_format));

    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOCOLOR;
    ch_format.dwMask |= CFM_COLOR;
    ch_format.crTextColor = clr;

    m_GuiRT_Register.SetSel (ch_range);
    m_GuiRT_Register.SetSelectionCharFormat (ch_format);
  }

  LRESULT OnHScroll_ViewRegister (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
      m_GuiRT_Register.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
    } else if (dwMainSBCode == SB_ENDSCROLL) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      CancelSomeInputStatus (m_GuiRT_Memory);
      m_GuiRT_Register.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
      m_GuiRT_Register.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    }
    return 0;
  }
  LRESULT OnVScroll_ViewRegister (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.GetScrollInfo ( SB_VERT, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
      m_GuiRT_Register.SetScrollInfo ( SB_VERT, & scrBar, FALSE);
    } else if (dwMainSBCode == SB_ENDSCROLL) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Register.GetScrollInfo ( SB_VERT, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      CancelSomeInputStatus (m_GuiRT_Memory);
      m_GuiRT_Register.SetScrollInfo ( SB_VERT, & scrBar, FALSE);
      m_GuiRT_Register.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    }
    return 0;
  }
  LRESULT OnMouseWheel_ViewRegister (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    return 0;
  }
  LRESULT OnSysKeyUp_ViewRegister (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    bHandled= TRUE;
    switch (pMsg->wparam) {
    case 'a':
    case 'A': {
      CHARFORMAT ch_format;
      ZeroMemory (&ch_format, sizeof(CHARFORMAT));
      m_GuiRT_Register.GetDefaultCharFormat(ch_format);
      ch_format.cbSize = sizeof(ch_format);
      ch_format.dwEffects &= ~CFE_AUTOCOLOR;
      ch_format.dwMask |= CFM_COLOR;

      if (m_ViewRegisterInfos.bAutoUpdate != FALSE) {
        ch_format.crTextColor = memory_main_text_clr;
        m_GuiRT_Register.SetBackgroundColor (memory_main_bg_clr);
        m_ViewOutput.AppendText (_T ("Register::Auto Update Close!\n"));
      } else {
        ch_format.crTextColor = memory_auto_update_main_text_clr;
        m_GuiRT_Register.SetBackgroundColor (memory_auto_update_main_bg_clr);
        m_ViewOutput.AppendText (_T ("Register::Auto Update Open!\n"));
        CancelSomeInputStatus (m_GuiRT_Register);
        m_GuiRT_Register.SetFocus ();
      }
      m_ViewRegisterInfos.bAutoUpdate = !m_ViewRegisterInfos.bAutoUpdate;
      m_GuiRT_Register.SetDefaultCharFormat(ch_format);
      }
      return 0;
    case 'u':
    case 'U':
      break;
    default:
      return 0;
    }
    UpdateRegisterOutput ();
    return 0;
  }
  // dwOutputFormat:: 0~2 byte | halfword | word
  void UpdateRegisterOutput (void) {
    struct arm7 *cur = & m_ViewRegisterInfos.GBA->arm7;
    struct arm7 *old = & m_ViewRegisterInfos._swap_arm7;
    ATLASSERT (cur != NULL);
    if (m_AttachFrame->m_bLoadRom == FALSE) {
      m_GuiRT_Register.SetWindowText (_T ("Oops! Nothing to do in register window"));
      return ; // nothing to do.
    }
    if (memcmp (cur , old, sizeof (*cur)) == 0)
      return ;

    static LPCTSTR mode_gp[16] = {
      _T ("USR"), _T ("FIQ"), _T ("IRQ"), _T ("SVC"),
      _T ("UB-MODE"), _T ("UB-MODE"), _T ("UB-MODE"), _T ("UB-MODE"),
      _T ("UB-MODE"), _T ("UB-MODE"), _T ("UB-MODE"), _T ("UNDEF"),
      _T ("UB-MODE"), _T ("UB-MODE"), _T ("UB-MODE"), _T ("SYS")
    };

    LPCTSTR s =     _T ("SPSR := %08X <- %08X\n")// 0
                    _T ("CPSR := %08X <- %08X\n")// 1
                    _T ("     |\n")// 2
                    _T ("     |=====CPSR FIELD====\n")// 3
                    _T ("     |== M := %s <- %s\n") //4
                    _T ("     |== I := %d <- %d\n") //5
                    _T ("     |== F := %d <- %d\n")//6
                    _T ("     |== T := %d <- %d\n")//7
                    _T ("     |== N := %d <- %d\n")//8
                    _T ("     |== Z := %d <- %d\n")//9
                    _T ("     |== C := %d <- %d\n")//10
                    _T ("     |== V := %d <- %d\n") //11
                    _T ("     |===================\n\n")  //12    
                    _T ("R0 := %08X <- %08X\n")//14
                    _T ("R1 := %08X <- %08X\n")
                    _T ("R2 := %08X <- %08X\n")
                    _T ("R3 := %08X <- %08X\n")
                    _T ("R4 := %08X <- %08X\n")
                    _T ("R5 := %08X <- %08X\n")
                    _T ("R6 := %08X <- %08X\n")
                    _T ("R7 := %08X <- %08X\n")
                    _T ("R8 := %08X <- %08X\n")
                    _T ("R9 := %08X <- %08X\n")
                    _T ("R10:= %08X <- %08X\n")
                    _T ("R11:= %08X <- %08X\n")
                    _T ("R12:= %08X <- %08X\n")
                    _T ("R13:= %08X <- %08X\n")
                    _T ("R14:= %08X <- %08X\n")
                    _T ("R15:= %08X <- %08X\n")
                    _T ("PC:= %08X <- \n\n")

                    _T ("R8_BANK_^FIQ := %08X <- %08X\n")
                    _T ("R9_BANK_^FIQ := %08X <- %08X\n")
                    _T ("R10_BANK_^FIQ := %08X <- %08X\n")
                    _T ("R11_BANK_^FIQ := %08X <- %08X\n")
                    _T ("R12_BANK_^FIQ := %08X <- %08X\n")


                    _T ("R8_BANK_FIQ := %08X <- %08X\n")
                    _T ("R9_BANK_FIQ := %08X <- %08X\n")
                    _T ("R10_BANK_FIQ := %08X <- %08X\n")
                    _T ("R11_BANK_FIQ := %08X <- %08X\n")
                    _T ("R12_BANK_FIQ := %08X <- %08X\n")


                    _T ("R13_SVC := %08X <- %08X\n")
                    _T ("R13_IRQ := %08X <- %08X\n")
                    _T ("R13_USR := %08X <- %08X\n")
                    _T ("R13_FIQ := %08X <- %08X\n")
                    _T ("R13_ABT := %08X <- %08X\n")
                    _T ("R13_UDEF := %08X <- %08X\n")



                    _T ("R14_SVC := %08X <- %08X\n")
                    _T ("R14_IRQ := %08X <- %08X\n")
                    _T ("R14_USR := %08X <- %08X\n")
                    _T ("R14_FIQ := %08X <- %08X\n")
                    _T ("R14_ABT := %08X <- %08X\n")
                    _T ("R14_UDEF := %08X <- %08X\n")


                    _T ("SPSR_SVC := %08X <- %08X\n")
                    _T ("SPSR_IRQ := %08X <- %08X\n")
                    _T ("SPSR_USR := %08X <- %08X\n")
                    _T ("SPSR_FIQ := %08X <- %08X\n")
                    _T ("SPSR_ABT := %08X <- %08X\n")
                    _T ("SPSR_UDEF := %08X <- %08X\n\n-");


    CString CStr;
    DWORD dwCurrentCPSR = gba_get_cpsr (m_GBA);
    DWORD dwPreviousCPSR = gba_get_cpsr (m_GBA);

    CStr.Format (s, arm7_current_std_spsr (cur), arm7_current_std_spsr (old), // spsr.
                            dwCurrentCPSR, dwPreviousCPSR, // cpsr.
                            mode_gp[dwCurrentCPSR & 15], mode_gp[dwPreviousCPSR & 15], // mode
                            dwCurrentCPSR >> 7 & 1, dwPreviousCPSR >> 7 & 1, // interrupt-i.
                            dwCurrentCPSR >> 6 & 1, dwPreviousCPSR >> 6 & 1, // interrupt-f.
                            dwCurrentCPSR >> 5 & 1, dwPreviousCPSR >> 5 & 1, // thumb-state 
                            dwCurrentCPSR >> 31 & 1, dwPreviousCPSR >> 31 & 1, // -n
                            dwCurrentCPSR >> 30 & 1, dwPreviousCPSR >> 30 & 1, // -z
                            dwCurrentCPSR >> 29 & 1, dwPreviousCPSR >> 29 & 1, // -c
                            dwCurrentCPSR >> 28 & 1, dwPreviousCPSR >> 28 & 1, // -v
                            // r0- r15 current.
                            cur->regs[0], old->regs[0],
                            cur->regs[1], old->regs[1],
                            cur->regs[2], old->regs[2],
                            cur->regs[3], old->regs[3],
                            cur->regs[4], old->regs[4],
                            cur->regs[5], old->regs[5],
                            cur->regs[6], old->regs[6],
                            cur->regs[7], old->regs[7],
                            cur->regs[8], old->regs[8],
                            cur->regs[9], old->regs[9],
                            cur->regs[10], old->regs[10],
                            cur->regs[11], old->regs[11],
                            cur->regs[12], old->regs[12],
                            cur->regs[13], old->regs[13],
                            cur->regs[14], old->regs[14],
                            cur->regs[15], old->regs[15],
                            arm7_getpc ( cur), 
                            // r8-r12 bank common 
                            cur->r812_t[R812b_EXCEPT_FIQ+0], old->r812_t[R812b_EXCEPT_FIQ+0],
                            cur->r812_t[R812b_EXCEPT_FIQ+1], old->r812_t[R812b_EXCEPT_FIQ+1],
                            cur->r812_t[R812b_EXCEPT_FIQ+2], old->r812_t[R812b_EXCEPT_FIQ+2],
                            cur->r812_t[R812b_EXCEPT_FIQ+3], old->r812_t[R812b_EXCEPT_FIQ+3],
                            cur->r812_t[R812b_EXCEPT_FIQ+4], old->r812_t[R812b_EXCEPT_FIQ+4],
                            // r8-r12 bank fiq 
                            cur->r812_t[R812b_FIQ+0], old->r812_t[R812b_FIQ+0],
                            cur->r812_t[R812b_FIQ+1], old->r812_t[R812b_FIQ+1],
                            cur->r812_t[R812b_FIQ+2], old->r812_t[R812b_FIQ+2],
                            cur->r812_t[R812b_FIQ+3], old->r812_t[R812b_FIQ+3],
                            cur->r812_t[R812b_FIQ+4], old->r812_t[R812b_FIQ+4],

                            // r13 bank.
                            cur->r1314_t[R1314b_MGR], old->r1314_t[R1314b_MGR],
                            cur->r1314_t[R1314b_IRQ], old->r1314_t[R1314b_IRQ],
                            cur->r1314_t[R1314b_SYSUSER], old->r1314_t[R1314b_SYSUSER],
                            cur->r1314_t[R1314b_FIQ], old->r1314_t[R1314b_FIQ],
                            cur->r1314_t[R1314b_ABT], old->r1314_t[R1314b_ABT],
                            cur->r1314_t[R1314b_UDEF], old->r1314_t[R1314b_UDEF],

                            // r14 bank
                            cur->r1314_t[R1314b_MGR+1], old->r1314_t[R1314b_MGR+1],
                            cur->r1314_t[R1314b_IRQ+1], old->r1314_t[R1314b_IRQ+1],
                            cur->r1314_t[R1314b_SYSUSER+1], old->r1314_t[R1314b_SYSUSER+1],
                            cur->r1314_t[R1314b_FIQ+1], old->r1314_t[R1314b_FIQ+1],
                            cur->r1314_t[R1314b_ABT+1], old->r1314_t[R1314b_ABT+1],
                            cur->r1314_t[R1314b_UDEF+1], old->r1314_t[R1314b_UDEF+1],


                            // spsr bank
                            cur->spsr_t[SPSRb_MGR], old->spsr_t[SPSRb_MGR],
                            cur->spsr_t[SPSRb_IRQ], old->spsr_t[SPSRb_IRQ],
                            cur->spsr_t[SPSRb_SYSUSER], old->spsr_t[SPSRb_SYSUSER],
                            cur->spsr_t[R1314b_FIQ], old->spsr_t[R1314b_FIQ],
                            cur->spsr_t[R1314b_ABT], old->spsr_t[R1314b_ABT],
                            cur->spsr_t[R1314b_UDEF], old->spsr_t[R1314b_UDEF] );
      // Set It.
      // m_panel.m_GuiRT_Register.SetSel (0, -1);
      

      m_GuiRT_Register.EnableWindow (FALSE);
      m_GuiRT_Register.SetWindowText (CStr);


      // Cmp update. set font color.
      if (arm7_current_std_spsr (cur) != arm7_current_std_spsr (old))
        SetUpdateColor (0, 8, 8, register_old_tonew_update_clr);
      if (dwCurrentCPSR != dwPreviousCPSR)
        SetUpdateColor (1, 8, 8, register_old_tonew_update_clr);
      if ((dwCurrentCPSR & 15) != (dwPreviousCPSR & 15))
        SetUpdateColor (4, 14, 3, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 7 & 1) != (dwCurrentCPSR >> 7 & 1))
        SetUpdateColor (5, 14, 1, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 6 & 1) != (dwCurrentCPSR >> 6 & 1))
        SetUpdateColor (6, 14, 1, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 5 & 1) != (dwCurrentCPSR >> 5 & 1))
        SetUpdateColor (7, 14, 1, register_old_tonew_update_clr);

      // n, z, c, v 
      if ((dwCurrentCPSR >> 31 & 1) != (dwCurrentCPSR >> 31 & 1))
        SetUpdateColor (8, 14, 1, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 30 & 1) != (dwCurrentCPSR >> 30 & 1))
        SetUpdateColor (9, 14, 1, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 29 & 1) != (dwCurrentCPSR >> 29 & 1))
        SetUpdateColor (10, 14, 1, register_old_tonew_update_clr);
      if ((dwCurrentCPSR >> 28 & 1) != (dwCurrentCPSR >> 28 & 1))
        SetUpdateColor (11, 14, 1, register_old_tonew_update_clr);

      // r0- r15 
      for (DWORD id =0 ; id != 16; id++) 
        if (old->regs[id].blk != cur->regs[id].blk)
          SetUpdateColor (14+id, 6, 8, register_old_tonew_update_clr);
        else ;
      // bank cmp.
      if (cur->r1314_t[R1314b_MGR].blk != old->r1314_t[R1314b_MGR].blk)
        SetUpdateColor (31, 11, 8, register_old_tonew_update_clr);
      if (cur->r1314_t[R1314b_IRQ].blk != old->r1314_t[R1314b_IRQ].blk)
        SetUpdateColor (32, 11, 8, register_old_tonew_update_clr);
      if (cur->r1314_t[R1314b_SYSUSER].blk != old->r1314_t[R1314b_SYSUSER].blk)
        SetUpdateColor (33, 11, 8, register_old_tonew_update_clr);
      if (cur->r1314_t[R1314b_MGR+1].blk != old->r1314_t[R1314b_MGR+1].blk)
        SetUpdateColor (34, 11, 8, register_old_tonew_update_clr);
      if (cur->r1314_t[R1314b_IRQ+1].blk != old->r1314_t[R1314b_IRQ+1].blk)
        SetUpdateColor (35, 11, 8, register_old_tonew_update_clr);
      if (cur->r1314_t[R1314b_SYSUSER+1].blk != old->r1314_t[R1314b_SYSUSER+1].blk)
        SetUpdateColor (36, 11, 8, register_old_tonew_update_clr);
      if (cur->spsr_t[SPSRb_MGR].blk != old->spsr_t[SPSRb_MGR].blk)
        SetUpdateColor (37, 12, 8, register_old_tonew_update_clr);
      if (cur->spsr_t[SPSRb_IRQ].blk != old->spsr_t[SPSRb_IRQ].blk)
        SetUpdateColor (38, 12, 8, register_old_tonew_update_clr);
      if (cur->spsr_t[SPSRb_SYSUSER].blk != old->spsr_t[SPSRb_SYSUSER].blk)
        SetUpdateColor (39, 12, 8, register_old_tonew_update_clr);

      m_GuiRT_Register.EnableWindow (TRUE);
      memcpy (old, cur, sizeof (*cur));
  }

  //  Window - Register Work.
  void OnUpdateRegisterWork (BOOL bForceUpdate = FALSE) {
    if (m_ViewRegisterInfos.bAutoUpdate != FALSE || bForceUpdate) {
      SCROLLINFO scrBarH;
      SCROLLINFO scrBarV;
      
      scrBarH.cbSize = 
      scrBarV.cbSize = sizeof (SCROLLBARINFO);
      scrBarH.fMask =
      scrBarV.fMask = SIF_ALL;

      m_GuiRT_Memory.GetScrollInfo ( SB_HORZ, & scrBarH);
      m_GuiRT_Memory.GetScrollInfo ( SB_VERT, & scrBarV);

      UpdateRegisterOutput ();
      
      m_GuiRT_Memory.SetScrollInfo ( SB_HORZ, & scrBarH, FALSE);
      m_GuiRT_Memory.SetScrollInfo ( SB_VERT, & scrBarV, FALSE);
    }

  }
  // <View-Disasm Work > =======================================================================
  struct ViewDisasmWorkInfos {
   ViewDisasmWorkInfos (struct gba *attachGBA): 
       GBA (attachGBA), 
          iCurrentFormat (0) {      SetCodeRegion (0x8000000); iTopAddressFull = 0x8000000; iMiddleAddressFull = 0x8000000; }
   ~ViewDisasmWorkInfos (void) {}

    BOOL GetCodeRegion (int iAddress) {
      if (iAddress < 0x4000) {
        iAddrBankBase_temp = 0;
        iAddrMaxInRange_temp = 0x4000;
      } else if (iAddress >= 0x2000000 && iAddress < 0x2040000) {
        iAddrBankBase_temp = 0x2000000;
        iAddrMaxInRange_temp = 0x40000;
      } else if (iAddress >= 0x3000000 && iAddress < 0x3008000) {
        iAddrBankBase_temp = 0x3000000;
        iAddrMaxInRange_temp = 0x8000;
      } else if (iAddress >= 0x8000000 && iAddress < 0xA000000) {
        iAddrBankBase_temp = 0x8000000 + 0x800000 * ((iAddress - 0x8000000) /0x800000);;
        iAddrMaxInRange_temp = 0x800000;
      } else {
        return FALSE;
      }
      iAddrFull_temp = iAddress;
      iAddrOffsetInRange_temp = iAddrFull_temp - iAddrBankBase_temp;
      iAddrMaxFull_temp = iAddrBankBase_temp + iAddrMaxInRange_temp;
      return TRUE;
    }

    BOOL SetCodeRegion (int iAddress) {
      if (iAddress < 0x4000) {
        iAddrBankBase = 0;
        iAddrMaxInRange = 0x4000;
      } else if (iAddress >= 0x2000000 && iAddress < 0x2040000) {
        iAddrBankBase = 0x2000000;
        iAddrMaxInRange = 0x40000;
      } else if (iAddress >= 0x3000000 && iAddress < 0x3008000) {
        iAddrBankBase = 0x3000000;
        iAddrMaxInRange = 0x8000;
      } else if (iAddress >= 0x8000000 && iAddress < 0xA000000) {
        iAddrBankBase = 0x8000000 + 0x800000 * ((iAddress - 0x8000000) /0x800000);;
        iAddrMaxInRange = 0x800000;
      } else {
        return FALSE;
      }
      iAddrFull = iAddress;
      iAddrOffsetInRange = iAddrFull - iAddrBankBase;
      iAddrMaxFull = iAddrBankBase + iAddrMaxInRange;
      return TRUE;
    }

   struct gba *GBA;
   int iCurrentFormat; // 0:ARM7 1:THUMB 2:BY ARM7's FLAG 
   int iTopAddressFull;
   int iTopAddressInRange;
   int iMiddleAddressFull;
   int iMiddleAddressInRange;

   int iAddrMaxFull;
   int iAddrFull;
   int iAddrMaxInRange;
   int iAddrOffsetInRange;
   int iAddrBankBase;

   int iAddrMaxFull_temp;
   int iAddrFull_temp;
   int iAddrMaxInRange_temp;
   int iAddrOffsetInRange_temp;
   int iAddrBankBase_temp;
  } m_ViewDisasmInfos;
  

struct OpReg {

DWORD Rd;
DWORD Rn;
DWORD Rm;
DWORD Rs;

__forceinline
void ARM7_GetRegField (DWORD &Opcode, const int RdBitStart = -1, const int RnBitStart = -1,
                                                                                      const int RmBitStart = -1, const int RsBitStart = -1)
{
  if (RdBitStart != -1)
    Rd = Opcode >> RdBitStart & 15;
  if (RnBitStart != -1)
    Rn = Opcode >> RnBitStart & 15;
  if (RmBitStart != -1)
    Rm = Opcode >> RmBitStart & 15;
  if (RsBitStart != -1)
    Rs = Opcode >> RsBitStart & 15;
}

};

struct OpBit {
  BOOL _S6;
  BOOL _S20;
  BOOL _S22;
  BOOL _H;
  BOOL _P;
  BOOL _U;
  BOOL _B;
  BOOL _W;
  BOOL _L;

  void setBitField (DWORD &Opcode) 
  {
  _S6 = Opcode >> 6 & 1;
  _S20 = Opcode >> 20 & 1;
  _S22 = Opcode >> 22 & 1;
  _H = Opcode >> 5 & 1;
  _P = Opcode >> 24 & 1;
  _U = Opcode >> 23 & 1;
  _B = Opcode >> 22 & 1;
  _W = Opcode >> 21 & 1;
  _L = Opcode >> 20 & 1;
  }
};

__forceinline static BOOL TestBit (DWORD mem, const DWORD dwBit) {
  return !!(mem & 1 << dwBit);
}

// FIXME || TODO : The disassembly of Thumb and ARM7 was written at will, and I found many mistakes in my tests. Waiting for refactoring
// 0x1C18_opcode_err
static 
int GetARM7Decode (DWORD address, DWORD Opcode, CString *CStr) {

    static LPCTSTR pCondLink[16] = { _T ("EQ"), _T ("NE"), _T ("CS"), _T ("CC"),
                                         _T ("MI"), _T ("PL"), _T ("VS"), _T ("VC"),
                                         _T ("HI"), _T ("LS"), _T ("GE"), _T ("LT"),
                                         _T ("GT"), _T ("LE"), _T (""), _T ("NV") };
    static LPCTSTR pAluLink[16] = { _T ("AND"), _T ("EOR"), _T ("SUB"), _T ("RSB"),
                                         _T ("ADD"), _T ("ADC"), _T ("SBC"), _T ("RSC"),
                                         _T ("TST"), _T ("TEQ"), _T ("CMP"), _T ("CMN"),
                                         _T ("ORR"), _T ("MOV"), _T ("BIC"), _T ("MVN") };
    static LPCTSTR pSftLink[4] = { _T ("LSL"), _T ("LSR"), _T ("ASR"), _T ("ROR")};
    static LPCTSTR pRegLink[16] = { _T ("R0"), _T ("R1"), _T ("R2"), _T ("R3"),
                                         _T ("R4"), _T ("R5"), _T ("R6"), _T ("R7"),
                                         _T ("R8"), _T ("R9"), _T ("R10"), _T ("R11"),
                                         _T ("R12"), _T ("SP"), _T ("LR"), _T ("PC") };

    OpReg OPR;
    OpBit OPB;

    DWORD dwAddr;
    //DWORD dwImm12;
    DWORD dwImm8;
    DWORD dwImm5;
    DWORD dwShift;
    DWORD dwAluID;
    DWORD dwMainField = Opcode >> 24 & 15;
    DWORD dwAuixField = Opcode >> 4 & 15;

#define ARM7_REGBIT(BitStart)\
    Opcode >> (BitStart) & 15
#define ARM7_REGBIT_STRING(BitStart)\
    pRegLink[Opcode >> (BitStart) & 15]
#define APPEND_STRING CStr->Append
#define APPEND_FORMAT CStr->AppendFormat
#define ARM_COND_FIELD_MASK 0xF0000000
#define ARM_CODE_FIELD_AL       0xE0000000
#define ARM_COND_FIELD_BIT_LSB 28
#define ARM_ALU_START_BIT 21
#define GET_COND() pCondLink[Opcode >> ARM_COND_FIELD_BIT_LSB & 15]

    switch (dwMainField) {
    case 0x0:
    case 0x1:
      // MUL || SWP Check.
      if (dwAuixField == 0x09) {
        if (dwMainField == 0) {
  #define MUL_LONG_FLAG_BIT 23
  #define MUL_SIGN_FLAG_BIT 22
  #define MUL_ADD_FLAG_BIT 21
  #define MUL_NZCV_FLAG_BIT 20
          if (TestBit (Opcode, MUL_LONG_FLAG_BIT)) {
            // long mul
            // OPR.ARM7_GetRegField (12, 16, 
            ALONE_IF_ELSE_ASSERT (TestBit (Opcode, MUL_SIGN_FLAG_BIT), APPEND_STRING (_T ("S")), APPEND_STRING (_T ("U")));
            ALONE_IF_ELSE_ASSERT (TestBit(Opcode, MUL_ADD_FLAG_BIT), APPEND_STRING (_T ("MLAL")), APPEND_STRING (_T ("MULL")));
            APPEND_STRING (GET_COND ());
            ALONE_IF_ASSERT (TestBit(Opcode, MUL_NZCV_FLAG_BIT), APPEND_STRING (_T ("S")));
            APPEND_FORMAT (_T (" %s, %s, %s, %s"), 
                                                  ARM7_REGBIT_STRING (12), ARM7_REGBIT_STRING (16),
                                                  ARM7_REGBIT_STRING (0), ARM7_REGBIT_STRING(8));
          } else {
            ALONE_IF_ELSE_ASSERT ((Opcode & 1 << MUL_ADD_FLAG_BIT), APPEND_STRING (_T ("MLA")), APPEND_STRING (_T ("MUL")));
            APPEND_STRING (GET_COND ());
            ALONE_IF_ASSERT ((Opcode & 1 << MUL_NZCV_FLAG_BIT), APPEND_STRING (_T ("S")));

            // Check Remain Bit.
            if (Opcode & 3 << 22) {
              APPEND_STRING( _T ("-----------------"));
              return -1;
            }
            if (Opcode & 1 << MUL_ADD_FLAG_BIT)
                 APPEND_FORMAT (_T (" %s, %s, %s, %s"), 
                                                  ARM7_REGBIT_STRING (16), ARM7_REGBIT_STRING (0),
                                                  ARM7_REGBIT_STRING (8), ARM7_REGBIT_STRING(12));
            else 
                 APPEND_FORMAT (_T (" %s, %s, %s"), 
                                                  ARM7_REGBIT_STRING (16), ARM7_REGBIT_STRING (0),
                                                  ARM7_REGBIT_STRING (8));     
          }
        } else {
#define MUL_SWP_BYTE_FLAG_BIT 22
            OPR.ARM7_GetRegField (Opcode, 12, 16, 0);
            if (TestBit (Opcode, MUL_SWP_BYTE_FLAG_BIT))
            APPEND_FORMAT (_T ("SWPB%s R%d, R%d, [R%d]"), 
                                                 GET_COND (),
                                                     OPR.Rd, OPR.Rm, OPR.Rn); 
            else 
            APPEND_FORMAT (_T ("SWP%s R%d, R%d, [R%d]"), 
                                                 GET_COND (),
                                                     OPR.Rd, OPR.Rm, OPR.Rn); 
        }
        return 0;
      } else if ((Opcode & 0xFFFFFF0) == 0x01200010) {
        APPEND_FORMAT (_T ("BX%s R%d"), GET_COND (), ARM7_REGBIT (0));
      } else if ((Opcode & 0xFBF0FFF) == 0x010F0000) {
#define PSR_R_BIT 22
#define GET_PSR() TestBit (Opcode, PSR_R_BIT) ? _T ("SPSR") : _T ("CPSR")
        APPEND_FORMAT (_T ("MRS%s R%d, %s"), GET_COND (), ARM7_REGBIT (12), GET_PSR ());
      } else if ((Opcode & 0xFB0FFF0) == 0x0120F000) {
        APPEND_FORMAT (_T ("MSR%s %s_"), GET_COND (), GET_PSR ());
        if (TestBit (Opcode, 16)) APPEND_STRING (_T ("c"));
        if (TestBit (Opcode, 17)) APPEND_STRING (_T ("x"));
        if (TestBit (Opcode, 18)) APPEND_STRING (_T ("s"));
        if (TestBit (Opcode, 19)) APPEND_STRING (_T ("f"));
                                  APPEND_FORMAT (_T (", R%d"), ARM7_REGBIT (0));
      } else if ((Opcode & 0xE400F90) == 0x90 && (Opcode & 6) != 0) {
        // LDR Half Register Offset.
        dwImm8 = Opcode & 15 | (Opcode >> 8 & 15) << 4;
        OPB.setBitField (Opcode);
        OPR.ARM7_GetRegField (Opcode, 12, 16);
        ALONE_IF_ASSERT ( (!OPB._L && !OPB._S6 && OPB._H), APPEND_STRING (_T ("STRH")));
        ALONE_IF_ASSERT ( (OPB._L && !OPB._S6 && OPB._H), APPEND_STRING (_T ("LDRH")));
        ALONE_IF_ASSERT ( (OPB._L && OPB._S6 && !OPB._H) ,APPEND_STRING (_T ("LDRSB")));
        ALONE_IF_ASSERT ( (OPB._L && OPB._S6 && OPB._H) , APPEND_STRING (_T ("LDRSH")));
#define GET_RN_BACK() OPB._W ? _T ("!") : _T ("")
#define GET_PHASE() OPB._U ? _T ("") : _T ("-")
        if (OPB._P != FALSE) 
          APPEND_FORMAT (_T ("%s R%d, [R%d, #%s%d:0x%02X]%s"), GET_COND (), OPR.Rd, OPR.Rn, GET_PHASE (), dwImm8, dwImm8, GET_RN_BACK ());
        else 
          APPEND_FORMAT (_T ("%s R%d, [R%d], #%s%d:0x%02X"), GET_COND (), OPR.Rd, OPR.Rn, GET_PHASE (), dwImm8, dwImm8);
        return 0;
      } else if ((Opcode & 0xE400090) == 0x400090 && (Opcode & 6) != 0) {
        OPB.setBitField (Opcode);
        OPR.ARM7_GetRegField (Opcode, 12, 16, 0);
        ALONE_IF_ASSERT ( (!OPB._L && !OPB._S6 && OPB._H), APPEND_STRING (_T ("STRH")));
        ALONE_IF_ASSERT ( (OPB._L && !OPB._S6 && OPB._H), APPEND_STRING (_T ("LDRH")));
        ALONE_IF_ASSERT ( (OPB._L && OPB._S6 && !OPB._H) ,APPEND_STRING (_T ("LDRSB")));
        ALONE_IF_ASSERT ( (OPB._L && OPB._S6 && OPB._H) , APPEND_STRING (_T ("LDRSH")));

        if (OPB._P != FALSE) 
          APPEND_FORMAT (_T ("%s R%d, [R%d, %sR%d]%s"), GET_COND (), OPR.Rd, OPR.Rn, GET_PHASE (), OPR.Rm, GET_RN_BACK());
        else
          APPEND_FORMAT (_T ("%s R%d, [R%d], %sR%d"), GET_COND (), OPR.Rd, OPR.Rn, GET_PHASE (), OPR.Rm);
        return 0;
      } else {
        // ALU Main...
#define ARM7_ALU_TST 8
#define ARM7_ALU_TEQ 9
#define ARM7_ALU_CMP 10
#define ARM7_ALU_CMN 11
#define ARM7_ALU_MOV 13
#define ARM7_ALU_MVN 15
#define ARM7_FLAG_BIT 20

        dwAluID = Opcode >> ARM_ALU_START_BIT & 15;
        dwShift = Opcode >> 5 & 3;
        OPR.ARM7_GetRegField (Opcode, 12, 16, 0, 8);

        APPEND_FORMAT (_T ("%s%s"), pAluLink[dwAluID], GET_COND ());
        if (Opcode & 1 << ARM7_FLAG_BIT)
          APPEND_STRING (_T ("S"));

        if (dwAluID == ARM7_ALU_MVN || dwAluID == ARM7_ALU_MOV)
          APPEND_FORMAT (_T (" R%d, R%d"), OPR.Rd, OPR.Rm);
        else   if (dwAluID == ARM7_ALU_CMN || dwAluID == ARM7_ALU_CMP
              || dwAluID == ARM7_ALU_TST 
              || dwAluID == ARM7_ALU_TEQ)
          APPEND_FORMAT (_T (" R%d, R%d"), OPR.Rn, OPR.Rm);
        else
          APPEND_FORMAT (_T (" R%d, R%d, R%d"), OPR.Rd, OPR.Rn, OPR.Rm);
        // Make Oprand2..
        if (dwAuixField & 1)
          APPEND_FORMAT (_T (", %s R%d"), pSftLink[dwShift], ARM7_REGBIT (8));
        else {
          dwImm5 = Opcode >> 7 & 31;
          if (dwImm5 != 0 || dwShift != 0)
            if (dwImm5 == 0 && dwShift == 3)
              APPEND_STRING (_T (", RRX"));
            else
              APPEND_FORMAT (_T (", %s #%d"), pSftLink[dwShift], dwImm5);
          else {}
        }
      }
      return 0;
    case 0x2:
    case 0x3:
      if (TRUE) {
        DWORD ImmSft = Opcode & 255;
        DWORD Imm8Sft4 = Opcode >> 8 & 15;
        DWORD dwAluID = Opcode >> ARM_ALU_START_BIT & 15;
        OPR.ARM7_GetRegField (Opcode, 12, 16);
        Imm8Sft4 <<= 1;
        ImmSft = ImmSft >> Imm8Sft4 | ImmSft << (32 - Imm8Sft4);

        if ((Opcode & 0xFB0F000) == 0x320F000) {
          APPEND_FORMAT (_T ("MSR%s %s_"), GET_COND (), GET_PSR ());
          if (TestBit (Opcode, 16)) APPEND_STRING (_T ("c"));
          if (TestBit (Opcode, 17)) APPEND_STRING (_T ("x"));
          if (TestBit (Opcode, 18)) APPEND_STRING (_T ("s"));
          if (TestBit (Opcode, 19)) APPEND_STRING (_T ("f"));
                                    APPEND_FORMAT (_T ("#0x%08X"), ImmSft);
        } else {
          APPEND_FORMAT (_T ("%s%s%s"), pAluLink[dwAluID], GET_COND (), TestBit (Opcode, ARM7_FLAG_BIT) ? _T ("S") : _T (""));
          if (dwAluID == ARM7_ALU_MVN || dwAluID == ARM7_ALU_MOV)
            APPEND_FORMAT (_T (" %s, #0x%08X"), pRegLink[OPR.Rd], ImmSft);
          else   if (dwAluID == ARM7_ALU_CMN || dwAluID == ARM7_ALU_CMP
                || dwAluID == ARM7_ALU_TST 
                || dwAluID == ARM7_ALU_TEQ)
            APPEND_FORMAT (_T (" %s, #0x%08X"), pRegLink[OPR.Rn], ImmSft);
          else
            APPEND_FORMAT (_T (" %s, %s, #0x%08X"), pRegLink[OPR.Rd], pRegLink[OPR.Rn], ImmSft);
        }
      }
      return 0;
    case 0x4:
    case 0x5:
      OPR.ARM7_GetRegField (Opcode, 12, 16);
      OPB.setBitField (Opcode);
      APPEND_STRING (OPB._L ? _T ("LDR") : _T ("STR"));
      ALONE_IF_ASSERT (OPB._B, APPEND_STRING (_T ("B")));
      ALONE_IF_ASSERT ((OPB._P == FALSE && OPB._W), APPEND_STRING (_T ("T")));
      APPEND_STRING (GET_COND ());

      if (OPB._P != FALSE) 
        APPEND_FORMAT (_T (" R%d, [R%d, %s#%d:0x%03X]%s"), OPR.Rd, OPR.Rn, GET_PHASE (), Opcode & 0xFFF, Opcode & 0xFFF, GET_RN_BACK ());
      else 
        APPEND_FORMAT (_T (" R%d, [R%d], %s#%d:0x%03X"), OPR.Rd, OPR.Rn, GET_PHASE (), Opcode & 0xFFF, Opcode & 0xFFF);
      return 0;
    case 0x6:
    case 0x7:
      dwImm5 = Opcode >> 7 & 31;
      dwShift = Opcode >> 5 & 3;
      OPR.ARM7_GetRegField (Opcode, 12, 16, 0);
      OPB.setBitField (Opcode);
      APPEND_FORMAT (OPB._L ? _T ("LDR") : _T ("STR"));
      ALONE_IF_ASSERT (OPB._B, APPEND_STRING (_T ("B")));
      ALONE_IF_ASSERT ((OPB._P == FALSE && OPB._W), APPEND_STRING (_T ("T")));
      APPEND_STRING (GET_COND ());

      if (OPB._P != FALSE) {
        APPEND_FORMAT (_T (" R%d, [R%d, %sR%d"), OPR.Rd, OPR.Rn, GET_PHASE (), OPR.Rm);
        if (dwImm5 != 0 || dwShift != 0)
          if (dwImm5 == 0 && dwShift == 3)
            APPEND_FORMAT (_T (", RRX]%s"), GET_RN_BACK ());
          else
            APPEND_FORMAT (_T (", %s #%d:%02X]%s"), pSftLink[dwShift], dwImm5, dwImm5, GET_RN_BACK ());
        else 
          APPEND_FORMAT (_T ("]%s"), GET_RN_BACK ());
      } else {
        APPEND_FORMAT (_T (" R%d, [R%d], %sR%d"), OPR.Rd, OPR.Rn, GET_PHASE (), OPR.Rm);
        if (dwImm5 != 0 || dwShift != 0)
          if (dwImm5 == 0 && dwShift == 3)
            APPEND_STRING (_T (", RRX"));
          else
            APPEND_FORMAT (_T (", %s #%d:%02X"), pSftLink[dwShift], dwImm5, dwImm5);
        else {}
      }
      return 0;
    case 0x8:
    case 0x9: // LDM, STM.
      OPB.setBitField (Opcode);
      APPEND_STRING (OPB._L ? _T ("LDM") : _T ("STM"));
      APPEND_STRING (OPB._U ? _T ("I") : _T ("D"));
      APPEND_STRING (OPB._P ? _T ("B") : _T ("A"));
      APPEND_FORMAT (_T ("%s [R%d]%s"), GET_COND (), ARM7_REGBIT (16), GET_RN_BACK ());

      for (DWORD id = 0; id != 16; id++)
        if (Opcode & 1 << id)
          APPEND_FORMAT (_T (", R%d"), id);
        else {}

      ALONE_IF_ASSERT (OPB._S22, APPEND_STRING (_T ("^")));
      return 0;
    case 0xA:
    case 0xB:
      dwAddr= Opcode & 0x800000 - 1;
      if (Opcode & 0x800000)
        dwAddr |= 0xFF000000;
      dwAddr = address + (dwAddr << 2) & -4;
      if (dwMainField == 10)
        APPEND_FORMAT (_T ("B%s #0x%08X"), GET_COND (), dwAddr);
      else 
        APPEND_FORMAT (_T ("BL%s #0x%08X"), GET_COND (), dwAddr);
      return 0;
    case 0xC:
    case 0xD:
    case 0xE: // float !.
      APPEND_STRING( _T ("-----------------"));
      return -1;
    case 0xF:
      APPEND_FORMAT (_T ("SWI%s #0x%06X"), GET_COND (), Opcode & 0xFFFFFF);
      return 0;
    default:
      ATLASSERT (FALSE);
    }
    return 0;
}

static
int GetThumbDecode (DWORD address, WORD Opcode, CString *CStr) {

    static LPCTSTR pCondLink[16] = { _T ("EQ"), _T ("NE"), _T ("CS"), _T ("CC"),
                                         _T ("MI"), _T ("PL"), _T ("VS"), _T ("VC"),
                                         _T ("HI"), _T ("LS"), _T ("GE"), _T ("LT"),
                                         _T ("GT"), _T ("LE"), _T (""), _T ("NV") };
    static LPCTSTR pAluLink[16] = { _T ("AND"), _T ("EOR"), _T ("LSL"), _T ("LSR"),
                                         _T ("ASR"), _T ("ADC"), _T ("SBC"), _T ("ROR"),
                                         _T ("TST"), _T ("NEG"), _T ("CMP"), _T ("CMN"),
                                         _T ("ORR"), _T ("MUL"), _T ("BIC"), _T ("MVN") };
    static LPCTSTR pSftLink[4] = { _T ("LSL"), _T ("LSR"), _T ("ASR"), _T ("ROR")};
    static LPCTSTR pI8CCLink[4] =  { _T ("MOV"), _T ("CMP"), _T ("ADD"), _T ("SUB")};
    static LPCTSTR pR16CCLink[4] =  { _T ("ADD"), _T ("CMP"), _T ("MOV"), _T ("BX")};

    DWORD dwAddr;
   // DWORD dwImm12;
    //DWORD dwImm8;
    //DWORD dwImm5;
    //DWORD dwShift;
    //DWORD dwAluID;
    DWORD dwMainField = Opcode >> 12 & 15;
    DWORD dwAuixField = Opcode >> 8 & 15;

#define THUMB_REGBIT(BitStart)\
    Opcode >> BitStart & 7
#define OP_AND(Mask)\
    (Opcode & (Mask))
#define OP_BIT(Bit)\
    (Opcode & 1 << Bit)
#define OP_SFT_AND(Sft, Mask)\
    Opcode >> (Sft) & (Mask)
#define OP_GET_SFT_LINK(BitIndex)\
    pSftLink[BitIndex]

    DWORD dwSubOP;
    switch (dwMainField) {
    case 0x0:
    case 0x1:
      if ((Opcode & 0x1F00) != 0x1800)
        APPEND_FORMAT (_T ("%s R%d, R%d, #%d:%02X"), pSftLink [OP_SFT_AND(11, 3)], THUMB_REGBIT (0), THUMB_REGBIT (3), OP_SFT_AND (6, 31), OP_SFT_AND (6, 31));
      else {
        dwSubOP = OP_SFT_AND (8, 7)  >> 1;
        APPEND_STRING ((dwSubOP & 1) ? _T ("SUB") : _T ("ADD"));
        APPEND_FORMAT (_T (", R%d, R%d, %s%d"), THUMB_REGBIT (0),   THUMB_REGBIT (3),  (dwSubOP & 2) ? _T ("#") : _T ("R"), THUMB_REGBIT (6));
      }
      return 0;
    case 0x2:
    case 0x3:
      dwSubOP = (Opcode >> 8 & 0x1F) >> 3;
      APPEND_FORMAT (_T ("%s, R%d, #%d:%02X"), pI8CCLink[dwSubOP], THUMB_REGBIT(8), Opcode & 255, Opcode & 255);
      return 0;
    case 0x4:
      if (dwAuixField <= 3) {
        dwSubOP = OP_SFT_AND (6, 15);
        APPEND_FORMAT (_T ("%s, R%d, R%d"), pAluLink[dwSubOP], THUMB_REGBIT(0), THUMB_REGBIT(3));
      } else if (dwAuixField <= 7) {
        dwSubOP = dwAuixField - 4;
        DWORD dwRmR16Bit = OP_SFT_AND (3, 15);
        DWORD dwRd_Rn_R16Bit = Opcode & 7 | (Opcode >> 7 & 1) << 3;
        if (dwSubOP != 3)
          APPEND_FORMAT (_T ("%s, R%d, R%d"), pR16CCLink[dwSubOP], dwRd_Rn_R16Bit, dwRmR16Bit);
        else 
          APPEND_FORMAT (_T ("BX, R%d"), dwRmR16Bit);
      } else {
        DWORD dwImm8 = (Opcode & 255) << 2;
        APPEND_FORMAT (_T ("LDR R%d, [#0x%08X]"), THUMB_REGBIT(8), (address & -4) + dwImm8);
      }
      return 0;
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8: {
      dwSubOP = dwAuixField >> 1;
      DWORD dwRd = THUMB_REGBIT (0);
      DWORD dwRn = THUMB_REGBIT (3);
      DWORD dwImm5 = Opcode >> 6 & 31;
      DWORD dwRm = Opcode >> 6 & 7;
#undef GET_B_BIT
#define GET_B_BIT() (Opcode & 0x1000)
#undef GET_L_BIT
#define GET_L_BIT() (Opcode & 0x800)
      if ((Opcode & 0xE000) == 0x6000)
        APPEND_FORMAT (_T ("%s%s R%d, [R%d, #%d:%03X]"), GET_L_BIT () ? _T ("LDR") : _T ("STR"), GET_B_BIT () ? _T ("B") : _T (""), dwRd, dwRn, GET_B_BIT () ? dwImm5 : dwImm5 * 4, GET_B_BIT () ? dwImm5 : dwImm5 * 4);
      else if ((Opcode & 0xF000) == 0x8000)
        APPEND_FORMAT (_T ("%s R%d, [R%d, #%d:%02X]"), GET_L_BIT () ? _T ("LDRH") : _T ("STRH"), dwRd, dwRn, dwImm5 *2, dwImm5 *2);
      else if ((Opcode & 0xF200) == 0x5000)
        APPEND_FORMAT (_T ("%s%s R%d, [R%d, R%d]"), GET_L_BIT () ? _T ("LDR") : _T ("STR"), GET_B_BIT () ? _T ("B") : _T (""), dwRd, dwRn, dwRm);
      else if ((Opcode & 0xF200) == 0x5200) {
        static LPCTSTR pMACLink[] = { _T ("STRH"), _T ("LDRSB"), _T ("LDRH"), _T ("LDRSH") };
        APPEND_FORMAT (_T ("%s R%d, [R%d, R%d]"), pMACLink[Opcode >> 10 & 3], dwRd, dwRn, dwRm);
      } else {
        APPEND_FORMAT ( _T ("Undef Instruction"));
        return -1;
      } }
      return 0;
    case 0x9:
      APPEND_FORMAT (_T ("%s R%d, [SP, #%d:0x%03X]"), (Opcode & 0x800) ? _T ("LDR") : _T ("STR"), THUMB_REGBIT (8), (Opcode & 0xFF) * 4, (Opcode & 0xFF) * 4);
      return 0;
    case 0xA:
      if (dwAuixField & 8)
        APPEND_FORMAT (_T ("%ADD R%d, SP, #%d:0x%03X"), THUMB_REGBIT (8), (Opcode & 0xFF) * 4, (Opcode & 0xFF) * 4);
      else 
        // APPEND_FORMAT (_T ("%ADD R%d, PC, #0x%03X"), THUMB_REGBIT (8), (Opcode & 0xFF) * 4 + address);
        // APPEND_FORMAT (_T ("MOV R%d, #0x%08X"), THUMB_REGBIT (8), (Opcode & 0xFF) * 4 + (address & -4));
      APPEND_FORMAT (_T ("ADR R%d, #0x%08X"), THUMB_REGBIT (8), (Opcode & 0xFF) * 4 + (address & -4));
      return 0;
    case 0xB:
      if (dwAuixField == 0)
        if (Opcode & 0x80)
          APPEND_FORMAT (_T ("SUB SP, #%d:%03X"), (Opcode & 0x7F) * 4, (Opcode & 0x7F) * 4);
        else 
          APPEND_FORMAT (_T ("ADD SP, #%d:%03X"), (Opcode & 0x7F) * 4, (Opcode & 0x7F) * 4);
      else if (dwAuixField == 4 || (dwAuixField == 5)) {
        APPEND_STRING (_T ("PUSH"));
        for (DWORD id = 0; id != 9; id++)
          if (Opcode & 1 << id)
            if (id != 8)
              APPEND_FORMAT (_T (" R%d"), id);
            else 
              APPEND_STRING (_T (" LR"));
          else {}
      } else if (dwAuixField == 4 + 8 || (dwAuixField == 5 + 8)) {
        APPEND_STRING (_T ("POP"));
        for (DWORD id = 0; id != 9; id++)
          if (Opcode & 1 << id)
            if (id != 8)
              APPEND_FORMAT (_T (" R%d"), id);
            else 
              APPEND_STRING (_T (" PC")); // TODO: ,
          else {}
      } else {
        APPEND_FORMAT ( _T ("Undef Instruction"));
        return -1;
      }
      break;
    case 0xC:
      APPEND_FORMAT (_T ("%s [R%d]"), (dwAuixField & 8) ? _T ("LDMIA") : _T ("STMIA"), THUMB_REGBIT (8));
      for (DWORD id = 0; id != 8; id++)
        if (Opcode & 1 << id)
          APPEND_FORMAT (_T (", R%d"), id);
        else  {}
      return 0;
    case 0xD:
      if (dwAuixField != 15) 
        APPEND_FORMAT (_T ("B%s #%08X"), pCondLink[dwAuixField], address + 2 * (CHAR)(Opcode & 255));
      else 
        APPEND_FORMAT (_T ("SWI #%d:%02X"), Opcode & 255, Opcode & 255);
      return 0;
    case 0xE:
      dwAddr = Opcode & 0x7FF;
      if (dwAddr & 0x400)
        dwAddr |= ~(0x400 - 1);
      if (dwAuixField < 8)
        APPEND_FORMAT (_T ("B #%08X"), address + 2 * dwAddr);
      else {
        APPEND_FORMAT ( _T ("Undef Instruction"));
        return -1;
      }
      return 0;
    case 0xF:
      // BL_10 always comes first, doesn't it?
      static DWORD LR_base = 0;
      if (dwAuixField < 8) {
        LR_base = Opcode & 0x7FF;
        if (LR_base & 0x400)
          LR_base |= ~(0x400 - 1);
        APPEND_STRING (_T ("BL_H10"));
        LR_base = address + (LR_base << 12);
      } else  {
        APPEND_FORMAT (_T ("BL_H11 #%08X"), ((Opcode & 0x7FF) << 1) + LR_base);
      }
      return 0;
    }
    return 0;
}
  int GetDisasmFormat (int iFormat = -1, BOOL bUpdateDisasmFormat = FALSE) {
    if (iFormat < 0)
      iFormat = m_ViewDisasmInfos.iCurrentFormat;
    if (iFormat == 2)
      if (arm7_thumbmode (& m_ViewDisasmInfos.GBA->arm7) != false)
        iFormat = 1;
      else 
        iFormat = 0;
    else {}
    if (bUpdateDisasmFormat != FALSE)
      m_ViewDisasmInfos.iCurrentFormat = iFormat;
    return iFormat;
  }

  void UpdateSpecAddress (DWORD dwAddressMid, int  dwCurrentFormat = -1, BOOL bUpdateDisasmFormat = FALSE) {
    DWORD dwDisasmFormat = GetDisasmFormat (dwCurrentFormat, bUpdateDisasmFormat);
    DWORD dwShowMiddleAddress;
    DWORD dwShowTopAddress;

    if (m_AttachFrame->m_bLoadRom == FALSE) {
      m_GuiRT_Disasm.SetWindowText (_T ("Oops! Nothing to do in disasm window"));
      return ; // nothing to do.
    }

    int iElem = GetiElemSize (m_GuiRT_Disasm);
    if (iElem < 0) iElem = 0;
    int iHalfOffset = iElem / 2  * (dwDisasmFormat ? 2 : 4);

    DWORD dwEIP = arm7_getpc (& m_ViewDisasmInfos.GBA->arm7);
    dwAddressMid &= dwDisasmFormat ? -2 : -4;

    // Limit address .
    m_ViewDisasmInfos.GetCodeRegion (dwAddressMid);
    if ( ((int) dwAddressMid) - iHalfOffset < m_ViewDisasmInfos.iAddrBankBase_temp)
      dwShowMiddleAddress = m_ViewDisasmInfos.iAddrBankBase_temp + iHalfOffset;
    else if (dwAddressMid + iHalfOffset >= (m_ViewDisasmInfos.iAddrMaxFull_temp))
      dwShowMiddleAddress = m_ViewDisasmInfos.iAddrMaxFull_temp - iHalfOffset;
    else 
      dwShowMiddleAddress = dwAddressMid;

    dwShowTopAddress = dwShowMiddleAddress - iHalfOffset;
    m_ViewDisasmInfos.iTopAddressFull = dwShowTopAddress;
    m_ViewDisasmInfos.iMiddleAddressFull = dwShowMiddleAddress;
    m_ViewDisasmInfos.SetCodeRegion (dwShowTopAddress);
    m_ViewDisasmInfos.iTopAddressInRange = dwShowTopAddress- m_ViewDisasmInfos.iAddrBankBase;
    m_ViewDisasmInfos.iMiddleAddressInRange = dwShowMiddleAddress - m_ViewDisasmInfos.iAddrBankBase;

    // Show disasm text..
    int line = -1;
    CString CStr;
    for (DWORD id=0; id != iElem; id++) {
      if (dwShowTopAddress == dwEIP)
        line = id;
      CStr.AppendFormat (_T ("%07X: "), dwShowTopAddress);
      if (dwDisasmFormat == 0) {
        DWORD code = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress);
        ((WORD *)& code)[1] = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress +2);
        GetARM7Decode (dwShowTopAddress+ 8, code, & CStr);
        dwShowTopAddress += 4;
      } else {
        WORD code = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress);
        GetThumbDecode (dwShowTopAddress+ 4, code, & CStr);
        dwShowTopAddress += 2;
      }
      if (id != iElem - 1)
        CStr.Append (_T ("\n"));
    }
    dwShowTopAddress = m_ViewDisasmInfos.iTopAddressFull;
    m_GuiRT_Disasm.SetWindowText (CStr);

    // Set scroll infos.
    SCROLLINFO  scrBarSettings;
    ZeroMemory (& scrBarSettings, sizeof (scrBarSettings));

    scrBarSettings.cbSize = sizeof (scrBarSettings);
    scrBarSettings.fMask = SIF_ALL;
    scrBarSettings.nMax = m_ViewDisasmInfos.iAddrMaxInRange >> (dwDisasmFormat ? 1 : 2);
    scrBarSettings.nPage = iElem;
    scrBarSettings.nPos = (dwShowTopAddress - m_ViewDisasmInfos.iAddrBankBase) / (dwDisasmFormat ? 2 : 4);

    // Shadow breakpoint item.
    for (DWORD id=0; id != iElem; id++) {
      if (gba_get_addr_breakpoint_slot (m_GBA, dwShowTopAddress) != null)
        SetUpdateBKColor (id, disasm_breakpoint_bg_clr);
      dwShowTopAddress += dwDisasmFormat ?   2 : 4;
    }
    if (line >= 0)
      SetUpdateBKColor (line, disasm_pchit_bg_clr);

    m_GuiRT_Disasm.SetScrollInfo (SB_VERT, & scrBarSettings, TRUE);
  }

  void UpdateFollowPC (DWORD dwAddressMidOld, BOOL bUpdateDisasmFormat = FALSE) {
    DWORD dwDisasmFormat = GetDisasmFormat (2, bUpdateDisasmFormat);
    DWORD dwCurrentMidSpec_ = 0;
    DWORD dwOldMidSpec_= 0;
    DWORD dwShowMiddleAddress;
    DWORD dwShowTopAddress;

    if (m_AttachFrame->m_bLoadRom == FALSE) {
      m_GuiRT_Disasm.SetWindowText (_T ("Oops! Nothing to do in Disasm window"));
      return ; // nothing to do.
    }

    int iElem = GetiElemSize (m_GuiRT_Disasm);
    if (iElem < 0) iElem = 0;
    int iHalfOffset = iElem / 2  * (dwDisasmFormat ? 2 : 4);

    DWORD dwEIP = arm7_getpc (& m_ViewDisasmInfos.GBA->arm7);
    DWORD dwAddressMid = dwEIP & (dwDisasmFormat ? -2 : -4);

    // Limit old address .
    m_ViewDisasmInfos.GetCodeRegion (dwAddressMidOld);
    if ( ((int) dwAddressMidOld) - iHalfOffset < m_ViewDisasmInfos.iAddrBankBase_temp)
      dwOldMidSpec_ = m_ViewDisasmInfos.iAddrBankBase_temp + iHalfOffset;
    else if (dwAddressMidOld + iHalfOffset >= (m_ViewDisasmInfos.iAddrMaxFull_temp))
      dwOldMidSpec_ = m_ViewDisasmInfos.iAddrMaxFull_temp - iHalfOffset;
    else 
      dwOldMidSpec_ = dwAddressMidOld;

    m_ViewDisasmInfos.GetCodeRegion (dwEIP);
    if ( ((int) dwAddressMid) - iHalfOffset < m_ViewDisasmInfos.iAddrBankBase_temp)
      dwCurrentMidSpec_ = m_ViewDisasmInfos.iAddrBankBase_temp + iHalfOffset;
    else if (dwAddressMid + iHalfOffset >= (m_ViewDisasmInfos.iAddrMaxFull_temp))
      dwCurrentMidSpec_ = m_ViewDisasmInfos.iAddrMaxFull_temp - iHalfOffset;
    else 
      dwCurrentMidSpec_ = dwAddressMid;

    dwOldMidSpec_ &= dwDisasmFormat ? -2 : -4;
    dwCurrentMidSpec_ &= dwDisasmFormat ? -2 : -4;
    if (((int)dwCurrentMidSpec_ >= ((int) dwOldMidSpec_) - iHalfOffset
    && (int)dwCurrentMidSpec_ < ((int) dwOldMidSpec_) + iHalfOffset)) 
      // PC in range. done normal.
      dwShowMiddleAddress = dwOldMidSpec_;
    else 
      dwShowMiddleAddress = dwCurrentMidSpec_;

    dwShowTopAddress = dwShowMiddleAddress - iHalfOffset;
    m_ViewDisasmInfos.iTopAddressFull = dwShowTopAddress;
    m_ViewDisasmInfos.iMiddleAddressFull = dwShowMiddleAddress;
    m_ViewDisasmInfos.SetCodeRegion (dwShowTopAddress);
    m_ViewDisasmInfos.iTopAddressInRange = dwShowTopAddress- m_ViewDisasmInfos.iAddrBankBase;
    m_ViewDisasmInfos.iMiddleAddressInRange = dwShowMiddleAddress - m_ViewDisasmInfos.iAddrBankBase;

    // Show disasm text..
    int line = -1;
    CString CStr;
    for (DWORD id=0; id != iElem; id++) {
      if (dwShowTopAddress == dwEIP)
        line = id;
      CStr.AppendFormat (_T ("%07X: "), dwShowTopAddress);
      if (dwDisasmFormat == 0) {
        DWORD code = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress);
        ((WORD *)& code)[1] = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress +2);
        GetARM7Decode (dwShowTopAddress+ 8, code, & CStr);
        dwShowTopAddress += 4;
      } else {
        WORD code = gba_fast_rhw (m_ViewDisasmInfos.GBA, dwShowTopAddress);
        GetThumbDecode (dwShowTopAddress+ 4, code, & CStr);
        dwShowTopAddress += 2;
      }
      if (id != iElem - 1)
        CStr.Append (_T ("\n"));
    }
    dwShowTopAddress = m_ViewDisasmInfos.iTopAddressFull;
    m_GuiRT_Disasm.SetWindowText (CStr);

    // Set scroll infos.
    SCROLLINFO  scrBarSettings;
    ZeroMemory (& scrBarSettings, sizeof (scrBarSettings));

    scrBarSettings.cbSize = sizeof (scrBarSettings);
    scrBarSettings.fMask = SIF_ALL;
    scrBarSettings.nMax = m_ViewDisasmInfos.iAddrMaxInRange >> (dwDisasmFormat ? 1 : 2);
    scrBarSettings.nPage = iElem;
    scrBarSettings.nPos = (dwShowTopAddress - m_ViewDisasmInfos.iAddrBankBase) / (dwDisasmFormat ? 2 : 4);

    // Shadow breakpoint item.
    for (DWORD id=0; id != iElem; id++) {
      if (gba_get_addr_breakpoint_slot (m_GBA, dwShowTopAddress) != null)
        SetUpdateBKColor (id, disasm_breakpoint_bg_clr);
      dwShowTopAddress += dwDisasmFormat ?   2 : 4;
    }
    if (line >= 0)
      SetUpdateBKColor (line, disasm_pchit_bg_clr);

    m_GuiRT_Disasm.SetScrollInfo (SB_VERT, & scrBarSettings, TRUE);
  }

  LRESULT OnHScroll_ViewDisasm (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    
    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    if (dwMainSBCode == SB_THUMBTRACK || dwMainSBCode == SB_THUMBPOSITION) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Disasm.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Disasm.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
      m_GuiRT_Disasm.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
    } else if (dwMainSBCode == SB_ENDSCROLL) {
      SCROLLINFO scrBar;
      scrBar.cbSize = sizeof (SCROLLBARINFO);
      scrBar.fMask = SIF_ALL;
      m_GuiRT_Disasm.GetScrollInfo ( SB_HORZ, & scrBar);
      // CancelSomeInputStatus (m_GuiRT_Memory);
      scrBar.nPos = scrBar.nTrackPos;
      scrBar.fMask = SIF_ALL;
      CancelSomeInputStatus (m_GuiRT_Disasm);
      m_GuiRT_Disasm.SetScrollInfo ( SB_HORZ, & scrBar, FALSE);
      m_GuiRT_Disasm.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    }
    return 0;
  }
  LRESULT OnSize_ViewDisasm (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    // bHandled = FALSE;
    m_GuiRT_Disasm.DefWindowProc (pMsg->msg, pMsg->wparam, pMsg->lparam);
    UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull);
    return 0;
  }

  LRESULT OnVScroll_ViewDisasm (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    int iElem = GetiElemSize (m_GuiRT_Disasm);
    if (iElem <= 0)
      iElem = 0;

    DWORD dwMainSBCode = LOWORD (pMsg->wparam);
    DWORD dwShowPos;
    DWORD dwDisasmFormat = GetDisasmFormat ();
    DWORD vec = dwDisasmFormat ? 2 : 4;

    SCROLLINFO scrBar;
    scrBar.cbSize = sizeof (SCROLLBARINFO);
    scrBar.fMask = SIF_ALL;
    m_GuiRT_Disasm.GetScrollInfo ( SB_VERT, & scrBar);

    dwShowPos = scrBar.nTrackPos;
    switch (dwMainSBCode) {
    case SB_ENDSCROLL: 
      dwShowPos = scrBar.nPos;
      break;

    case  SB_LINEUP:
      dwShowPos = scrBar.nPos;

      if (dwShowPos != 0) 
        dwShowPos--;
      break;

    case  SB_LINEDOWN: 
      dwShowPos = scrBar.nPos;
      if (scrBar.nPos + scrBar.nPage < scrBar.nMax)
        dwShowPos++;
      break;
    case  SB_PAGEUP:    
      dwShowPos = scrBar.nPos;

      if (dwShowPos >= iElem) 
        dwShowPos-= iElem;
      break;
    case SB_PAGEDOWN:        
      dwShowPos = scrBar.nPos + iElem;
      if (dwShowPos >= scrBar.nMax - scrBar.nPage)
        dwShowPos = scrBar.nMax - scrBar.nPage;
    case  SB_TOP:      
    case  SB_BOTTOM:   
    default:
      break;
    }
    // Show text.
    UpdateSpecAddress (m_ViewDisasmInfos.iAddrBankBase +  (dwShowPos+iElem/2) *vec, dwDisasmFormat);
    return 0;
  }

  BOOL GetCurrentLineEIP (DWORD *pEIP) {
    DWORD dwDisasmFormat = GetDisasmFormat (  m_ViewDisasmInfos.iCurrentFormat);
    CRect rc;
    CHARRANGE chrange;
    m_GuiRT_Disasm.GetSel (chrange);
    int iElem;
    int iRow = SendMessage (m_GuiRT_Disasm, EM_LINEFROMCHAR, chrange.cpMin, 0);
    GetClientRect (rc);
    iElem = rc.Height () / nes8bitfont_size;
    if (iElem <= 0 || iRow < 0 || !m_ViewDisasmInfos.GBA)
      return FALSE;
    else {}
    if (pEIP != NULL)
      if (dwDisasmFormat == 0)
      *pEIP = (m_ViewDisasmInfos.iTopAddressFull & -4) + iRow * 4;
      else 
      *pEIP = (m_ViewDisasmInfos.iTopAddressFull & -2) + iRow * 2;
    else {}
    return TRUE;
  }

  LRESULT OnKeyUp_ViewDisasm (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    DWORD temp;
    switch (pMsg->wparam) {
    case VK_F8: // Fast addr breakpoint remove or append.
      CHARRANGE cr;
      m_GuiRT_Disasm.GetSel (cr);
      if (GetCurrentLineEIP (& temp) != FALSE) {
        struct breakpoint *devbp = gba_get_addr_breakpoint_slot (m_GBA, temp);
        if (devbp != null) {
          gba_breakpoint_remove (m_GBA, devbp);
          gba_set_addr_breakpoint_slot (m_GBA, temp, null);
        } else {
          // Add Breakpoint.
          devbp = gba_breakpoint_create (m_GBA, true, BREAKPOINT_REASON_R15,
            true, temp, 0, null, 0, 0, null,  0, null, 0);
          ATLASSERT (devbp != null);
          gba_set_addr_breakpoint_slot (m_GBA, temp, devbp);
        }
      }
      m_ViewBreakpoint.m_panel_arm7.enumShowCurrentBreakPoint ();
      UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull);
      m_GuiRT_Disasm.SetSel (cr);
      break;
    case VK_F9: // Step Curosr.
      if (GetCurrentLineEIP (& temp) != FALSE) {
        gba_cursor_bp (m_ViewDisasmInfos.GBA, temp);
        SetDebuggerInterrupt (FALSE);
      }
      break;
    case VK_F11: //  : Step In
      gba_stepin (m_ViewDisasmInfos.GBA);
      SetDebuggerInterrupt (FALSE);
      break;
    case VK_F12: //  : Step Out
      gba_stepout (m_ViewDisasmInfos.GBA);
      SetDebuggerInterrupt (FALSE);
      break;
    default:
      return 0;
    }
    return 0;
  }

  LRESULT OnSysKeyUp_ViewDisasm (CDnMsg<CDebugger>* pMsg, BOOL& bHandled) {
    switch (pMsg->wparam) {
    case VK_F10: // Step Over
      gba_stepover (m_ViewDisasmInfos.GBA);
      SetDebuggerInterrupt (FALSE);
      break;
    case '1': //  : BIOS
      UpdateSpecAddress (0x0000000);
      break;
    case '2': // : ERAM
      UpdateSpecAddress (0x2000000);
      break;
    case '3': // : IRAM
      UpdateSpecAddress (0x3000000);
      break;
    case '4': // : PROM
      UpdateSpecAddress (0x8000000);
      break;
    case '5': // : PROM
      UpdateSpecAddress (0x8800000);
      break;
    case '6': // : PROM
      UpdateSpecAddress (0x9000000);
      break;
    case '7': // : PROM
      UpdateSpecAddress (0x9800000);
      break;
    case 'A':
    case 'a':
      UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull, 0, TRUE);
      break;
    case 'T':
    case 't':
      UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull, 1, TRUE);
      break;
    case 'q':
    case 'Q':
      UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull, 2, TRUE);
      break;
    case 'u':
    case 'U':
      UpdateSpecAddress (m_ViewDisasmInfos.iMiddleAddressFull);
    default:
      break;
    }
    return 0;
  }

  void SetUpdateBKColor (int line, DWORD color) {
    int n = m_GuiRT_Disasm.LineIndex (line);
    CHARRANGE ch_range;
    ch_range.cpMin = n;
    ch_range.cpMax = m_GuiRT_Disasm.LineIndex (line + 1) - 1;

    CHARFORMAT2 ch_format;
    ZeroMemory (& ch_format, sizeof (ch_format));

    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwEffects &= ~CFE_AUTOBACKCOLOR;
    ch_format.dwMask |= CFM_BACKCOLOR;
    ch_format.crBackColor = color;

    m_GuiRT_Disasm.SetSel (ch_range);
    m_GuiRT_Disasm.SetSelectionCharFormat (ch_format);
    m_GuiRT_Disasm.SetSelNone ();
  }
  void SetUpdateColor (CRichEditCtrl *RichCtrl, int line, DWORD dwMask, DWORD bg_clr, DWORD text_clr, BOOL bSetNone = TRUE) {
    int n = RichCtrl->LineIndex (line);
    CHARRANGE ch_range;
    ch_range.cpMin = n;
    ch_range.cpMax = RichCtrl->LineIndex (line + 1) - 1;

    CHARFORMAT2 ch_format;
    ZeroMemory (& ch_format, sizeof (ch_format));

    if (dwMask & CFM_BACKCOLOR) {
      ch_format.dwEffects &= ~CFE_AUTOBACKCOLOR;
      ch_format.crBackColor = bg_clr;
    }
    if (dwMask & CFM_COLOR) {
      ch_format.dwEffects &= ~CFE_AUTOCOLOR;
      ch_format.crTextColor = text_clr;
    }

    ch_format.cbSize = sizeof(ch_format);
    ch_format.dwMask |= dwMask;

    RichCtrl->SetSel (ch_range);
    RichCtrl->SetSelectionCharFormat (ch_format);
    if (bSetNone != FALSE)
      RichCtrl->SetSelNone ();
  }

  CViewComment m_ViewComment;
  CViewOutput m_ViewOutput;
  CViewBreakpoint m_ViewBreakpoint;
  CViewIO m_ViewIO;
  CViewWatch m_ViewWatch;
  CRichEditREF<CDebugger> m_GuiRT_Disasm;
  CRichEditREF<CDebugger> m_GuiRT_Register;
  CRichEditREF<CDebugger> m_GuiRT_Memory;
  CRichEditREF<CDebugger> m_GuiRT_Stack;
  CMainFrame *m_AttachFrame;
  gba *m_GBA;
  CSplitterWindowT<true> m_SplitterVert;
  CSplitterWindowT<false> m_SplitterRightHori;
  CSplitterWindowT<true> m_SplitterRightHori_Vert;
  BOOL m_bMasterUpdate;
};

// < FORWARD - CGPUViewer >========================================================
class CGPUViewer : public dockwins::CDockingFrameImpl<CGPUViewer, CWindow, dockwins::CVC7LikeDockingFrameTraits>, CIdleHandler, CMessageFilter {
public: 
  // Some GPU Viewer 's global settings..

  CViewPalette m_ViewPalette;
  CViewTile <ID_GPU_TILE0, 0x00000> m_ViewTile0;
  CViewTile <ID_GPU_TILE1, 0x04000> m_ViewTile1;
  CViewTile <ID_GPU_TILE2, 0x08000> m_ViewTile2;
  CViewTile <ID_GPU_TILE3, 0x0C000> m_ViewTile3;
  CViewTile <ID_GPU_TILE_SP, 0x10000> m_ViewTileSP;
  CViewNametable <ID_GPU_BG0, 0, 512 + 256> m_ViewBG0;
  CViewNametable <ID_GPU_BG1, 1, 512 + 256> m_ViewBG1;
  CViewNametable <ID_GPU_BG2, 2, 1024 + 378> m_ViewBG2;
  CViewNametable <ID_GPU_BG3, 3, 1024 + 378> m_ViewBG3;
  CViewOam <ID_GPU_SP> m_ViewOam;

  BOOL m_bMasterUpdate;
  CMainFrame *m_AttachFrame;

  gba *m_GBA;

public:
  typedef dockwins::CDockingFrameImpl<CGPUViewer, CWindow, dockwins::CVC7LikeDockingFrameTraits> baseClass;
  typedef CGPUViewer thisClass;

  CGPUViewer (gba *attachGBA, CMainFrame *attachFrame) 
    : m_GBA (attachGBA), 
    m_bMasterUpdate (FALSE), 
     m_AttachFrame (attachFrame)
  {}
  ~CGPUViewer (void) {}

  BOOL OnIdle (void) { 
    // m_ViewPalette.m_ContainerOverview.UpdateGBAItemOut ();
    if (m_bMasterUpdate != FALSE) {
      m_ViewTile0.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewTile1.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewTile2.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewTile3.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewTileSP.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewBG0.UpdateGBAItemOut ();
      m_ViewBG1.UpdateGBAItemOut ();
      m_ViewBG2.UpdateGBAItemOut ();
      m_ViewBG3.UpdateGBAItemOut ();
      m_ViewPalette.m_ContainerOverview.UpdateGBAItemOut ();
      m_ViewOam.UpdateGBAItemOut ();
    }
    return TRUE; 
  } 

  BOOL PreTranslateMessage(MSG* pMsg)
  {
    return FALSE;
  }

  LRESULT OnClose (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    ShowWindow (SW_HIDE);
    return 0;
  }

  LRESULT OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

    InitializeDockingFrame();
    m_ViewPalette.kCreate (*this, m_GBA);
    m_ViewTile0.kCreate (*this, m_GBA);
    m_ViewTile1.kCreate (*this, m_GBA);
    m_ViewTile2.kCreate (*this, m_GBA);
    m_ViewTile3.kCreate (*this, m_GBA);
    m_ViewTileSP.kCreate (*this, m_GBA);
    m_ViewBG0.kCreate (*this, m_GBA);
    m_ViewBG1.kCreate (*this, m_GBA);
    m_ViewBG2.kCreate (*this, m_GBA);
    m_ViewBG3.kCreate (*this, m_GBA);
    m_ViewOam.kCreate (*this, m_GBA);

    m_ViewTile0.SetWindowText (_T ("Tile - 0"));
    m_ViewTile1.SetWindowText (_T ("Tile - 1"));
    m_ViewTile2.SetWindowText (_T ("Tile - 2"));
    m_ViewTile3.SetWindowText (_T ("Tile - 3"));
    m_ViewTileSP. SetWindowText (_T ("Tile - SP"));

    m_ViewBG0.SetWindowText (_T ("BG - 0"));
    m_ViewBG1.SetWindowText (_T ("BG - 1"));
    m_ViewBG2.SetWindowText (_T ("BG - 2 / RGB Out"));
    m_ViewBG3.SetWindowText (_T ("BG - 3"));

    // m_ViewPalette.ShowWindow (SW_SHOWNORMAL);
    _Module.GetMessageLoop ()->AddIdleHandler (this);
    return 0;
  }

  LRESULT OnViewPalette (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewPalette.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewTile0 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewTile0.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewTile1 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewTile1.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewTile2 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewTile2.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewTile3 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewTile3.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewBG0 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewBG0.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewBG1 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewBG1.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewBG2 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewBG2.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewBG3 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewBG3.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewTileSP (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewTileSP.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnViewOam (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_ViewOam.OnTestCreate (*this, GetMenu ());
    return 0;
  }
  LRESULT OnEnableDone (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {

    m_bMasterUpdate = TRUE;
    EnableMenuItem (GetMenu (), ID_OPEN_GPU_MASTER_ENABLE, MF_GRAYED | MF_BYCOMMAND );
    EnableMenuItem (GetMenu (), ID_CLOSE_GPU_MASTER_ENABLE, MF_ENABLED | MF_BYCOMMAND );
    SetWindowText (_T ("GPUViewer@Active!"));
    return 0;
  }

  LRESULT OnCloseDone (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
    m_bMasterUpdate = FALSE;
    EnableMenuItem (GetMenu (), ID_OPEN_GPU_MASTER_ENABLE, MF_ENABLED | MF_BYCOMMAND );
    EnableMenuItem (GetMenu (), ID_CLOSE_GPU_MASTER_ENABLE, MF_GRAYED | MF_BYCOMMAND );
    SetWindowText (_T ("GPUViewer@Frozen!"));
    return 0;
  }

  DECLARE_FRAME_WND_CLASS(_T("CGPUViewer"), IDR_GPU_VIEWER)
  BEGIN_MSG_MAP (thisClass)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_CLOSE, OnClose)
    COMMAND_ID_HANDLER (ID_GPU_PALETTERVIEWER, OnViewPalette)
    COMMAND_ID_HANDLER (ID_GPU_TILE0, OnViewTile0)
    COMMAND_ID_HANDLER (ID_GPU_TILE1, OnViewTile1)
    COMMAND_ID_HANDLER (ID_GPU_TILE2, OnViewTile2)
    COMMAND_ID_HANDLER (ID_GPU_TILE3, OnViewTile3)
    COMMAND_ID_HANDLER (ID_GPU_TILE_SP, OnViewTileSP)
    COMMAND_ID_HANDLER (ID_GPU_BG0, OnViewBG0)
    COMMAND_ID_HANDLER (ID_GPU_BG1, OnViewBG1)
    COMMAND_ID_HANDLER (ID_GPU_BG2, OnViewBG2)
    COMMAND_ID_HANDLER (ID_GPU_BG3, OnViewBG3)
    COMMAND_ID_HANDLER (ID_GPU_SP, OnViewOam)
    COMMAND_ID_HANDLER (ID_OPEN_GPU_MASTER_ENABLE, OnEnableDone)
    COMMAND_ID_HANDLER (ID_CLOSE_GPU_MASTER_ENABLE, OnCloseDone)
    // MESSAGE_HANDLER (WM_SIZE, OnSize)
    CHAIN_MSG_MAP (baseClass)
  END_MSG_MAP()
};






// < IMPL - CMainFrame >========================================================
#ifndef CMAINFRAME_IMPL

BOOL CMainFrame::OnIdle (void) {
  if (    m_bLoadRom != FALSE
    && m_GBA != NULL) {
    if (m_bPause == FALSE) {
      // Run GBA device. 
      DWORD t = timeGetTime ();
      if (gba_frame (m_GBA) != 0) { 
        // GBA trap..
        m_Debugger->SetDebuggerInterrupt (TRUE);
        // Update disasm window. 
        m_Debugger->UpdateFollowPC (m_Debugger->m_ViewDisasmInfos.iMiddleAddressFull, TRUE);
        m_Debugger->OnUpdateStackWork (TRUE, TRUE);
        m_Debugger->OnUpdateRegisterWork (TRUE);
        m_Debugger->OnUpdateMemoryWork (TRUE);
        m_Debugger->m_ViewBreakpoint.m_panel_arm7.enumShowCurrentBreakPoint2 ();
      }
       //printf ("time-elsp:%d\n",  timeGetTime ()- t);

      // Sync timing...

    }
    // Update other view infos.
  }
  return TRUE;
}

// GameBoy Audio callback
void callc CMainFrame::GBA_APU_SwapBuffer  (struct gba *gba, apu_framebuffer *buf) {

    CMainFrame *frame = (CMainFrame *)gba->platform_io.sound_obj;
    CGameAudioSample16_44100Hz_2 *audio = frame->m_GameAudio;

    audio->write (buf->buf);
}
// GameBoy VSync callback
void callc CMainFrame::GBA_VSync  (struct gba *gba) {

    CMainFrame *frame = (CMainFrame *)gba->platform_io.vsync_obj;

    __int64 timeStampFreq;
    __int64 timeCurrent;
  QueryPerformanceFrequency ((LARGE_INTEGER *)& timeStampFreq);

  if (frame->m_i64TimeStart == 0)
    QueryPerformanceCounter ( (LARGE_INTEGER *)& frame->m_i64TimeStart);
  __int64 s = timeStampFreq * 167 / 10000;

   while (TRUE) {
     QueryPerformanceCounter ( (LARGE_INTEGER *)& timeCurrent);
     __int64 t = timeCurrent - frame->m_i64TimeStart;
     
     if (t >= s ) {
       frame->m_i64TimeStart = timeCurrent;
       break;
     }
     _mm_pause ();
   }
}
// GameBoy Device Controller Callback 
void callc CMainFrame::GBA_Controller_Callback  (gba *gba, void *buf) {
    
  CMainFrame *frame = (CMainFrame *)gba->platform_io.controller_obj;
  CGameInput & dev_input = frame->m_GameInput;
  Controller_Mapper & key_settings = frame->m_Keypad_Settings;

  dev_input.poll ();

  struct GBA_KEYPAD {
    uint16_t a:1; 
    uint16_t b:1; 
    uint16_t select:1; 
    uint16_t start:1; 
    uint16_t right:1; 
    uint16_t left:1; 
    uint16_t up:1; 
    uint16_t down:1; 
    uint16_t r:1;  
    uint16_t l:1;  
  } pad;

  * (uint16_t *)& pad =  * (uint16_t *)buf;
  if (dev_input.isPressedFirst (key_settings.a)) pad.a = 0;
  else if (dev_input.isUpFirst (key_settings.a)) pad.a = 1;
  if (dev_input.isPressedFirst (key_settings.b)) pad.b = 0;
  else if (dev_input.isUpFirst (key_settings.b)) pad.b = 1;
  if (dev_input.isPressedFirst (key_settings.select)) pad.select = 0;
  else if (dev_input.isUpFirst (key_settings.select)) pad.select = 1;
  if (dev_input.isPressedFirst (key_settings.start)) pad.start = 0;
  else if (dev_input.isUpFirst (key_settings.start)) pad.start = 1;
  if (dev_input.isPressedFirst (key_settings.right)) pad.right = 0;
  else if (dev_input.isUpFirst (key_settings.right)) pad.right = 1;
  if (dev_input.isPressedFirst (key_settings.left)) pad.left = 0;
  else if (dev_input.isUpFirst (key_settings.left)) pad.left = 1;
  if (dev_input.isPressedFirst (key_settings.up)) pad.up = 0;
  else if (dev_input.isUpFirst (key_settings.up)) pad.up = 1;
  if (dev_input.isPressedFirst (key_settings.down)) pad.down = 0;
  else if (dev_input.isUpFirst (key_settings.down)) pad.down = 1;
  if (dev_input.isPressedFirst (key_settings.weak_right)) pad.r = 0;
  else if (dev_input.isUpFirst (key_settings.weak_right)) pad.r = 1;
  if (dev_input.isPressedFirst (key_settings.weak_left)) pad.l = 0;
  else if (dev_input.isUpFirst (key_settings.weak_left)) pad.l = 1;

  * (uint16_t *)buf =  * (uint16_t *)& pad;
}

void callc CMainFrame::GBA_Video_Callback  (gba *gba, struct ppu_framebuffer *buf) {
    
  CMainFrame &frame = *(CMainFrame *)gba->platform_io.lcd_obj;
  CGameVideoRoot *dev_display = frame.m_GameVideo;

    CRect rcClient;
    frame.GetClientRect (rcClient);

    if (rcClient.Width () >0 
        && rcClient.Height () >0 && frame.IsWindowVisible ()) {
          // CTimingOut<> t;
            if (dev_display->getBackBufferSize () != CSize (rcClient.Width (), rcClient.Height ()))
              dev_display->resetDisplayCache (& CSize (rcClient.Width (), rcClient.Height ()), CGameVideoRoot::DisplayFormat_KeepOld);
            dev_display->blit (& CGameVideoRoot::CGameVideoCache (buf->pixel_16, buf->pitch, buf->w, buf->h));
    }
}
void CMainFrame::GBA_Breakpoint_Callback  (struct gba *gba, breakpoint *bp) {
  CMainFrame &frame = *(CMainFrame *)gba->platform_io.lcd_obj;
  CGameVideoRoot *dev_display = frame.m_GameVideo;

  if (bp != NULL) {

    CString strReason;
    BOOL bAddr = FALSE;

    switch (bp->reason) {
    case BREAKPOINT_REASON_MEM_READ: bAddr = TRUE; strReason = _T ("MEMREAD BREAKPOINT!"); break;
    case BREAKPOINT_REASON_MEM_WRITE: bAddr = TRUE; strReason = _T ("MEMWRITE BREAKPOINT!"); break;
    case BREAKPOINT_REASON_R15: bAddr = TRUE; strReason = _T ("R15, PC BREAKPOINT!"); break;
    case BREAKPOINT_REASON_CURSOR: strReason = _T ("CURSOR BREAKPOINT!"); break;
    case BREAKPOINT_REASON_SOFTWARE_INTERRUPT: strReason = _T ("SWI BREAKPOINT!"); break;
    case BREAKPOINT_REASON_STEP_IN: strReason = _T ("STEP-IN BREAKPOINT!"); break;
    case BREAKPOINT_REASON_STEP_OUT: strReason = _T ("STEP-OUT BREAKPOINT!"); break;
    case BREAKPOINT_REASON_STEP_OVER: strReason = _T ("STEP-OVER BREAKPOINT!"); break;
    case BREAKPOINT_REASON_INTERRUPT: strReason = _T ("BRK BREAKPOINT!"); break;
    case BREAKPOINT_REASON_HARDWARE_INTERRUPT: strReason = _T ("IRQ BREAKPOINT!"); break;
    default:
      ATLASSERT (FALSE);
      break;
    }
    if (bAddr != FALSE)
      frame.m_Debugger->m_ViewOutput.AppendTextV (_T ("BREAKPOINT-EVENT  M:%s, A:%s, E:%s, H:%d\n"),
            strReason.GetBuffer (),
            bp->address,
            bp->describe,
            bp->match_expect);
    else 
      frame.m_Debugger->m_ViewOutput.AppendTextV (_T ("BREAKPOINT-EVENT  M:%s, E:%s, H:%d\n"),
            strReason.GetBuffer (),
            bp->describe,
            bp->match_expect);
    frame.m_Debugger->m_ViewOutput.WrapToTail ();
  }
}

LRESULT CMainFrame::OnRunEmulator (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  m_Debugger->SetDebuggerInterrupt (FALSE);
  return 0;
}

LRESULT CMainFrame::OnDirect3D_Fast (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  delete m_GameVideo;
  m_GameVideo = new CGameVideoDirect3D ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_hardfilter (CGameVideoDirect3D::iHardFilter_fast_disable_softfilter, & CSize (240, 160), & CSize (240, 160));
  return 0;
}

LRESULT CMainFrame::OnDirect3D_Std (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CGameVideoRoot::  DisplaySoftwareFilter iSoftold = m_GameVideo->get_device_softfilter ();
  delete m_GameVideo;
  m_GameVideo = new CGameVideoDirect3D ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_hardfilter (CGameVideoDirect3D::iHardFilter_stdandenablle_softfilter, & CSize (240, 160), & CSize (240, 160));
  
  CRect rcClient;
  GetClientRect (& rcClient);
  m_GameVideo->set_device_softfilter (iSoftold, & CSize (rcClient.Width (), rcClient.Height ()));
  return 0;
}

LRESULT CMainFrame::OnDirect3D_Bilinear (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  delete m_GameVideo;
  m_GameVideo = new CGameVideoDirect3D ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_hardfilter (CGameVideoDirect3D::iHardFilter_biliner_disable_softfilter, & CSize (240, 160), & CSize (240, 160));
  return 0;
}

LRESULT CMainFrame::OnGL_1 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CGameVideoRoot::  DisplaySoftwareFilter iSoftold = m_GameVideo->get_device_softfilter ();
  delete m_GameVideo;
  m_GameVideo = new CGameVideoGL_1 ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_softfilter (iSoftold, NULL);
  return 0;
}

LRESULT CMainFrame::OnGDI_DIB (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CGameVideoRoot::  DisplaySoftwareFilter iSoftold = m_GameVideo->get_device_softfilter ();
  delete m_GameVideo;
  m_GameVideo = new CGameVideoGDI ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_softfilter (iSoftold, NULL);
  return 0;
}

LRESULT CMainFrame::OnFilter_NO (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CRect rcClient;
  GetClientRect (& rcClient);
  m_GameVideo->set_device_softfilter (CGameVideoRoot::DisplayFilter_Nouse, & CSize (rcClient.Width (), rcClient.Height ()));
  return 0;
}

LRESULT CMainFrame::OnFilter_Interp (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CRect rcClient;
  GetClientRect (& rcClient);
  m_GameVideo->set_device_softfilter (CGameVideoRoot::DisplayFilter_InterpolationX, & CSize (rcClient.Width (), rcClient.Height ()));
  return 0;
}

LRESULT CMainFrame::OnFilter_Interp_2 (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CRect rcClient;
  GetClientRect (& rcClient);
  m_GameVideo->set_device_softfilter (CGameVideoRoot::DisplayFilter_InterpolationX_Half, & CSize (rcClient.Width (), rcClient.Height ()));
  return 0;
}

LRESULT CMainFrame::OnFilter_Scanline (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CRect rcClient;
  GetClientRect (& rcClient);
  m_GameVideo->set_device_softfilter (CGameVideoRoot::DisplayFilter_Scanline, & CSize (rcClient.Width (), rcClient.Height ()));
  return 0;
}

  int CMainFrame::ResetRom (CString *strPathRom) {

    // Try save battery 
    FILE *fd;
    if (m_bLoadRom != FALSE 
      && m_PathSave.GetLength () > 0) {
      if ((fd = _tfopen (m_PathSave ,_T("wb+"))) != NULL) {
        gba_save_battery (m_GBA, fd);
        fclose (fd);
      }
    }

    m_bLoadRom = FALSE;
    m_PathSave = _T ("");
    m_PathROM = _T ("");

    extern unsigned char bios[];
    gba_set_bios (m_GBA, bios, 0x4000);

    if ((fd = _tfopen (*strPathRom ,_T("rb"))) == NULL) {
      AtlMessageBox (*this, _T ("Open files failed"));
      return -1;
    } else if (gba_fetchrom (m_GBA, fd) != 0) {
      AtlMessageBox (*this, _T ("Not a GBA Rom.!"));
      return -2;
    }
     // Try find save files. 
    m_PathROM = * strPathRom;

    int iLen =m_PathROM.GetLength ();
    int iDotpos;
    for ( iDotpos = iLen - 1; iDotpos != -1; iDotpos--) {
      if (m_PathROM [iDotpos] == _T ('.'))
        break;
    }
    if (iDotpos != iLen -1) {
      m_PathSave = m_PathROM;
      m_PathSave.SetAt (iDotpos+1, _T ('s'));
      m_PathSave.SetAt (iDotpos+2, _T ('s'));
      m_PathSave.SetAt (iDotpos+3, _T ('a'));
      // m_PathSave.SetAt (iDotpos+4, 0);

      FILE *fp = _tfopen (m_PathSave, _T ("rb"));
      if ( fp != NULL) {
        gba_read_battery (m_GBA, fp);
        fclose (fp);
      }
    }
    // Reset video 

    // Reset sound 

    // Reset joypad.

    // Reset debugger 

    // Pause it.
    m_bLoadRom = TRUE;
    m_Debugger->SetDebuggerInterrupt (TRUE);
    m_Debugger->OnUpdateMemoryWork (TRUE);
    m_Debugger->OnUpdateRegisterWork (TRUE);
    m_Debugger->UpdateSpecAddress (0x8000000);

    // Install callback io.
    driver drv;
    ZeroMemory (& drv, sizeof (drv));

    drv.lcd_obj = (void *)this;
    drv.video_post = GBA_Video_Callback;
    drv.controller_drv = GBA_Controller_Callback;
    drv.controller_obj = (void *)this;
    drv.sound_obj = (void *)this;
    drv.sound_post = GBA_APU_SwapBuffer;
    drv.vsync_obj = (void *)this;
    drv.vsync = GBA_VSync;
    drv.bp_obj = (void *)this;
    drv.bp_hit = GBA_Breakpoint_Callback;

    gba_setdriver (m_GBA, & drv);
    gba_set_apubuffer (m_GBA, APU_SAMPLE_TYPE_SINT16, 44100);

    return 0;
  }


LRESULT CMainFrame::OnLoadRom (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  CFileDialogFilter df (_T("GBA files (*.gba; *.agb)| *.gba; *.agb"));
  CFileDialog fd_dlg  (   TRUE, _T ("Open ROMS..."), 
                              NULL, OFN_ALLOWMULTISELECT | OFN_EXPLORER, 
                              df, 
                              this->m_hWnd        );

  if ( fd_dlg.DoModal () == IDOK) 
    return ResetRom (& CString (fd_dlg.m_szFileName));
  return 0;
}

LRESULT CMainFrame::OnKeyModeForge(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  m_GameInput.resetPlayType (*this, CGameInput::FOREGROUND);
  CheckMenuItem (::GetMenu (*this), ID_MODE_BACKGROUND, MF_BYCOMMAND | MF_UNCHECKED);
  CheckMenuItem (::GetMenu (*this), ID_MODE_FORGEGROUND, MF_BYCOMMAND | MF_CHECKED);
  return 0;
}
LRESULT CMainFrame::OnKeyModeBack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  m_GameInput.resetPlayType (*this, CGameInput::BACKGROUND);
  CheckMenuItem (::GetMenu (*this), ID_MODE_BACKGROUND, MF_BYCOMMAND | MF_CHECKED);
  CheckMenuItem (::GetMenu (*this), ID_MODE_FORGEGROUND, MF_BYCOMMAND | MF_UNCHECKED);
  return 0;
}
LRESULT CMainFrame::OnOpenDebuggerWindow (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  m_Debugger->ShowWindow (SW_SHOWNORMAL);
  CheckMenuItem (::GetMenu (*this), ID_MODE_FORGEGROUND, MF_BYCOMMAND | MF_GRAYED);
  return 0;
}
LRESULT CMainFrame::OnOpenGPUWindow (WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled) {
  m_GPUViewer->ShowWindow (SW_SHOWNORMAL);
  CheckMenuItem (::GetMenu (*this), ID_ADV_GPUVIEWER, MF_BYCOMMAND | MF_GRAYED);
  return 0;
}





LRESULT CMainFrame::OnKeyPadSettings(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
  BYTE keyset[10];
  m_Keypad_Settings.GetMask (& keyset[0]);

  if (CKeySettings (& keyset[0]).DoModal (*this) == IDOK) 
    m_Keypad_Settings.SetMask (& keyset[0]);
  return 0;
}

LRESULT CMainFrame::OnDropFile (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
  // QueryDropFile
  return 0;
}


LRESULT CMainFrame::OnCreate (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

  m_GameInput.resetPlayType (*this);

  _Module.GetMessageLoop ()->AddIdleHandler (this);

//    m_ViewComment.kCreate (*this);
//    m_ViewOutput.kCreate (*this);

  gba_create (& m_GBA);

  m_GameVideo = new CGameVideoDirect3D ();
  m_GameVideo->attach_window (*this);
  m_GameVideo->reset_device (& CSize (240, 160), CGameVideoRoot::DisplayFormat_RGB16);
  m_GameVideo->set_device_hardfilter (CGameVideoDirect3D::iHardFilter_fast_disable_softfilter, & CSize (240, 160), & CSize (240, 160));

  m_GameAudio = new CGameAudioSample16_44100Hz_2 (*this);
  m_GameAudio->init ();
  // m_GameAudio->StartupRenderThread ();

  m_Debugger = new CDebugger (m_GBA, this);
  m_Debugger->CreateEx (NULL, CRect (0, 0, 48, 32), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW);
  m_Debugger->ResizeClient (720, 480, TRUE);
  m_Debugger->SetWindowText (_T ("AGB Debugger"));

  m_GPUViewer = new CGPUViewer (m_GBA, this);
  m_GPUViewer->CreateEx (NULL, CRect (0, 0, 48, 32), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW);
  m_GPUViewer->ResizeClient (720, 480, TRUE);
  m_GPUViewer->SetWindowText (_T ("GPUViewer@Frozen!"));

  return 0;
}

#endif 