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

#if _MSC_VER >= 1000
# pragma once
#endif // _MSC_VER >= 1000

#if !defined (_CAGB_DISPLAY_INCLUED_HEADER_CXX_)
# define _CAGB_DISPLAY_INCLUED_HEADER_CXX_

# include <d3d9.h>
# include <ddraw.h>
# include <gl/GL.h>
# include <Windows.h>
# include <atldef.h>
# include "stdwtl.h"

// GL Unoc API.
EXTERN_C VOID __stdcall wglSwapBuffers(HDC);

// Fast ASM filter.
extern "C" void __cdecl nearest_32K (uint16_t *dstPtr, intptr_t dstPitch, 
            uint16_t *srcPtr, intptr_t srcPitch,
     intptr_t dstW, intptr_t dstH,
     intptr_t srcW, intptr_t srcH) ;
extern "C"   void __cdecl linear_32K (uint16_t *dstPtr, intptr_t dstPitch, 
            uint16_t *srcPtr, intptr_t srcPitch,
     intptr_t dstW, intptr_t dstH,
     intptr_t srcW, intptr_t srcH) ;
extern "C"   void __cdecl gray_32K (  uint16_t *dsPtr, intptr_t dsPitch, 
                          intptr_t dsW, intptr_t dsH, 
                         int mem_order/* == 0 ? D0-D15  ->R5G5BX1  : D0-D15  ->B5G5RX1 */ );
extern "C"   void __cdecl scanline_32K (uint16_t *dstPtr, intptr_t dstPitch, 
                      uint16_t *srcPtr, intptr_t srcPitch,
               intptr_t dstW, intptr_t dstH,
               intptr_t srcW, intptr_t srcH );// 0~31
extern "C"   void __cdecl linear_x32K_half (uint16_t *dstPtr, intptr_t dstPitch, 
                      uint16_t *srcPtr, intptr_t srcPitch,
               intptr_t dstW, intptr_t dstH,
               intptr_t srcW, intptr_t srcH);// 0~31
extern "C"   void __cdecl linear_x32K_quad (uint16_t *dstPtr, intptr_t dstPitch, 
                      uint16_t *srcPtr, intptr_t srcPitch,
               intptr_t dstW, intptr_t dstH,
               intptr_t srcW, intptr_t srcH);// 0~31

class CGameVideoRoot {
public:
  enum DisplayDevice {
    DisplayDevice_GdiDib,
    // DisplayDevice_DirectDraw, DirectDraw is really very troublesome.
    DisplayDevice_GL_1,
    DisplayDevice_Direct3D,
    DisplayDevice_KeepOld
  };
  enum DisplayFormat {
    DisplayFormat_RGB32,
    DisplayFormat_RGB16, // TODO:
    DisplayFormat_KeepOld
  };
  enum DisplaySoftwareFilter {
    DisplayFilter_Nearest,
    DisplayFilter_InterpolationX,
    DisplayFilter_InterpolationX_Half,
    DisplayFilter_Scanline,
    DisplayFilter_Scanline2,
    DisplayFilter_Scanline2_Half,
    DisplayFilter_Nouse,
    DisplayFilter_KeepOld
  };
  struct CGameVideoCache {
    union {
      unsigned short *pix16;
      unsigned int *pix32;
      void *pix;
    };
    int pitch;
    int width;
    int height;
    CGameVideoCache (void) {}
    CGameVideoCache (void *pix_, int pitch_, int width_, int height_) :
      pix(pix_), pitch(pitch_), width(width_), height(height_) {}
  };
  virtual BOOL get_device_hardfilter (int *FilterID) = 0;
  virtual DisplaySoftwareFilter get_device_softfilter (void) { return m_SoftFilter; }
  virtual BOOL set_device_hardfilter (int FilterID, CSize *RequesetSize, CSize *FixedSize) { return FALSE; }
  virtual BOOL set_device_softfilter (DisplaySoftwareFilter FilterID, CSize *size) { 
    if (FilterID != DisplayFilter_KeepOld)
       m_SoftFilter = FilterID;    
    return TRUE; 
  }
  virtual int blit (CGameVideoCache *vmem)  = 0;
  virtual int reset_device (CSize *Resize, DisplayFormat format) = 0; // XXX: GL Device must init in emultor thread..
  virtual int release_device (void) = 0;
  virtual void attach_window (HWND hWindow) { m_xAttachWindow.Attach (hWindow); }
  virtual DisplayDevice get_device_vector (void) = 0;
  virtual int resetDisplayCache (CSize *Resize, DisplayFormat format) = 0;
  virtual int getDisplayCache (CGameVideoCache &t) = 0;
  virtual int releaseDisplayCache (void) = 0;

  virtual CSize &getBackBufferSize (void) {
    return m_RequestSize;
  }
  virtual ~CGameVideoRoot (void) {}

  CGameVideoRoot (void):  m_DisplayScreenSize (GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN))
                    , m_SoftFilter (DisplayFilter_Nouse)       { m_SoftFilter =DisplayFilter_Nouse; }
  CWindow m_xAttachWindow;

protected :
  CSize m_DisplayScreenSize;
  CSize m_RequestSize;
  DisplaySoftwareFilter m_SoftFilter;
  DisplayFormat m_DisplayFormat;
};

class CGameVideoGDI : public CGameVideoRoot {
public:
  virtual BOOL enable_device_vsync (void) { return FALSE; }
  virtual BOOL set_device_hardfilter (int FilterID) { return FALSE; } 
  virtual BOOL get_device_hardfilter (int *FilterID) { return FALSE; }
  virtual BOOL set_device_vsync (BOOL enable) { return FALSE; }
  virtual BOOL get_device_vsync_status (void) { return FALSE; }
  virtual DisplayDevice get_device_vector (void) { return DisplayDevice_GdiDib; }

  CGameVideoGDI (void) : m_VideoBufferGDI (NULL), m_MemDC (NULL), m_GdiOldObj (NULL) {
  }
~CGameVideoGDI (void) { release_device (); }

  virtual int blit (CGameVideoCache *vmem)  {

    CRect rtSourceMapper (0, 0, vmem->width, vmem->height);
    CRect rtTargetMapper (0, 0,  m_RequestSize.cx, m_RequestSize.cy);
    CRect rcClient;
    CGameVideoCache refback_v;
    
    m_xAttachWindow.GetClientRect (rcClient);
    if ((rtTargetMapper.Width () <= 0 
                                || rtTargetMapper.Height () <= 0
                                || rtSourceMapper.Width () <= 0 
                                || rtSourceMapper.Height () <= 0 
                                || rcClient.Width () <= 0 
                                || rcClient.Height () <= 0 
                                || m_RequestSize.cx <= 0
                                || m_RequestSize.cy <= 0))
        return -1;

    getDisplayCache (refback_v);
    switch (m_SoftFilter) {
    case DisplayFilter_Nouse: {
        CClientDC dc (m_xAttachWindow);
        for (int Id = 0; Id != vmem->height; Id++)
          memcpy (& m_VideoBufferGDI[m_VideoPitchGDI/4*Id], & vmem->pix16[vmem->pitch/2*Id], vmem->width * 2);

        :: StretchBlt (dc, 0,0, rcClient.Width (), rcClient.Height (), *m_MemDC, 0, 0, vmem->width, vmem->height, SRCCOPY);
      }
      return 0;
    case DisplayFilter_Nearest:
        nearest_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_InterpolationX:
        linear_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_InterpolationX_Half:
        linear_x32K_half ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_Scanline:
      scanline_32K  ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_Scanline2:
    default:
      ATLASSERT (0);
    }
    CClientDC dc (m_xAttachWindow);
    :: BitBlt (dc, 0,0, rcClient.Width (), rcClient.Height (), *m_MemDC, 0, 0, SRCCOPY);
    return 0;
  }

  virtual int reset_device (CSize *Resize, DisplayFormat format) {
    HDC hdc;
    HBITMAP bitmap = NULL; // attach buffer .
    BITMAPINFO bmpinfos = {0};

    if (format == CGameVideoRoot::DisplayFormat_KeepOld)
      format =m_DisplayFormat;

    m_DisplayFormat = format;
    switch (format) {
    case DisplayFormat_RGB16:
      release_device ();

      hdc = m_xAttachWindow.GetDC ();
      m_MemDC = new CDC;
      m_MemDC->CreateCompatibleDC (hdc);
      
      ZeroMemory (& bmpinfos, sizeof (bmpinfos));
	    bmpinfos.bmiHeader.biSize        =  sizeof (bmpinfos);
	    bmpinfos.bmiHeader.biWidth       =  m_DisplayScreenSize.cx;
	    bmpinfos.bmiHeader.biHeight      = -(LONG)m_DisplayScreenSize.cy; // see https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376(v=vs.85).aspx
	    bmpinfos.bmiHeader.biPlanes      =  1;
	    bmpinfos.bmiHeader.biBitCount    = 16;
	    bmpinfos.bmiHeader.biCompression =  BI_RGB;

	    bitmap = :: CreateDIBSection	(*m_MemDC, & bmpinfos, 
										                DIB_RGB_COLORS, (void* *)& m_VideoBufferGDI, NULL, 0);
      ATLASSERT (bitmap != NULL);

      m_GdiOldObj = m_MemDC->SelectBitmap (bitmap);
      ATLASSERT (m_GdiOldObj != NULL && (m_GdiOldObj != HGDI_ERROR));

      m_VideoPitchGDI  = bmpinfos.bmiHeader.biWidth << 1;
      m_xAttachWindow.ReleaseDC (hdc);
      return 0;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT (FALSE);
      break;
    }
    return 0;
  }

  virtual int release_device (void) {
    if (m_GdiOldObj != NULL 
      && m_GdiOldObj != HGDI_ERROR)
    {
      if (m_MemDC != NULL) {
        // Select old object.
        HGDIOBJ bitmap = m_MemDC->SelectBitmap ((HBITMAP)m_GdiOldObj);
    if (bitmap != NULL 
      && bitmap != HGDI_ERROR)
    {
      :: DeleteObject (bitmap);
    }
          delete m_MemDC;
      }

    } else 
    {
      if (m_MemDC != NULL)
          delete m_MemDC;
      }
      m_GdiOldObj = NULL;
  m_MemDC = NULL;

  return 0;
    }

  virtual int resetDisplayCache (CSize *Resize, DisplayFormat format)  {
    if (Resize != NULL) {
      if (Resize->cx >= 0)
        m_RequestSize.cx = Resize->cx;
      if (Resize->cy >= 0)
        m_RequestSize.cy = Resize->cy;
    }
    if (format != DisplayFormat_KeepOld
      || format != m_DisplayFormat) {
      reset_device (Resize, format);
      if (Resize != NULL) {
        if (Resize->cx >= 0)
          m_RequestSize.cx = Resize->cx;
        if (Resize->cy >= 0)
          m_RequestSize.cy = Resize->cy;
      }
    }
    return 0;
  }

  virtual int getDisplayCache (CGameVideoCache &t) {
    switch (m_DisplayFormat) {
    case DisplayFormat_RGB16:
      t.pix = m_VideoBufferGDI;
      t.pitch = m_VideoPitchGDI;
      t.width =m_RequestSize.cx;
      t.height = m_RequestSize.cy;
      return 0;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT (FALSE);
      break;
    }
    return 0;
  }

  virtual int releaseDisplayCache (void) {
    // no unlock action in GDI.
    return 0;
  }

  DWORD *m_VideoBufferGDI;
  DWORD m_VideoPitchGDI;
  
  CDC *m_MemDC; 
  HGDIOBJ m_GdiOldObj;
};

class CGameVideoGL_1: public CGameVideoRoot {
#ifndef GL_UNSIGNED_SHORT_5_5_5_1
#  define GL_UNSIGNED_SHORT_5_5_5_1  0x8034
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#  define GL_UNSIGNED_SHORT_1_5_5_5_REV  0x8366
#endif
#ifndef GL_BGRA
# define GL_BGRA		0x80E1
#endif
public:
  virtual BOOL set_device_hardfilter (int FilterID) { return FALSE; } 
  virtual BOOL get_device_hardfilter (int *FilterID) { return FALSE; }
  virtual DisplayDevice get_device_vector (void) { return DisplayDevice_GL_1; }

  CGameVideoGL_1 (void) : m_GL_1_handle (NULL), m_VideoBufferGL (NULL) {
  }
~CGameVideoGL_1 (void) { release_device (); }

  virtual int blit (CGameVideoCache *vmem)  {

    CRect rtSourceMapper (0, 0, vmem->width, vmem->height);
    CRect rtTargetMapper (0, 0,  m_RequestSize.cx, m_RequestSize.cy);
    CRect rcClient;
    CGameVideoCache refback_v;
    
    m_xAttachWindow.GetClientRect (rcClient);
    if ((rtTargetMapper.Width () <= 0 
                                || rtTargetMapper.Height () <= 0
                                || rtSourceMapper.Width () <= 0 
                                || rtSourceMapper.Height () <= 0 
                                || rcClient.Width () <= 0 
                                || rcClient.Height () <= 0 
                                || m_RequestSize.cx <= 0
                                || m_RequestSize.cy <= 0))
        return -1;

    getDisplayCache (refback_v);
    refback_v.pitch = rcClient.Width () * 2;
    refback_v.width = rcClient.Width ();
    refback_v.height = rcClient.Height ();
    if (refback_v.pitch & 3)
      refback_v.pitch = (refback_v.pitch & -4) + 4;

    switch (m_SoftFilter) {
    case DisplayFilter_Nearest:
    case DisplayFilter_Nouse:
        nearest_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_InterpolationX:
        linear_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_InterpolationX_Half:
        linear_x32K_half ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_Scanline:
      scanline_32K  ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
            & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
      break;
    case DisplayFilter_Scanline2:
    default:
      ATLASSERT (0);
    }
    CClientDC dc (m_xAttachWindow);

	  glRasterPos2i (-1, 1);
	  glViewport  (0, 0, rcClient.right, rcClient.bottom);

    glDrawPixels (rcClient.Width (), rcClient.Height (),
					                    GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, m_VideoBufferGL );
	  glPixelZoom  ( (float)1.0, - (float)1.0	);
    wglSwapBuffers (dc);
    return 0;
  }

  virtual int reset_device (CSize *Resize, DisplayFormat format) {
    if (format == DisplayFormat_KeepOld)
      format = m_DisplayFormat;
    m_DisplayFormat = format;
    switch (format) {
    case DisplayFormat_RGB16: {
      release_device ();

      CClientDC dc (m_xAttachWindow);

	    PIXELFORMATDESCRIPTOR pfd =  {  sizeof( PIXELFORMATDESCRIPTOR ), 
									    1, 
									    PFD_DRAW_TO_WINDOW | 
									    PFD_SUPPORT_OPENGL
									    | PFD_DOUBLEBUFFER, 
									    PFD_TYPE_RGBA, 
									    16, 
									    0, 0, 0, 0, 0, 0, 
									    0, 0, 0, 0, 0, 0, 0,
									    0, 0, 0, PFD_MAIN_PLANE, 
									    0, 0, 0, 0 	
								    }; 	
    
      BOOL ret = SetPixelFormat (dc, dc.ChoosePixelFormat (& pfd), &pfd);
      ATLASSERT (ret != FALSE);

      m_GL_1_handle = dc.wglCreateContext ();
      ATLASSERT (m_GL_1_handle != NULL);

      ret = dc.wglMakeCurrent (m_GL_1_handle) ;
      ATLASSERT (ret != FALSE);

      m_VideoPitchGL = Align2Mod<DWORD> ( GetSystemMetrics (SM_CXSCREEN) * 2);
      m_VideoBufferGL = (DWORD *)_aligned_malloc (m_VideoPitchGL * GetSystemMetrics (SM_CYSCREEN), 64);
      }
      return 0;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT (FALSE);
      break;
    }
    return 0;
  }

  virtual int release_device (void) {
      if (m_GL_1_handle != NULL) {
        BOOL ret = wglMakeCurrent (NULL, NULL) ;
        ATLASSERT (ret != FALSE);

        ret = wglDeleteContext (m_GL_1_handle);
        ATLASSERT (ret != FALSE);

        m_GL_1_handle = NULL;
      }
      if (m_VideoBufferGL != NULL)
        _aligned_free (m_VideoBufferGL);
      m_VideoBufferGL = NULL;
        return 0;
    }

  virtual int resetDisplayCache (CSize *Resize, DisplayFormat format)  {
    if (Resize != NULL) {
      if (Resize->cx >= 0)
        m_RequestSize.cx = Resize->cx;
      if (Resize->cy >= 0)
        m_RequestSize.cy = Resize->cy;
    }
    if (format != DisplayFormat_KeepOld
      || format != m_DisplayFormat) {
      reset_device (Resize, format);
      if (Resize != NULL) {
        if (Resize->cx >= 0)
          m_RequestSize.cx = Resize->cx;
        if (Resize->cy >= 0)
          m_RequestSize.cy = Resize->cy;
      }
    }
    return 0;
  }

  virtual int getDisplayCache (CGameVideoCache &t) {
    switch (m_DisplayFormat) {
    case DisplayFormat_RGB16:
      t.pix = m_VideoBufferGL;
      t.pitch = m_VideoPitchGL;
      t.width =m_RequestSize.cx;
      t.height = m_RequestSize.cy;
      return 0;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT (FALSE);
      break;
    }
    return 0;
  }

  virtual int releaseDisplayCache (void) {
    // no unlock action in GL 1.
    return 0;
  }

  DWORD *m_VideoBufferGL;
  DWORD m_VideoPitchGL;
  HGLRC m_GL_1_handle;
};

class CGameVideoDirect3D : public CGameVideoRoot {
public:
  CGameVideoDirect3D (void) : m_D3d9 (NULL), m_DDevice9 (NULL),
                                       m_D3d9Surface (NULL), m_D3d9SurfaceVRamCaps (NULL), m_iHardFilter (iHardFilter_fast_disable_softfilter) {
  }
~CGameVideoDirect3D (void) { release_device (); }

  virtual BOOL set_device_hardfilter (int FilterID, CSize *RequesetSize, CSize *FixedSize) { 
    m_iHardFilter = FilterID;
    if (FilterID == iHardFilter_stdandenablle_softfilter) {
      reset_device (RequesetSize, DisplayFormat_KeepOld);
    } else if (FilterID == iHardFilter_fast_disable_softfilter) {
      reset_device2 (FixedSize, DisplayFormat_KeepOld);
    } else if (FilterID == iHardFilter_biliner_disable_softfilter ) {
      reset_device (RequesetSize, DisplayFormat_KeepOld);
    } else {
      ATLASSERT (FALSE);
    }
    return TRUE;
  }

  virtual BOOL set_device_softfilter (DisplaySoftwareFilter FilterID, CSize *size) {
    if (m_iHardFilter == iHardFilter_stdandenablle_softfilter) {
      DisplaySoftwareFilter oldfilter = m_SoftFilter;
      if (FilterID != DisplayFilter_KeepOld)
        m_SoftFilter = FilterID;
      reset_device (size, DisplayFormat_KeepOld);
      return TRUE;
    }
    return TRUE;
  }

  virtual BOOL get_device_hardfilter (int *FilterID) { return m_iHardFilter; }
  virtual DisplayDevice get_device_vector (void) { return DisplayDevice_Direct3D; }

  virtual int blit (CGameVideoCache *vmem)  {

    CRect rtSourceMapper (0, 0, vmem->width, vmem->height);
    CRect rtTargetMapper (0, 0,  m_RequestSize.cx, m_RequestSize.cy);
    CRect rcClient;
    CGameVideoCache refback_v;
    
    m_xAttachWindow.GetClientRect (rcClient);
    if ((rtTargetMapper.Width () <= 0 
                                || rtTargetMapper.Height () <= 0
                                || rtSourceMapper.Width () <= 0 
                                || rtSourceMapper.Height () <= 0 
                                || rcClient.Width () <= 0 
                                || rcClient.Height () <= 0 
                                || m_RequestSize.cx <= 0
                                || m_RequestSize.cy <= 0))
        return -1;

    switch (m_DisplayFormat) {
    case DisplayFormat_RGB16: {
      if (m_iHardFilter == iHardFilter_fast_disable_softfilter) {
        HRESULT sig;
        LPDIRECT3DSURFACE9 d3d9dev_mainSurface;
        D3DLOCKED_RECT directwrite_desc;
        
        const DWORD lockflg = D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT | D3DLOCK_NOOVERWRITE;

        sig = m_D3d9Surface->LockRect (& directwrite_desc, NULL, lockflg);
        ATLASSERT (SUCCEEDED (sig));

        // Update surface to back .

    #ifndef _WIN64
        for (int Id = 0; Id != vmem->height; Id++)
          memcpy (& ((char *)directwrite_desc.pBits)[directwrite_desc.Pitch *Id], & vmem->pix16        [vmem->pitch/2*Id], vmem->width << 1);
    #else 
        for (int Id = 0; Id != vmem->height; Id++)
          memcpy (& ((char *)directwrite_desc.pBits)[directwrite_desc.Pitch *Id], & vmem->pix16        [vmem->pitch/2*Id], vmem->width << 1);
    #endif 

        sig = m_D3d9Surface->UnlockRect ();
        ATLASSERT (SUCCEEDED (sig));

        sig = m_DDevice9->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, & d3d9dev_mainSurface);
        ATLASSERT (SUCCEEDED (sig));
        // Update surface 
        sig = m_DDevice9->UpdateSurface (m_D3d9Surface, & CRect (0, 0, vmem->width, vmem->height), d3d9dev_mainSurface, & CPoint (0, 0));
        ATLASSERT (SUCCEEDED (sig));
        sig = d3d9dev_mainSurface->Release ();
        ATLASSERT (SUCCEEDED (sig));
        sig = m_DDevice9->Present (NULL, NULL, NULL, NULL);
        ATLASSERT (SUCCEEDED (sig));
        return 0;

      } else if (m_iHardFilter == iHardFilter_biliner_disable_softfilter) {
        LPDIRECT3DSURFACE9 d3d9SurfaceTemp;
        D3DLOCKED_RECT directwrite_desc;
        const DWORD lockflg = D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT | D3DLOCK_NOOVERWRITE;
        HRESULT sig = m_D3d9Surface->LockRect (& directwrite_desc, NULL, lockflg);
        ATLASSERT (SUCCEEDED (sig));

        // Update surface to back .

    #ifndef _WIN64
        for (int Id = 0; Id != vmem->height; Id++)
          memcpy (& ((char *)directwrite_desc.pBits)[directwrite_desc.Pitch *Id], & vmem->pix16        [vmem->pitch/2*Id], vmem->width << 1);
    #else 
        for (int Id = 0; Id != vmem->height; Id++)
          memcpy (& ((char *)directwrite_desc.pBits)[directwrite_desc.Pitch *Id], & vmem->pix16        [vmem->pitch/2*Id], vmem->width << 1);
    #endif 

        sig = m_D3d9Surface->UnlockRect ();
        ATLASSERT (SUCCEEDED (sig));

        sig = m_DDevice9->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, & d3d9SurfaceTemp);
        ATLASSERT (SUCCEEDED (sig));
        // Update surface 
        sig = m_DDevice9->UpdateSurface (m_D3d9Surface, & CRect (0, 0,vmem->width, vmem->height), m_D3d9SurfaceVRamCaps, & CPoint (0, 0));
        ATLASSERT (SUCCEEDED (sig));

        sig = m_DDevice9->StretchRect (m_D3d9SurfaceVRamCaps, & CRect (0, 0,vmem->width, vmem->height), d3d9SurfaceTemp, 
                                            &CRect (0, 0, rcClient.Width (), rcClient.Height ()), D3DTEXF_LINEAR);
        sig = d3d9SurfaceTemp->Release ();
        ATLASSERT (SUCCEEDED (sig));
        
        sig = m_DDevice9->Present (&CRect (0, 0, rcClient.Width (), rcClient.Height ()), NULL, NULL, NULL);
        ATLASSERT (SUCCEEDED (sig));
        return 0;

      } else if (m_iHardFilter == iHardFilter_stdandenablle_softfilter) {
        const DWORD lockflg = D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT | D3DLOCK_NOOVERWRITE;
        D3DLOCKED_RECT directwrite_desc;
        HRESULT sig = m_D3d9Surface->LockRect (& directwrite_desc, NULL, lockflg);
        ATLASSERT (SUCCEEDED (sig));
        refback_v.pix = directwrite_desc.pBits;
        refback_v.pitch = directwrite_desc.Pitch;

        switch (m_SoftFilter) {
        case DisplayFilter_Nearest:
        case DisplayFilter_Nouse:
            nearest_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
                & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
          break;
        case DisplayFilter_InterpolationX:
            linear_32K ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
                & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
          break;
        case DisplayFilter_InterpolationX_Half:
            linear_x32K_half ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
                & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
          break;
        case DisplayFilter_Scanline:
          scanline_32K  ((uint16_t *)& refback_v.pix16[refback_v.pitch/ 2*rtTargetMapper.top + rtTargetMapper.left], refback_v.pitch,
                & vmem->pix16[vmem->pitch/ 2*rtSourceMapper.top + rtSourceMapper.left], vmem->pitch, rtTargetMapper.Width(), rtTargetMapper.Height (), rtSourceMapper.Width(), rtSourceMapper.Height ());
          break;
        case DisplayFilter_Scanline2:
        default:
          ATLASSERT (0);
        }
        sig = m_D3d9Surface->UnlockRect ();
        ATLASSERT (SUCCEEDED (sig));
        // Get deivc buffer 
        LPDIRECT3DSURFACE9 d3d9SurfaceTemp;
        sig = m_DDevice9->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, & d3d9SurfaceTemp);
        ATLASSERT (SUCCEEDED (sig));
        // Update surface 
        sig = m_DDevice9->UpdateSurface (m_D3d9Surface, & rtTargetMapper, d3d9SurfaceTemp, & rtTargetMapper.TopLeft ());
        ATLASSERT (SUCCEEDED (sig));
        sig = d3d9SurfaceTemp->Release ();
        ATLASSERT (SUCCEEDED (sig));
        sig = m_DDevice9->Present (& rtTargetMapper, NULL, NULL, NULL);
        ATLASSERT (SUCCEEDED (sig));
        return 0;
      } else {
        ATLASSERT (FALSE);
      }
      }
      break;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT( FALSE);
      break;
    }
    return 0;
  }

  virtual int reset_device (CSize *Resize, DisplayFormat format) {

    if (format == DisplayFormat_KeepOld)
      format =m_DisplayFormat;

    ZeroMemory (& m_d3dpp, sizeof (m_d3dpp));
    m_d3dpp.BackBufferFormat =  format == DisplayFormat_RGB32 ? D3DFMT_X8R8G8B8 : D3DFMT_X1R5G5B5;
    m_d3dpp.SwapEffect            = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.hDeviceWindow         = m_xAttachWindow;
    m_d3dpp.Windowed              = TRUE;
    m_d3dpp.BackBufferWidth       = m_DisplayScreenSize.cx;
    m_d3dpp.BackBufferHeight      = m_DisplayScreenSize.cy;
    m_d3dpp.PresentationInterval  = D3DPRESENT_INTERVAL_IMMEDIATE; /* Disable VSync */

    // Release old direct3d object 
    release_device ();
    m_RequestSize = *Resize;
    m_DisplayFormat = format;

    m_D3d9 = Direct3DCreate9 (D3D_SDK_VERSION);
    ATLASSERT (m_D3d9 != NULL);

    // Create direct3d device 
    HRESULT sig = m_D3d9->CreateDevice (0,
                          D3DDEVTYPE_HAL, 
                             m_d3dpp.hDeviceWindow, 
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING, & m_d3dpp, & m_DDevice9);
    ATLASSERT (SUCCEEDED (sig));

    // Create direct3d surface (system-memory)
    sig = m_DDevice9->CreateOffscreenPlainSurface (m_DisplayScreenSize.cx, 
          m_DisplayScreenSize.cy, m_d3dpp.BackBufferFormat, D3DPOOL_SYSTEMMEM, 
        & m_D3d9Surface, NULL);

    ATLASSERT (SUCCEEDED (sig));

    // Create direct3d surface (video memory)
    sig = m_DDevice9->CreateOffscreenPlainSurface (m_DisplayScreenSize.cx, 
          m_DisplayScreenSize.cy, m_d3dpp.BackBufferFormat, D3DPOOL_DEFAULT, 
        & m_D3d9SurfaceVRamCaps, NULL);

    ATLASSERT (SUCCEEDED (sig));

    return 0;
  }

  virtual int reset_device2 (CSize *fixedsize, DisplayFormat format) {
    if (format == DisplayFormat_KeepOld)
      format =m_DisplayFormat;
    else {}

    ZeroMemory (& m_d3dpp, sizeof (m_d3dpp));
    m_d3dpp.BackBufferFormat =  format == DisplayFormat_RGB32 ? D3DFMT_X8R8G8B8 : D3DFMT_X1R5G5B5;
    m_d3dpp.SwapEffect            = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.hDeviceWindow         = m_xAttachWindow;
    m_d3dpp.Windowed              = TRUE;
    m_d3dpp.BackBufferWidth       = fixedsize->cx;
    m_d3dpp.BackBufferHeight      = fixedsize->cy;
    m_d3dpp.PresentationInterval  = D3DPRESENT_INTERVAL_IMMEDIATE; /* Disable VSync */

    // Release old direct3d object 
    release_device ();
    m_DisplayFormat = format;

    m_D3d9 = Direct3DCreate9 (D3D_SDK_VERSION);
    ATLASSERT (m_D3d9 != NULL);

    // Create direct3d device 
    HRESULT sig = m_D3d9->CreateDevice (0,
                          D3DDEVTYPE_HAL, 
                             m_d3dpp.hDeviceWindow, 
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING, & m_d3dpp, & m_DDevice9);
    ATLASSERT (SUCCEEDED (sig));

    // Create direct3d surface (system-memory)
    sig = m_DDevice9->CreateOffscreenPlainSurface (fixedsize->cx, 
          fixedsize->cy, m_d3dpp.BackBufferFormat, D3DPOOL_SYSTEMMEM, 
        & m_D3d9Surface, NULL);

    ATLASSERT (SUCCEEDED (sig));
    return 0;
  }
  virtual int release_device (void) {
    if (m_D3d9Surface != NULL)
      m_D3d9Surface->Release ();
    if (m_D3d9SurfaceVRamCaps != NULL)
      m_D3d9SurfaceVRamCaps->Release ();
    if (m_DDevice9 != NULL)
      m_DDevice9->Release ();
    if (m_D3d9 != NULL)
      m_D3d9->Release ();

    m_D3d9Surface = NULL;
    m_D3d9SurfaceVRamCaps = NULL;
    m_DDevice9 = NULL;
    m_D3d9 = NULL;

    return 0;
  }

  virtual int resetDisplayCache (CSize *Resize, DisplayFormat format)  {
    if (Resize != NULL) {
      if (Resize->cx >= 0)
        m_RequestSize.cx = Resize->cx;
      if (Resize->cy >= 0)
        m_RequestSize.cy = Resize->cy;
    }
    if (m_iHardFilter == iHardFilter_stdandenablle_softfilter) {
      if (format != DisplayFormat_KeepOld
        || format != m_DisplayFormat) {
        reset_device (Resize, format);
        if (Resize != NULL) {
          if (Resize->cx >= 0)
            m_RequestSize.cx = Resize->cx;
          if (Resize->cy >= 0)
            m_RequestSize.cy = Resize->cy;
        }
      }
    }
    return 0;
  }

  virtual int getDisplayCache (CGameVideoCache &t) {
    switch (m_DisplayFormat) {
    case DisplayFormat_RGB16:
      t.width =m_RequestSize.cx;
      t.height = m_RequestSize.cy;
      return 0;
    case DisplayFormat_RGB32:
    default:
      ATLASSERT (FALSE);
      break;
    }
    return 0;
  }

  virtual int releaseDisplayCache (void) {
    return 0;
  }

  LPDIRECT3D9 m_D3d9;
  LPDIRECT3DDEVICE9 m_DDevice9;
  LPDIRECT3DSURFACE9 m_D3d9Surface;
  LPDIRECT3DSURFACE9 m_D3d9SurfaceVRamCaps;

  D3DLOCKED_RECT m_D3d9SurfaceDesc;
  D3DPRESENT_PARAMETERS m_d3dpp;

  static const INT iHardFilter_stdandenablle_softfilter = 0;
  static const INT iHardFilter_fast_disable_softfilter= 1;
  static const INT iHardFilter_biliner_disable_softfilter = 2;

  INT m_iHardFilter; // 0: normal 1:fast 2:bilinear
};

#endif 