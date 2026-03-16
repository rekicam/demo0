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

#if !defined (_CWATCHSETTINGS_INCLUED_HEADER_CXX_)
# define _CWATCHSETTINGS_INCLUED_HEADER_CXX_

static __forceinline
void GetWatch (UINT32 &Source, int &iAddress, int &iSize) {
  iAddress = Source & 0xFFFFFFF;
  iSize = Source >> 28 & 15;
};

static __forceinline
void SetWatch (UINT32 &Target, int &iAddress, int &iSize) {
  Target = iAddress;
  Target&= 0xFFFFFFF;
  Target |= ( ((unsigned int)iSize) & 15) << 28;
};

static __forceinline
int GetWatchAddress (UINT32 &Target) {
  return Target& 0xFFFFFFF;
};

static __forceinline
int GetWatchSize (UINT32 &Target) {
  return Target>> 28 & 15;
};

struct WatchSettings {
  int iAddress;
  int iSize;
  UINT32 iMagicMix;

  WatchSettings (void) {}
  WatchSettings (UINT32 &source) { DecodeMixer (source); } 
  WatchSettings (int &iAddress_, int &iSize_) { EncodeMixer (iAddress_, iSize_); } 

  void EncodeMixer (int &iAddress_, int &iSize_) {
    :: SetWatch (iMagicMix, iAddress_, iSize_);
    iAddress = iAddress_;
    iSize = iSize_;
  }
  void EncodeMixer (void) {
    EncodeMixer (iAddress, iSize);
  }

  void DecodeMixer (UINT32 &source) {
    :: GetWatch (source, iAddress, iSize);
    iMagicMix = source;
  }
  void DecodeMixer (void) {
    DecodeMixer (iMagicMix);
  }
};

struct CWatchSettings : public CDialogImpl<CWatchSettings>
{
  enum { IDD = IDD_WATCH_SETTINGS };

  BOOL m_bModifyBP;
  UINT32 m_WatchAttach;
  UINT32 m_WatchSave;

  BEGIN_MSG_MAP (CWatchSettings)
    MESSAGE_HANDLER (WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER (IDOK, OnOkOrCancel)
    COMMAND_ID_HANDLER (IDCANCEL, OnOkOrCancel)
  END_MSG_MAP()
  
  CWatchSettings (BOOL bModifyBp = FALSE, UINT32 Item = 0) :
   m_bModifyBP (bModifyBp), m_WatchAttach  (Item) {}

  ~CWatchSettings (void) {}

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
    // Center the dialog on the screen
    CenterWindow();
    CString Caption;

    if (m_bModifyBP != FALSE) {
      Caption.Append (_T ("Modify"));
      :: SetWindowText (GetDlgItem (IDOK), _T ("Modify"));

      CEdit GEditAddr (GetDlgItem (IDET_WATCH_MEM_ADDR));
      CString strAddr;
      WatchSettings settings (m_WatchAttach);

      strAddr.Format (_T ("%07X"), settings.iAddress);
      GEditAddr.SetWindowText (strAddr);

      if (settings.iSize == 1)
        CheckDlgButton (IDRD_WATCH_MEM_SIZE_1, BST_CHECKED);
      else if  (GetWatchSize (m_WatchAttach) == 2)
        CheckDlgButton (IDRD_WATCH_MEM_SIZE_2, BST_CHECKED);
      else if  (GetWatchSize (m_WatchAttach) == 4)
        CheckDlgButton (IDRD_WATCH_MEM_SIZE_4, BST_CHECKED);
    } else {
      // Append Watch.
      CEdit GEditAddr (GetDlgItem (IDET_WATCH_MEM_ADDR));
      Caption.Append (_T ("Append"));
      GEditAddr.SetWindowText (_T ("3000000"));
      CheckDlgButton (IDRD_WATCH_MEM_SIZE_4, BST_CHECKED);
    }

    SetWindowText (Caption);
    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_WATCH)), TRUE);
    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_WATCH)), FALSE);
    return TRUE;
  }

  LRESULT OnOkOrCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    
    if (wID == IDOK) {
      // check result.
      CEdit GEditAddr (GetDlgItem (IDET_WATCH_MEM_ADDR));
      CString strAddr;

      WatchSettings settings;
      GEditAddr.GetWindowText (strAddr);
      
      settings.iAddress = _tcstol (strAddr.GetBuffer (), NULL, 16);
      if (IsDlgButtonChecked (IDRD_WATCH_MEM_SIZE_1) == BST_CHECKED)
        settings.iSize = 1;
      else if  (IsDlgButtonChecked (IDRD_WATCH_MEM_SIZE_2) == BST_CHECKED)
        settings.iSize  = 2;
      else if  (IsDlgButtonChecked (IDRD_WATCH_MEM_SIZE_4) == BST_CHECKED)
        settings.iSize  = 4;
      settings.EncodeMixer ();
      m_WatchSave = settings.iMagicMix;
    }
    EndDialog (wID);
    return 0;
  }
};

# endif 