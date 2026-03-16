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

#if !defined (_CKEYSETTINGS_INCLUED_HEADER_CXX_)
# define _CKEYSETTINGS_INCLUED_HEADER_CXX_

# include <atldef.h>
# include <dinput.h>
# include <map>
# include "stdwtl.h"
# include "..\resource.h"

// Unlike MFC, WTL 's CDockingFrameImpl cannot intercept messages using PreTranslateMessage
struct CKeySettings : public CDialogImpl<CKeySettings>
{
  enum { IDD = IDD_KEY_SETTINGS };

  BEGIN_MSG_MAP(CKeySettings)
    MESSAGE_HANDLER (WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER (IDOK, OnOkOrCancel)
    COMMAND_ID_HANDLER (IDCANCEL, OnOkOrCancel)
    COMMAND_ID_HANDLER (IDST_KEYSET_UP, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_DOWN, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_LEFT, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_RIGHT, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_L, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_R, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_A, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_B, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_SELECT, OnKeyMapper_Fire)
    COMMAND_ID_HANDLER (IDST_KEYSET_START, OnKeyMapper_Fire)
  END_MSG_MAP()
  
  ~CKeySettings (void) {}
  CKeySettings (BYTE *DIK_set) {
    
    m_keySettings = DIK_set;
    m_QuitConfirm = FALSE;

    m_map_DIK.clear ();
    m_map_DIK.insert (std::make_pair (DIK_ESCAPE, _T ("escape")));
    m_map_DIK.insert (std::make_pair (DIK_1, _T ("1")));
    m_map_DIK.insert (std::make_pair (DIK_2, _T ("2")));
    m_map_DIK.insert (std::make_pair (DIK_3, _T ("3")));
    m_map_DIK.insert (std::make_pair (DIK_4, _T ("4")));
    m_map_DIK.insert (std::make_pair (DIK_5, _T ("5")));
    m_map_DIK.insert (std::make_pair (DIK_6, _T ("6")));
    m_map_DIK.insert (std::make_pair (DIK_7, _T ("7")));
    m_map_DIK.insert (std::make_pair (DIK_8, _T ("8")));
    m_map_DIK.insert (std::make_pair (DIK_9, _T ("9")));
    m_map_DIK.insert (std::make_pair (DIK_0, _T ("0")));
    m_map_DIK.insert (std::make_pair (DIK_MINUS, _T ("minus")));
    m_map_DIK.insert (std::make_pair (DIK_EQUALS, _T ("equals")));
    m_map_DIK.insert (std::make_pair (DIK_BACK, _T ("backspace")));
    m_map_DIK.insert (std::make_pair (DIK_TAB, _T ("tab")));
    m_map_DIK.insert (std::make_pair (DIK_Q, _T ("q")));
    m_map_DIK.insert (std::make_pair (DIK_W, _T ("w")));
    m_map_DIK.insert (std::make_pair (DIK_E, _T ("e")));
    m_map_DIK.insert (std::make_pair (DIK_R, _T ("r")));
    m_map_DIK.insert (std::make_pair (DIK_T, _T ("t")));
    m_map_DIK.insert (std::make_pair (DIK_Y, _T ("y")));
    m_map_DIK.insert (std::make_pair (DIK_U, _T ("u")));
    m_map_DIK.insert (std::make_pair (DIK_I, _T ("i")));
    m_map_DIK.insert (std::make_pair (DIK_O, _T ("o")));
    m_map_DIK.insert (std::make_pair (DIK_P, _T ("p")));
    m_map_DIK.insert (std::make_pair (DIK_LBRACKET, _T ("l-bracket")));
    m_map_DIK.insert (std::make_pair (DIK_RBRACKET, _T ("r-bracket")));
    m_map_DIK.insert (std::make_pair (DIK_RETURN, _T ("enter")));
    m_map_DIK.insert (std::make_pair (DIK_LCONTROL, _T ("l-control")));
    m_map_DIK.insert (std::make_pair (DIK_A, _T ("a")));
    m_map_DIK.insert (std::make_pair (DIK_S, _T ("s")));
    m_map_DIK.insert (std::make_pair (DIK_D, _T ("d")));
    m_map_DIK.insert (std::make_pair (DIK_F, _T ("f")));
    m_map_DIK.insert (std::make_pair (DIK_G, _T ("g")));
    m_map_DIK.insert (std::make_pair (DIK_H, _T ("h")));
    m_map_DIK.insert (std::make_pair (DIK_J, _T ("j")));
    m_map_DIK.insert (std::make_pair (DIK_K, _T ("k")));
    m_map_DIK.insert (std::make_pair (DIK_L, _T ("l")));
    m_map_DIK.insert (std::make_pair (DIK_SEMICOLON, _T ("semicolon")));
    m_map_DIK.insert (std::make_pair (DIK_APOSTROPHE, _T ("apostrophe")));
    m_map_DIK.insert (std::make_pair (DIK_GRAVE, _T ("grave")));
    m_map_DIK.insert (std::make_pair (DIK_LSHIFT, _T ("l-shift")));
    m_map_DIK.insert (std::make_pair (DIK_BACKSLASH, _T ("backslash")));
    m_map_DIK.insert (std::make_pair (DIK_Z, _T ("z")));
    m_map_DIK.insert (std::make_pair (DIK_X, _T ("x")));
    m_map_DIK.insert (std::make_pair (DIK_C, _T ("c")));
    m_map_DIK.insert (std::make_pair (DIK_V, _T ("v")));
    m_map_DIK.insert (std::make_pair (DIK_B, _T ("b")));
    m_map_DIK.insert (std::make_pair (DIK_N, _T ("n")));
    m_map_DIK.insert (std::make_pair (DIK_M, _T ("m")));
    m_map_DIK.insert (std::make_pair (DIK_COMMA, _T ("comma")));
    m_map_DIK.insert (std::make_pair (DIK_PERIOD, _T ("period")));
    m_map_DIK.insert (std::make_pair (DIK_SLASH, _T ("slash")));
    m_map_DIK.insert (std::make_pair (DIK_RSHIFT, _T ("r-shift")));
    m_map_DIK.insert (std::make_pair (DIK_MULTIPLY, _T ("multiply")));
    m_map_DIK.insert (std::make_pair (DIK_LMENU, _T ("l-alt")));
    m_map_DIK.insert (std::make_pair (DIK_SPACE, _T ("space")));
    m_map_DIK.insert (std::make_pair (DIK_CAPITAL, _T ("capital")));
    m_map_DIK.insert (std::make_pair (DIK_F1, _T ("f1")));
    m_map_DIK.insert (std::make_pair (DIK_F2, _T ("f2")));
    m_map_DIK.insert (std::make_pair (DIK_F3, _T ("f3")));
    m_map_DIK.insert (std::make_pair (DIK_F4, _T ("f4")));
    m_map_DIK.insert (std::make_pair (DIK_F5, _T ("f5")));
    m_map_DIK.insert (std::make_pair (DIK_F6, _T ("f6")));
    m_map_DIK.insert (std::make_pair (DIK_F7, _T ("f7")));
    m_map_DIK.insert (std::make_pair (DIK_F8, _T ("f8")));
    m_map_DIK.insert (std::make_pair (DIK_F9, _T ("f9")));
    m_map_DIK.insert (std::make_pair (DIK_F10, _T ("f10")));
    m_map_DIK.insert (std::make_pair (DIK_NUMLOCK, _T ("numlock")));
    m_map_DIK.insert (std::make_pair (DIK_SCROLL, _T ("scroll")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD7, _T ("numpad-7")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD8, _T ("numpad-8")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD9, _T ("numpad-9")));
    m_map_DIK.insert (std::make_pair (DIK_SUBTRACT, _T ("numpad-subtract")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD4, _T ("numpad-4")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD5, _T ("numpad-5")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD6, _T ("numpad-6")));
    m_map_DIK.insert (std::make_pair (DIK_ADD, _T ("numpad-add")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD1, _T ("numpad-1")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD2, _T ("numpad-2")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD3, _T ("numpad-3")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPAD0, _T ("numpad-0")));
    m_map_DIK.insert (std::make_pair (DIK_DECIMAL, _T ("numpad-decimal")));
    m_map_DIK.insert (std::make_pair (DIK_OEM_102, _T ("oem-102")));
    m_map_DIK.insert (std::make_pair (DIK_F11, _T ("f11")));
    m_map_DIK.insert (std::make_pair (DIK_F12, _T ("f12")));
    m_map_DIK.insert (std::make_pair (DIK_F13, _T ("f13")));
    m_map_DIK.insert (std::make_pair (DIK_F14, _T ("f14")));
    m_map_DIK.insert (std::make_pair (DIK_F15, _T ("f15")));
    m_map_DIK.insert (std::make_pair (DIK_KANA, _T ("kana?")));
    m_map_DIK.insert (std::make_pair (DIK_ABNT_C1, _T ("abnt_c1?")));
    m_map_DIK.insert (std::make_pair (DIK_CONVERT, _T ("convert?")));
    m_map_DIK.insert (std::make_pair (DIK_NOCONVERT, _T ("no-convert?")));
    m_map_DIK.insert (std::make_pair (DIK_YEN, _T ("yen?")));
    m_map_DIK.insert (std::make_pair (DIK_ABNT_C2, _T ("abnt_c2?")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPADEQUALS, _T ("numpad-equals?")));
    m_map_DIK.insert (std::make_pair (DIK_PREVTRACK, _T ("previous Track?")));
    m_map_DIK.insert (std::make_pair (DIK_AT, _T ("at?")));
    m_map_DIK.insert (std::make_pair (DIK_COLON, _T ("colon?")));
    m_map_DIK.insert (std::make_pair (DIK_UNDERLINE, _T ("underline?")));
    m_map_DIK.insert (std::make_pair (DIK_KANJI, _T ("kanji?")));
    m_map_DIK.insert (std::make_pair (DIK_STOP, _T ("stop?")));
    m_map_DIK.insert (std::make_pair (DIK_AX, _T ("ax?")));
    m_map_DIK.insert (std::make_pair (DIK_UNLABELED, _T ("unlabeled?")));
    m_map_DIK.insert (std::make_pair (DIK_NEXTTRACK, _T ("next-track?")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPADENTER, _T ("numpad-enter")));
    m_map_DIK.insert (std::make_pair (DIK_RCONTROL, _T ("r-control")));
    m_map_DIK.insert (std::make_pair (DIK_MUTE, _T ("mute")));
    m_map_DIK.insert (std::make_pair (DIK_CALCULATOR, _T ("calculator")));
    m_map_DIK.insert (std::make_pair (DIK_PLAYPAUSE, _T ("play-pause")));
    m_map_DIK.insert (std::make_pair (DIK_MEDIASTOP, _T ("media-stop")));
    m_map_DIK.insert (std::make_pair (DIK_VOLUMEDOWN, _T ("volume-")));
    m_map_DIK.insert (std::make_pair (DIK_VOLUMEUP, _T ("volume+")));
    m_map_DIK.insert (std::make_pair (DIK_WEBHOME, _T ("web-home")));
    m_map_DIK.insert (std::make_pair (DIK_NUMPADCOMMA, _T ("numpad-comma")));
    m_map_DIK.insert (std::make_pair (DIK_DIVIDE, _T ("numpad-divide")));
    m_map_DIK.insert (std::make_pair (DIK_SYSRQ, _T ("sys-rq")));
    m_map_DIK.insert (std::make_pair (DIK_RMENU, _T ("r-alt")));
    m_map_DIK.insert (std::make_pair (DIK_PAUSE, _T ("pause")));
    m_map_DIK.insert (std::make_pair (DIK_HOME, _T ("home")));
    m_map_DIK.insert (std::make_pair (DIK_UP, _T ("up")));
    m_map_DIK.insert (std::make_pair (DIK_PRIOR, _T ("prior")));
    m_map_DIK.insert (std::make_pair (DIK_LEFT, _T ("left")));
    m_map_DIK.insert (std::make_pair (DIK_RIGHT, _T ("right")));
    m_map_DIK.insert (std::make_pair (DIK_END, _T ("end")));
    m_map_DIK.insert (std::make_pair (DIK_DOWN, _T ("down")));
    m_map_DIK.insert (std::make_pair (DIK_NEXT, _T ("next")));
    m_map_DIK.insert (std::make_pair (DIK_INSERT, _T ("insert")));
    m_map_DIK.insert (std::make_pair (DIK_DELETE, _T ("delete")));
    m_map_DIK.insert (std::make_pair (DIK_LWIN, _T ("l-win")));
    m_map_DIK.insert (std::make_pair (DIK_RWIN, _T ("r-win")));
    m_map_DIK.insert (std::make_pair (DIK_APPS, _T ("app-menu")));
    m_map_DIK.insert (std::make_pair (DIK_POWER, _T ("power")));
    m_map_DIK.insert (std::make_pair (DIK_SLEEP, _T ("sleep")));
    m_map_DIK.insert (std::make_pair (DIK_WAKE, _T ("wake")));
    m_map_DIK.insert (std::make_pair (DIK_WEBSEARCH, _T ("web-serach")));
    m_map_DIK.insert (std::make_pair (DIK_WEBFAVORITES, _T ("web-favorites")));
    m_map_DIK.insert (std::make_pair (DIK_WEBREFRESH, _T ("web-refresh")));
    m_map_DIK.insert (std::make_pair (DIK_WEBSTOP, _T ("web-stop")));
    m_map_DIK.insert (std::make_pair (DIK_WEBFORWARD, _T ("web-forward")));
    m_map_DIK.insert (std::make_pair (DIK_WEBBACK, _T ("web-back")));
    m_map_DIK.insert (std::make_pair (DIK_MYCOMPUTER, _T ("my-pc")));
    m_map_DIK.insert (std::make_pair (DIK_MAIL, _T ("mail")));
    m_map_DIK.insert (std::make_pair (DIK_MEDIASELECT, _T ("media-select")));
  }

  LRESULT OnKeyMapper_Fire(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
    
    resetStatic ();
    SetDlgItemText (wID, CString (_T ("to set..")));

    m_AttachID = wID;
    return 0;
  }
  
  UINT FindSlot (UINT CmdID) {
    for (int ID = 0; ID != 10; ID++) {
      if (m_IdHash[ID] == CmdID)
        return ID;
      else ;
    }
    ATLASSERT (FALSE);
    return -1;
  }

  LPCTSTR FindString (UINT CmdID) {
    BYTE I2 = FindSlot (CmdID);
    ATLASSERT (I2 != -1);

    I2 = m_keySettings[I2];
    std::map<BYTE, LPCTSTR>::iterator iter = m_map_DIK.begin ();
    for (; iter != m_map_DIK.end (); iter++) {
      if (iter->first == I2)
        return iter->second;
      else ;
    }
    ATLASSERT (FALSE);
    return nullptr;
  }
  
  LPCTSTR FindStringByDIK (UINT DIK_ID) {
    std::map<BYTE, LPCTSTR>::iterator iter = m_map_DIK.begin ();
    for (; iter != m_map_DIK.end (); iter++) {
      if (iter->first == DIK_ID)
        return iter->second;
      else ;
    }
    ATLASSERT (FALSE);
    return nullptr;
  }

  void resetStatic (void) {
     SetDlgItemText (IDST_KEYSET_UP, FindString (IDST_KEYSET_UP));
     SetDlgItemText (IDST_KEYSET_DOWN, FindString (IDST_KEYSET_DOWN));
     SetDlgItemText (IDST_KEYSET_LEFT, FindString (IDST_KEYSET_LEFT));
     SetDlgItemText (IDST_KEYSET_RIGHT, FindString (IDST_KEYSET_RIGHT));
     SetDlgItemText (IDST_KEYSET_L, FindString (IDST_KEYSET_L));
     SetDlgItemText (IDST_KEYSET_R, FindString (IDST_KEYSET_R));
     SetDlgItemText (IDST_KEYSET_B, FindString (IDST_KEYSET_B));
     SetDlgItemText (IDST_KEYSET_A, FindString (IDST_KEYSET_A));
     SetDlgItemText (IDST_KEYSET_SELECT, FindString (IDST_KEYSET_SELECT));
     SetDlgItemText (IDST_KEYSET_START, FindString (IDST_KEYSET_START));
  }

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
    // Center the dialog on the screen
    CenterWindow();

    m_IdHash[0] = IDST_KEYSET_UP;
    m_IdHash[1] = IDST_KEYSET_DOWN;
    m_IdHash[2] = IDST_KEYSET_LEFT;
    m_IdHash[3] = IDST_KEYSET_RIGHT;
    m_IdHash[4] = IDST_KEYSET_L;
    m_IdHash[5] = IDST_KEYSET_R;
    m_IdHash[6] = IDST_KEYSET_B;
    m_IdHash[7] = IDST_KEYSET_A;
    m_IdHash[8] = IDST_KEYSET_SELECT;
    m_IdHash[9] = IDST_KEYSET_START;
    
    m_InputTest = new CGameInput ();
    m_InputTest->resetPlayType (*this, CGameInput::FOREGROUND);

    // Set button text .
    resetStatic ();

    m_StampTest = 0;
    m_AttachID = -1;

    return TRUE;
  }

  LRESULT OnOkOrCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    
    if (wID == IDOK)
      m_QuitConfirm = TRUE;

    delete m_InputTest;

    DestroyWindow ();
    
    ::PostQuitMessage (0);
    return 0;
  }

  INT_PTR DoModal (HWND parent) {
    MSG msg;

    Create (parent);
    :: SetForegroundWindow (*this);
    :: EnableWindow (parent, FALSE);
    ShowWindow (SW_NORMAL);

		for(;;)
		{
      while (:: PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT) {

          :: EnableWindow (parent, TRUE);
          :: SetForegroundWindow (parent);
          :: BringWindowToTop (parent);
          :: SetActiveWindow(parent);

          if (m_QuitConfirm != FALSE) {
            return IDOK;
          } else {
            return IDCANCEL;
          }
        } else if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ) {
          continue;
        } else if (! :: IsDialogMessage (*this, &msg)) {
          ::TranslateMessage(&msg);
          ::DispatchMessage(&msg);
        }      
      }
      if (m_StampTest == 0 || m_AttachID == (WORD)-1) {
        m_StampTest = timeGetTime ();
      } else {
        DWORD time_cur = timeGetTime ();
        if (time_cur - m_StampTest > 45) {
          m_StampTest = time_cur;
          // Update input .
          m_InputTest->poll ();
          for (DWORD id= 0; id != 256; id++) {
            if (m_InputTest->isPressedFirst ((BYTE) id)) {
              SetDlgItemText (m_AttachID, FindStringByDIK (id));
              m_keySettings[FindSlot(m_AttachID)] =(BYTE) id;
              break;
            }
          }
        }
      }
    }
    ATLASSERT (FALSE);
    return IDCANCEL;
  }
  WORD m_AttachID;
  BYTE *m_keySettings;
  WORD m_IdHash[10];
  DWORD m_StampTest;
  std::map<BYTE, LPCTSTR> m_map_DIK;
  BOOL m_QuitConfirm;
  CGameInput *m_InputTest;
};

# endif 