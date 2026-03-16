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

// stdwtl.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_STD_WTL_INCLUED_HEADER_)
#  define _STD_WTL_INCLUED_HEADER_

// Change these values to use different versions
# define WINVER		0x0601
# define _WIN32_WINNT	0x0601
# define _WIN32_IE	0x0700
# define _RICHEDIT_VER	0x0500
# define _WTL_NO_CSTRING 
# define END_MSG_MAP_EX	END_MSG_MAP

# include <atlbase.h>
# include <atlcoll.h>
# include <WTL\atlapp.h>
# include <WTL\atlscrl.h>
# include <WTL\atlmisc.h>
# include <WTL\atlframe.h>
# include <WTL\atlctrls.h>
# include <WTL\atldlgs.h>
# include <WTL\atlctrlw.h>
# include <WTL\atlctrlx.h>
# include <WTL\atlctrls.h>
# include <WTL\atlsplit.h>
# include <vector>

extern CAppModule _Module;

# include <dockwins\atlgdix.h>
# include <dockwins\DWAutoHide.h>
# include <dockwins\CustomTabCtrl.h>
# include <dockwins\DockingFrame.h>
# include <dockwins\PlainTextView.h>
# include <dockwins\DotNetTabCtrl.h>
# include <dockwins\FlyingTabs.h>
# include <dinput.h>
# include "..\resource.h"
# include <atlwin.h>
# include "CDynamicMessageMap.hxx"

# include <d3d9.h>

// Embedded style list
# if defined (_M_IX86)
  # pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
# elif defined (_M_IA64)
  # pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
# elif defined (_M_X64)
  # pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
# else
  # pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
# endif

template<typename T> void release_com (T*& T_) {
  if (T_ != NULL)  {
    HRESULT sig= T_->Release ();
    ATLASSERT (SUCCEEDED (sig));
    T_ = NULL;
  }
}
template<typename T> void release_object (T*& T_) {
  if (T_ != NULL)  {
    delete T_;
    T_ = NULL;
  }
}
template<typename T> void release_objects (T*& T_) {
  if (T_ != NULL)  {
    delete [] T_;
    T_ = NULL;
  }
}

template<typename T> LPCTSTR GetIntBuf (T value) {
  static TCHAR szIntBuf[48];
  _stprintf (szIntBuf, _T("%d"), value);
  return szIntBuf;
}
template<typename T> LPCTSTR GetUintBuf (T value) {
  static TCHAR szIntBuf[48];
  _stprintf (szIntBuf, _T("%u"), value);
  return szIntBuf;
}

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CWidgetsREF : public CWindowImpl<CWidgetsREF<T, TWinTraits>, CWindow>,
                                              CDynamicMessageMap<CWindowImpl<CWidgetsREF<T, TWinTraits>, CWindow>, T> {
public:
  CWidgetsREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CWidgetsREF<T, TWinTraits>, CWindow>, T> (this, TThis_) {}
  ~CWidgetsREF (void) {}
  typedef CWindowImpl<CWidgetsREF<T, TWinTraits>, CWindow> baseClass;
  typedef CWidgetsREF thisClass;
  
  DECLARE_WND_CLASS(_T ("CWidgetsREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CEditREF : public CWindowImpl<CEditREF<T, TWinTraits>, CRichEditCtrl>,
                                              CDynamicMessageMap<CWindowImpl<CEditREF<T, TWinTraits>, CRichEditCtrl>, T> {
public:
  CEditREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CEditREF<T, TWinTraits>, CRichEditCtrl>, T> (this, TThis_) {}
  ~CEditREF (void) {}
  typedef CWindowImpl<CEditREF<T, TWinTraits>, CRichEditCtrl> baseClass;
  typedef CEditREF thisClass;
  
  DECLARE_WND_CLASS(_T ("CEditREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CRichEditREF : public CWindowImpl<CRichEditREF<T, TWinTraits>, CRichEditCtrl>,
                                              CDynamicMessageMap<CWindowImpl<CRichEditREF<T, TWinTraits>, CRichEditCtrl>, T> {
public:
  CRichEditREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CRichEditREF<T, TWinTraits>, CRichEditCtrl>, T> (this, TThis_) {}
  ~CRichEditREF (void) {}
  typedef CWindowImpl<CRichEditREF<T, TWinTraits>, CRichEditCtrl> baseClass;
  typedef CRichEditREF thisClass;
  
  DECLARE_WND_CLASS(_T ("CRichEditREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CTreeCtrlREF : public CWindowImpl<CTreeCtrlREF<T, TWinTraits>, CTreeViewCtrlEx>,
                                              CDynamicMessageMap<CWindowImpl<CTreeCtrlREF<T, TWinTraits>, CTreeViewCtrlEx>, T> {
public:
  CTreeCtrlREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CTreeCtrlREF<T, TWinTraits>, CTreeViewCtrlEx>, T> (this, TThis_) {}
  ~CTreeCtrlREF (void) {}
  typedef CWindowImpl<CTreeCtrlREF<T, TWinTraits>, CTreeViewCtrlEx> baseClass;

  DECLARE_WND_CLASS(_T ("CTreeCtrlREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CListCtrlREF : public CWindowImpl<CListCtrlREF<T, TWinTraits>, CListViewCtrl>,
                                              CDynamicMessageMap<CWindowImpl<CListCtrlREF<T, TWinTraits>, CListViewCtrl>, T> {
public:
  CListCtrlREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CListCtrlREF<T, TWinTraits>, CListViewCtrl>, T> (this, TThis_) {}
  ~CListCtrlREF (void) {}
  typedef CWindowImpl<CListCtrlREF<T, TWinTraits>, CListViewCtrl> baseClass;

  DECLARE_WND_CLASS(_T ("CListCtrlREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CStaticREF : public CWindowImpl<CStaticREF<T, TWinTraits>, CStatic>,
                                              CDynamicMessageMap<CWindowImpl<CStaticREF<T, TWinTraits>, CStatic>, T> {
public:
  CStaticREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CStaticREF<T, TWinTraits>, CStatic>, T> (this, TThis_) {}
  ~CStaticREF (void) {}
  typedef CWindowImpl<CStaticREF<T, TWinTraits>, CListViewCtrl> baseClass;

  DECLARE_WND_CLASS(_T ("CStaticREF"))
  SET_DYNA_MSG_MAP()
};

template <class T, class TWinTraits = ATL::CControlWinTraits>
struct CTrackBarCtrlREF : public CWindowImpl<CTrackBarCtrlREF<T, TWinTraits>, CTrackBarCtrl>,
                                              CDynamicMessageMap<CWindowImpl<CTrackBarCtrlREF<T, TWinTraits>, CTrackBarCtrl>, T> {
public:
  CTrackBarCtrlREF (T *TThis_):CDynamicMessageMap<CWindowImpl<CTrackBarCtrlREF<T, TWinTraits>, CTrackBarCtrl>, T> (this, TThis_) {}
  ~CTrackBarCtrlREF (void) {}
  typedef CWindowImpl<CTrackBarCtrlREF<T, TWinTraits>, CTrackBarCtrl> baseClass;

  DECLARE_WND_CLASS(_T ("CTrackBarCtrlREF"))
  SET_DYNA_MSG_MAP()
};

#define WM_PAINT2 (WM_USER+23000)

template <class T>
struct CRichEditREF2: public CScrollImpl<CRichEditREF2<T>>,
                                                            public CWindowImpl <CRichEditREF2<T>, CRichEditCtrl, ATL::CControlWinTraits>  {

  T &m_attachTThis_;

  CRichEditREF2 (T &TThis_): m_attachTThis_ (TThis_) { m_msg_call = NULL; }

  LRESULT (T::*m_msg_call)(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam,  BOOL& bHandled);

  void SetCallbackFilter (LRESULT (T::*call)(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam,  BOOL& bHandled)) {
      m_msg_call = call;
  }

	void DoPaint(CDCHandle dc) {
    // LPARAM is window dc.
    BOOL bTempHandle = FALSE;
    (m_attachTThis_.*m_msg_call) (*this, WM_PAINT, 0, (LPARAM) dc.m_hDC, bTempHandle); 
  }

  BEGIN_MSG_MAP (CRichEditREF2)
		bHandled = TRUE; 
    if (m_msg_call != NULL) 
	  	lResult = (m_attachTThis_.*m_msg_call) (hWnd, uMsg, wParam, lParam, bHandled); 
    else 
      bHandled = FALSE;
		if(bHandled) 
			return TRUE; 
    CHAIN_MSG_MAP(CScrollImpl<CRichEditREF2<T>>)
  END_MSG_MAP()
};

struct DirectWrite16 {
  uint16_t *vptr;
  uintptr_t vpitch16;
  uintptr_t width;
  uintptr_t height;
};
template <class T, class TBase, class TWinTraits, DWORD t_dwFrameBuffer_X, DWORD t_dwFrameBuffer_Y, BOOL t_bSetScroll = FALSE, BOOL t_bHookDoPaint = FALSE> 
struct ATL_NO_VTABLE CDirectWrite16WindowImpl : public CWindowImpl<T, TBase, TWinTraits>, public CScrollImpl<T>
{
  static const int col_byte = 2;

  BEGIN_MSG_MAP(CDirectWrite16WindowImpl)
    MESSAGE_HANDLER (WM_CREATE, OnCreate)
    MESSAGE_HANDLER (WM_DESTROY, OnDestroy)
    CHAIN_MSG_MAP(CScrollImpl<T>)
  END_MSG_MAP()

  IDirect3D9 *m_Direct3D9;
  IDirect3DSurface9 *m_Direct3DSurface;
  IDirect3DDevice9 *m_Direct3DDevice;
  D3DPRESENT_PARAMETERS m_D3dSettings;
  T *m_pThis_;

  void DoPaintHook (HWND window, CDCHandle dc) {
    if (t_bHookDoPaint != FALSE) {
      // must be implemented in a derived class
      ATLASSERT(FALSE);
    }
  }

  virtual void DoPaint (CDCHandle dc) {
    if (t_bHookDoPaint == FALSE) {
      // must be implemented in a derived class
      ATLASSERT(FALSE);
    } else {
      ((T *)(this)) -> DoPaintHook (m_hWnd, dc);
    }
  }

  // CDirectWrite16WindowImpl (T *pThis_): m_pThis_ (pThis_), m_Direct3D9 (NULL), m_Direct3DSurface (NULL), m_Direct3DDevice (NULL) {} 
  CDirectWrite16WindowImpl (void): m_Direct3D9 (NULL), m_Direct3DSurface (NULL), m_Direct3DDevice (NULL) {} 
  ~CDirectWrite16WindowImpl (void) {
    releaseCom ();
    // Detach ();
  }

  void releaseCom (void) {
    release_com <IDirect3DSurface9> (m_Direct3DSurface);
    release_com <IDirect3DDevice9> (m_Direct3DDevice);
    release_com <IDirect3D9> (m_Direct3D9);
  }

  __forceinline  void 
  drawHorzLine (DirectWrite16 &directwrite16, int iyPos, int iPos, int iVec, const unsigned short col16) {
    uint16_t *vptr = & directwrite16.vptr[directwrite16.vpitch16 * iyPos+iPos];
    for (int Id=  0; Id != iVec; Id++)
      vptr [Id] = col16;
  }

  __forceinline  void 
  drawVertLine (DirectWrite16 &directwrite16, int ixPos, int iPos, int iVec, const unsigned short col16) {
    uint16_t *vptr = & directwrite16.vptr[directwrite16.vpitch16 * iPos+ixPos];
    const uintptr_t goal = directwrite16.vpitch16 * iVec;
    for (int Id=  0; Id != goal; Id+= directwrite16.vpitch16)
      vptr [Id] = col16;
  }

  __forceinline  void 
  fillRect (DirectWrite16 &directwrite16, CRect &drawRect, const unsigned short col16) {
    const int width = drawRect.Width ();
    const int height = drawRect.Height ();
    uint16_t *ptr = & directwrite16.vptr[drawRect.top * directwrite16.vpitch16+drawRect.left];
    uintptr_t rva_pitch = directwrite16.vpitch16 - width;

    for (int yvec = height; yvec != 0; yvec--) {
      for (int xvec = width; xvec != 0; xvec--) {
        *ptr++ = col16;
      }
      ptr += rva_pitch;
    }
  }
  static __forceinline 
  uint16_t alpha16 (uint16_t tilePixel, uint16_t backdrop, uint16_t coeff1, uint16_t coeff2) {

    uint32_t u_out;
    uint32_t v_out;
    uint32_t   out;

    u_out = tilePixel | tilePixel << 16;
    u_out&= 0x3E07C1F;
    u_out*= coeff1;
    v_out = backdrop | backdrop << 16;
    v_out&= 0x3E07C1F;
    v_out*= coeff2;
    /* 0000 0011 1110 0000 0111 1100 0001 1111*/
    out = v_out +u_out >> 4;
    /* Check pixel saturation */
  #if 0
    if (out & 0x20)
      out |= 0x1F; /* mod:1 */
    if (out & 0x8000)
      out |= 0x7C00;/* mod:4 */
    if (out & 0x4000000)/* mod:2 */
      out |=   0x3E00000;
  #else 
      out |= 0x4008020 - (out >> 5 & 0x200401);
  #endif 
    out&=    0x3E07C1F;
    return out | out >> 16;
  }
  __forceinline  void  // alpha := 0 ~31
  alphaRect (DirectWrite16 &directwrite16, CRect &drawRect, const unsigned short col16, uint8_t src_alpha, uint8_t dst_alpha) {
    const int width = drawRect.Width ();
    const int height = drawRect.Height ();
    uint16_t *ptr = & directwrite16.vptr[drawRect.top * directwrite16.vpitch16+drawRect.left];
    uintptr_t rva_pitch = directwrite16.vpitch16 - width;
    uint32_t dst_mix = col16 | col16 << 16;
    dst_mix&= 0x3E07C1F;
    dst_mix*= dst_alpha;
    // Alpha pixel.
    for (int yvec = height; yvec != 0; yvec--) {
      for (int xvec = width; xvec != 0; xvec--) {
        uint32_t pix_mix;
        uint16_t src_pix = *ptr;
        uint32_t src_mix = src_pix | src_pix << 16;
        src_mix *= src_alpha;
        pix_mix = src_mix + dst_mix >> 4;
        pix_mix |= 0x4008020 - (pix_mix >> 5 & 0x200401);
        pix_mix&=    0x3E07C1F;
        *ptr++ = pix_mix;
      }
      ptr += rva_pitch;
    }
  }

  __forceinline  void 
  frameRect (DirectWrite16 &directwrite16, CRect &drawRect, const unsigned short col16) {
    const int width = drawRect.Width ();
    const int height = drawRect.Height ();
    uint16_t *ptr = & directwrite16.vptr[drawRect.top * directwrite16.vpitch16+drawRect.left];
    uint16_t *ptr2 = ptr[directwrite16.vpitch16* (height - 1)];
    uint16_t *ptr3 = ptr;
    uint16_t *ptr4 = ptr[width-1];

    for (int x = 0; x != width; x++)
      ptr[x] = col16;
    for (int x = 0; x != width; x++)
      ptr2[x] = col16;

    for (int y = height; y != 0; y--) {
      *ptr3 = col16;
      ptr3 += directwrite16.vpitch16;
    }
    for (int y = height; y != 0; y--) {
      *ptr4 = col16;
      ptr4 += directwrite16.vpitch16;
    }
  }

  BOOL LockSurface (DirectWrite16 &directwrite16) {
    BOOL ret = FALSE;
    D3DLOCKED_RECT d3dlock;
    D3DSURFACE_DESC d3dsurface_desc;
    if (SUCCEEDED (m_Direct3DSurface->LockRect (& d3dlock, NULL, D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE))) {
      if (SUCCEEDED (m_Direct3DSurface->GetDesc (& d3dsurface_desc))) {
        directwrite16.vptr = (uint16_t *)d3dlock.pBits;
        directwrite16.vpitch16 = d3dlock.Pitch >> 1;
        directwrite16.width = d3dsurface_desc.Width;
        directwrite16.height =  d3dsurface_desc.Height;
        return TRUE;
      } 
      ATLASSERT (0);
      m_Direct3DSurface->UnlockRect ();
    }
    ATLASSERT (0);
    return FALSE;
  }

  BOOL UnlockSurface (void) {
    m_Direct3DSurface->UnlockRect ();
    return TRUE;
  }

  void blitFullFromSurface (void) {
    CRect rcClient (0, 0, t_dwFrameBuffer_X, t_dwFrameBuffer_Y) ;

    IDirect3DSurface9 *ISurface9;
    m_Direct3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, & ISurface9);
    m_Direct3DDevice->UpdateSurface (m_Direct3DSurface, rcClient, ISurface9, & CPoint (0, 0));
    ISurface9->Release ();
    // FIXME: Direct3D may be lost when switched to a full-screen Direct3D rendering device
    HRESULT sig = m_Direct3DDevice->Present (rcClient, rcClient, NULL, NULL);
    ATLASSERT (SUCCEEDED (sig));
  }

  void blitRectFromSurface (CRect & rcBlit) {
    if (rcBlit.right > t_dwFrameBuffer_X)
      rcBlit.right = t_dwFrameBuffer_X;
    if (rcBlit.bottom > t_dwFrameBuffer_Y)
      rcBlit.bottom = t_dwFrameBuffer_Y;
    IDirect3DSurface9 *ISurface9;
    m_Direct3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, & ISurface9);
    m_Direct3DDevice->UpdateSurface (m_Direct3DSurface, rcBlit, ISurface9, & CPoint (0, 0));
    ISurface9->Release ();
    rcBlit.OffsetRect (CSize (-rcBlit.left, -rcBlit.top));
    // FIXME: Direct3D may be lost when switched to a full-screen Direct3D rendering device
    HRESULT sig = m_Direct3DDevice->Present (rcBlit, rcBlit, NULL, NULL);
    ATLASSERT (SUCCEEDED (sig));
  }

  void FillColor (WORD color) {
    DirectWrite16 dw16;

    BOOL sig = LockSurface (dw16);
    ATLASSERT (sig != FALSE);

    for (int Id = 0; Id != dw16.height; Id++) {
      uint16_t *ptr = & dw16.vptr[dw16.vpitch16 * Id];
      for (int x = 0; x != dw16.width; x++)
        ptr [x] = color;
    }
    sig = UnlockSurface ();
    ATLASSERT (sig != FALSE);

    // blitFullFromSurface ();
  }

  LRESULT OnDestroy (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 

    // CDirectWrite16WindowImpl::~CDirectWrite16WindowImpl ();
    releaseCom ();
    bHandled = FALSE;
    return 0;
  }

  LRESULT OnCreate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) { 

    ZeroMemory (& m_D3dSettings, sizeof (m_D3dSettings));
    m_D3dSettings.BackBufferFormat =  D3DFMT_X1R5G5B5;
    m_D3dSettings.SwapEffect            = D3DSWAPEFFECT_DISCARD;
    m_D3dSettings.hDeviceWindow         = *this;
    m_D3dSettings.Windowed              = TRUE;
    m_D3dSettings.BackBufferWidth       = t_dwFrameBuffer_X;
    m_D3dSettings.BackBufferHeight      = t_dwFrameBuffer_Y;
    m_D3dSettings.PresentationInterval  = D3DPRESENT_INTERVAL_IMMEDIATE; /* Disable VSync */

    m_Direct3D9 = Direct3DCreate9 (D3D_SDK_VERSION);
    ATLASSERT (m_Direct3D9 != NULL);

    // Create direct3d device 
    HRESULT sig = m_Direct3D9->CreateDevice (0,
                          D3DDEVTYPE_HAL, 
                             m_D3dSettings.hDeviceWindow, 
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING, & m_D3dSettings, & m_Direct3DDevice);
    ATLASSERT (SUCCEEDED (sig));

    // Create direct3d surface (system-memory)
    sig = m_Direct3DDevice->CreateOffscreenPlainSurface (t_dwFrameBuffer_X, 
          t_dwFrameBuffer_Y, m_D3dSettings.BackBufferFormat, D3DPOOL_SYSTEMMEM, 
        & m_Direct3DSurface, NULL);

    ATLASSERT (SUCCEEDED (sig));

    if (t_bSetScroll != FALSE)
      SetScrollSize (t_dwFrameBuffer_X, t_dwFrameBuffer_Y);

    bHandled = FALSE;
    return 0;
  }
};

template <class T, class TBase, class TWinTraits, DWORD t_dwFrameBuffer_X, DWORD t_dwFrameBuffer_Y, BOOL t_bSetScroll = FALSE, BOOL t_bHookDoPaint = FALSE> 
struct ATL_NO_VTABLE CDirectWrite16WindowREF :  
            public CDirectWrite16WindowImpl < 
                        CDirectWrite16WindowREF <T, TBase, TWinTraits, t_dwFrameBuffer_X, t_dwFrameBuffer_Y, t_bSetScroll, t_bHookDoPaint>,
                                                    TBase, TWinTraits, t_dwFrameBuffer_X, t_dwFrameBuffer_Y, t_bSetScroll, t_bHookDoPaint> 
{                            
  DECLARE_WND_CLASS(_T ("CDirectWrite16WindowREF"))

  T &attachTThis_;
  CDirectWrite16WindowREF (T &TThis_): attachTThis_ (TThis_) { m_msg_call = NULL; }

  void SetCallbackFilter (LRESULT (T::*call)(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,
		_In_ LPARAM lParam,  BOOL& bHandled)) {
      m_msg_call = call;
  }

  typedef  CDirectWrite16WindowImpl < 
                        CDirectWrite16WindowREF <T, TBase, TWinTraits, t_dwFrameBuffer_X, t_dwFrameBuffer_Y, t_bSetScroll, t_bHookDoPaint>,
                                                    TBase, TWinTraits, t_dwFrameBuffer_X, t_dwFrameBuffer_Y, t_bSetScroll, t_bHookDoPaint>  baseClass;

  BEGIN_MSG_MAP (CDirectWrite16WindowREF)
		bHandled = TRUE; 
    if (m_msg_call != NULL) 
	  	lResult = (attachTThis_.*m_msg_call) (hWnd, uMsg, wParam, lParam, bHandled); 
    else 
      bHandled = FALSE;
		if(bHandled) 
			return TRUE; 
    CHAIN_MSG_MAP(baseClass)
  END_MSG_MAP()

private:
  LRESULT (T::*m_msg_call)(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,
		_In_ LPARAM lParam,  BOOL& bHandled);
};

# define COMMAND_HANDLER2(wnd, code, func)                                    \
                                                                              \
	 if (uMsg == WM_COMMAND && wnd == (HWND)lParam && code == HIWORD(wParam))   \
	 {                                                                          \
		 bHandled = TRUE;                                                         \
		 lResult = func (HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		 if(bHandled)                                                             \
		 	return TRUE;                                                            \
	 }

#define MESSAGE_HANDLER2(wnd, msg, func) \
	if(uMsg == msg && hWnd == wnd) \
	{ \
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define DECLARE_WND_CLASS_FX(WndClassName, style, bkgnd, iconid) \
static ATL::CWndClassInfo& GetWndClassInfo() \
{ \
	static ATL::CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), style, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(bkgnd + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
    wc.m_wc.hIcon = LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (iconid));\
    wc.m_wc.hIconSm = LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (iconid));\
	return wc; \
}

#define ALONE_IF_ASSERT(expr, action) \
  if (expr) (action)
#define ALONE_IF_ELSE_ASSERT(expr, if_done, else_fone) \
  if (expr) (if_done);\
   else  (else_fone)

template <typename T>
static __forceinline T  Align2Mod (T value) {
  T Id;
  for (Id = 1; Id < value; )
     Id *= 2;
  return Id;
}

#ifndef _DEBUG0
# define DEBUG_OUTPUT(...) _tprintf (__VA_ARGS__)
#else 
# define DEBUG_OUTPUT(...) ((void)0)
#endif 

static __declspec (noinline) 
BOOL __cdecl CSetWindowText (CWindow &window, LPTSTR format, ...) {
  CString Content;
  va_list argList;
	va_start (argList, format);
	Content.FormatV (format, argList);
	va_end (argList);

  return window.SetWindowText (Content.GetBuffer ());
}

static __forceinline 
BOOL isClientRectEmpty (CWindow &window) {
  CRect client;
  window.GetClientRect (client);
  if (client.Width () <= 0
   || client.Height () <= 0)
   return TRUE;
  else 
    return FALSE;
}

static __forceinline
WORD Col_GDI32_2DirectDraw16 (COLORREF col) {
  WORD p = (col & 0xFF) >> 3;
  WORD p2 = (col & 0xFF00) >> 3 + 8;
  WORD p3 = (col & 0xFF0000) >> 3 + 8 + 8;
  return p3 | p2 << 5 | p << 10;
}

static __forceinline
COLORREF Col_DirectDraw16_2GDI32 (WORD col) {
  DWORD p = col & 0x1f;
  DWORD p2 = col >> 5 & 0x1f;
  DWORD p3 = col >> 10 & 0x1f;

  p <<= 3;
  p2 <<= 3;
  p3 <<= 3;
  return p3 | p2 << 8 | p << 16;
}

# endif  // !defined(_STD_WTL_INCLUED_HEADER_)