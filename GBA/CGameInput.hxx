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

#if !defined (_CDIRECTINPUT_INCLUED_HEADER_CXX_)
# define _CDIRECTINPUT_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include <dinput.h>

class CGameInput {
public:
  typedef int JP_MASK;
  static const int JP_MASK_PRESS = 1;
  static const int JP_MASK_LAST = 2; 
  static const int JP_MASK_ALL = JP_MASK_PRESS | JP_MASK_LAST;
     
  // play 
  enum PLAY_TYPE {
    FOREGROUND = 0,
    BACKGROUND
  };

  __forceinline
  JP_MASK  getMask (BYTE DIK_idx)  {
    JP_MASK t = 0;
    int c = DIK_idx + m_keyswi;
    int s = c ^ 256;
    
    if ( ( m_keybuf[s] & 0x80) != 0) {           //   cur press.
      t |= JP_MASK_PRESS;
      if ( ( m_keybuf[c] & 0x80) != 0)           //   old press.
        t |= JP_MASK_LAST;                             
      else  ;
    } else 
      if ( ( m_keybuf[c] & 0x80) == 0)           //    cur not press 
        t |= JP_MASK_LAST;                               
      else  ;
    return t;
  }
  
  __forceinline
  BOOL  isPressedFirst (BYTE DIK_idx)  {
    return ( (getMask (DIK_idx) & (JP_MASK_LAST | JP_MASK_PRESS)) == JP_MASK_PRESS) 
      ? TRUE 
      : FALSE;
  }
  __forceinline
  BOOL  isUpFirst (BYTE DIK_idx)  {
    return ( (getMask (DIK_idx) & (JP_MASK_LAST | JP_MASK_PRESS)) == 0) 
      ? TRUE 
      : FALSE;
  }

  __forceinline
  void poll (void)  {
    int s = m_keyswi;
    m_keyswi ^= 256;
    
    if ( FAILED( m_input->GetDeviceState (256, & m_keybuf[s])) ) 
      if (FAILED (m_input->Acquire ()))                   //  first   try   Acquire device.
        ZeroMemory (& m_keybuf[s], 256);
      else 
        if ( FAILED( m_input->GetDeviceState (256, & m_keybuf[s])) ) 
          ZeroMemory (& m_keybuf[s], 256);        //  nodone, clear buffer.
        else  ;
    else ;
  }

  void resetPlayType (HWND hWindow, enum PLAY_TYPE t = FOREGROUND)  {
    ATLASSERT (t == FOREGROUND || (t = BACKGROUND));

    Destroy ();

    m_keybuf = (PBYTE) _aligned_malloc (512, 64);
    ZeroMemory (m_keybuf, sizeof (m_keybuf));
    m_keyswi = 0;

    if (m_DirectInput8 == NULL) {
      HRESULT  sig = DirectInput8Create (GetModuleHandle (NULL), 
                             DIRECTINPUT_VERSION, 
                           IID_IDirectInput8, 
                              (void **)& m_DirectInput8, NULL);
      ATLASSERT (SUCCEEDED (sig));
    }

    HRESULT sig = m_DirectInput8->CreateDevice ( GUID_SysKeyboard, & m_input, NULL);   
    ATLASSERT (SUCCEEDED (sig));

    BOOL bOldisVisual  = !! IsWindowVisible (hWindow);
    if  (bOldisVisual == FALSE)  
      ShowWindow (hWindow, SW_SHOWNORMAL);  
    else {}

    // Set   SetCooperativeLevel 
    sig = m_input->SetCooperativeLevel (hWindow, (t == FOREGROUND ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | ((t == FOREGROUND )?  DISCL_FOREGROUND : DISCL_BACKGROUND));  
    ATLASSERT (SUCCEEDED (sig));

    // Set SetDataFormat KeyBoard.
    sig = m_input->SetDataFormat ( (LPDIDATAFORMAT) & c_dfDIKeyboard);
    ATLASSERT (SUCCEEDED (sig));

    if  (bOldisVisual == FALSE)  // reset old
      ShowWindow (hWindow, SW_HIDE);
  }
  // construct  
  CGameInput (void) : m_keybuf (NULL), m_input (NULL),  m_DirectInput8 (NULL)  { }
  
  // deconstruct
  ~CGameInput (void) {
    Destroy ();
  }; 

private:
  void Destroy (void) {
    if (m_keybuf != NULL) {
      _aligned_free (m_keybuf);
      m_keybuf = NULL;
    }
    if ( m_input != NULL) {
      HRESULT sig= m_input->Unacquire ();
      ATLASSERT (SUCCEEDED (sig));
      release_com<IDirectInputDevice8>(m_input);
    }
    if (m_DirectInput8 != NULL) {
      release_com<IDirectInput8>(m_DirectInput8);
    }
  }

  PBYTE m_keybuf; 
  WORD m_keyswi; 
  IDirectInput8 *m_DirectInput8;
  IDirectInputDevice8 *m_input;
};
# endif 