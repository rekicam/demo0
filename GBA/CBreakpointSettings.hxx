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

#if !defined (_CBREAKPOINTSETTINGS_INCLUED_HEADER_CXX_)
# define _CBREAKPOINTSETTINGS_INCLUED_HEADER_CXX_

// TODO Int, SoftInterrupt
struct CBreakpointSettings : public CDialogImpl<CBreakpointSettings>
{
  enum { IDD = IDD_BREAKPOINT_SETTINGS };

  BOOL m_bModifyBP;
  struct gba *m_GBA;
  BREAKPOINT_REASON m_iBPCase; 

  struct breakpoint * m_AttachBP;
  struct breakpoint * m_CrreateBP;

  BEGIN_MSG_MAP (CBreakpointSettings)
    MESSAGE_HANDLER (WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER (IDOK, OnOkOrCancel)
    COMMAND_ID_HANDLER (IDCANCEL, OnOkOrCancel)
  END_MSG_MAP()
  
  CBreakpointSettings (BREAKPOINT_REASON iBpCase = BREAKPOINT_REASON_R15, BOOL bModifyBp = FALSE, struct breakpoint *bp = NULL) :
    m_iBPCase (iBpCase), m_bModifyBP (bModifyBp), m_AttachBP  (bp), m_CrreateBP (NULL) {}

  ~CBreakpointSettings (void) {}

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    
    // Center the dialog on the screen
    CenterWindow();
    CString Caption;
    LPCTSTR pCmdStr[5] = { _T ("PADDR"), _T ("MEM-READ"), _T ("MEM-WRITE"), _T ("INTERRUPT"), _T ("SOFT-INTERRUPT") };
    ATLASSERT (m_iBPCase < (sizeof (pCmdStr) / sizeof (pCmdStr[0])));

    CheckDlgButton (IDCK_BP_EN, BST_CHECKED);
    CheckDlgButton (IDRD_WATCH_MEM_SIZE_4, BST_CHECKED);
    switch (m_iBPCase) {
    case BREAKPOINT_REASON_HARDWARE_INTERRUPT: // INT
    case BREAKPOINT_REASON_SOFTWARE_INTERRUPT: // SINT
    case BREAKPOINT_REASON_R15: // PADDR 
      :: EnableWindow (GetDlgItem (IDRD_BP_MEM_SIZE_4), FALSE);
      :: EnableWindow (GetDlgItem (IDRD_BP_MEM_SIZE_2), FALSE);
      :: EnableWindow (GetDlgItem (IDRD_BP_MEM_SIZE_1), FALSE);
    case BREAKPOINT_REASON_MEM_READ:
    case BREAKPOINT_REASON_MEM_WRITE: 
    default:
      break;
    }

    if (m_bModifyBP != FALSE) {
      ATLASSERT (m_AttachBP != NULL);
      Caption.Append (_T ("Modify @ "));
      Caption.Append (pCmdStr[m_iBPCase] );

      :: SetWindowText (GetDlgItem (IDOK), _T ("Modify"));

      CEdit GEditAddr (GetDlgItem (IDET_BP_MEM_ADDR));
      CEdit GEditHit (GetDlgItem (IDET_BP_HIT_COUNT));
      CEdit GEditCond  (GetDlgItem (IDET_BP_COND));
      CEdit CEditOutput (GetDlgItem (IDET_BP_OUTPUT));
      CString strAddr;
      CString strHit;
      CString strCond (m_AttachBP->describe);
      CString strOutput;

      strAddr.Format (_T ("%07X"), m_AttachBP->address & 0xFFFFFFF);
      strHit.Format (_T ("%d"), m_AttachBP->match_expect & 0xFFFFFFF);

      if (m_AttachBP->enable != false)
        CheckDlgButton (IDCK_BP_EN, BST_CHECKED);
      else 
        CheckDlgButton (IDCK_BP_EN, BST_UNCHECKED);
      GEditCond.SetWindowText (strCond);
      GEditHit.SetWindowText (strHit);
      GEditAddr.SetWindowText (strAddr);

      if (m_AttachBP->size == 1)
        CheckDlgButton (IDRD_BP_MEM_SIZE_1, BST_CHECKED);
      else if  (m_AttachBP->size == 2)
        CheckDlgButton (IDRD_BP_MEM_SIZE_2, BST_CHECKED);
      else if  (m_AttachBP->size == 4)
        CheckDlgButton (IDRD_BP_MEM_SIZE_4, BST_CHECKED);
    } else {
      // Append Breakpoint.
      Caption.Append (_T ("Append @ "));
      Caption.Append (pCmdStr[m_iBPCase] );
    }

    SetWindowText (Caption);
    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_BREAKPOINT)), TRUE);
    SetIcon (LoadIcon (_Module.GetResourceInstance (), MAKEINTRESOURCE (IDI_BREAKPOINT)), FALSE);
    return TRUE;
  }

  LRESULT OnOkOrCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    
    if (wID == IDOK) {
      // check result.
      CEdit GEditAddr (GetDlgItem (IDET_BP_MEM_ADDR));
      CEdit GEditHit (GetDlgItem (IDET_BP_HIT_COUNT));
      CEdit GEditCond  (GetDlgItem (IDET_BP_COND));
      CEdit CEditOutput (GetDlgItem (IDET_BP_OUTPUT));
      CString strAddr;
      CString strHit;
      CString strCond;
      CString strOutput;

      GEditAddr.GetWindowText (strAddr);
      GEditHit.GetWindowText (strHit);
      GEditCond.GetWindowText (strCond);
      CEditOutput.GetWindowText (strOutput);
      strOutput.Preallocate (512);

      int iAddress = _tcstol (strAddr.GetBuffer (), NULL, 16);
      int iCountHit = _tcstol (strHit.GetBuffer (), NULL, 10);
      int iSize = 0;
      kable enable = IsDlgButtonChecked (IDCK_BP_EN) == BST_CHECKED;

      if (IsDlgButtonChecked (IDRD_BP_MEM_SIZE_1) == BST_CHECKED)
        iSize = 1;
      else if  (IsDlgButtonChecked (IDRD_BP_MEM_SIZE_2) == BST_CHECKED)
        iSize = 2;
      else if  (IsDlgButtonChecked (IDRD_BP_MEM_SIZE_4) == BST_CHECKED)
        iSize = 4;

      // If current is bp level 0 (code interrupt)
      // Check addr range ..
      if (!m_iBPCase && gba_iscode_addr (m_GBA, iAddress) == false)  {
        strOutput.Format (_T ("Not a valid code addres, \nyour set:%08X"), iAddress);
          CEditOutput.SetWindowText (strOutput);
          return 0;
      }

      if (m_bModifyBP == FALSE) {
        m_CrreateBP = 
                   gba_breakpoint_create (m_GBA, false, m_iBPCase, enable, 
                                      iAddress, iSize, strCond.GetBuffer (), strCond.GetLength (),
                                         iCountHit, NULL, 0, strOutput.GetBuffer (), strOutput.GetLength ());
        if (m_CrreateBP == NULL) {
          CEditOutput.SetWindowText (strOutput);
          return 0;
        }
      } else {
        int sig = 
                   gba_breakpoint_modify (m_GBA, m_AttachBP, enable, 
                                      iAddress, iSize, strCond.GetBuffer (), strCond.GetLength (),
                                         iCountHit, NULL, 0, strOutput.GetBuffer (), strOutput.GetLength ());
        if (sig != 0) {
          CEditOutput.SetWindowText (strOutput);
          return 0;
        }
      }
    }
    EndDialog (wID);
    return 0;
  }
};

# endif 