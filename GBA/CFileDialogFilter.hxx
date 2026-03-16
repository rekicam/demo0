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

// Not mine, see .https://www.codeproject.com/articles/3167/webcontrols/ 
// Implementation of the CFileDialogFilter class.
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CFILEDIALOGFILTER_INCLUED_HEADER_CXX_)
#  define _CFILEDIALOGFILTER_INCLUED_HEADER_CXX_

# include <WTL\atlmisc.h>

// Class to support a filter list when using the WTL CFileDialog.
// Allows a filter string delimited with a pipe to be used (instead of a string
// delimited with '\0')
class CFileDialogFilter  
{
private:
  CString m_strFilter;
public:
  CFileDialogFilter()
  {
  }

  /// nID The ID of a resource string containing the filter
  CFileDialogFilter(UINT nID)
  {
    SetFilter(nID);
  }
  
  /// lpsz The filter string
  CFileDialogFilter(LPCTSTR lpsz)
  {
    SetFilter(lpsz);
  }
  
  ~CFileDialogFilter()
  {
  }

  inline LPCTSTR GetFilter() const { return m_strFilter; }
  inline operator LPCTSTR() const { return m_strFilter; }

  // Set the filter string to use
  // nID - The ID of a resource string containing the filter
  void SetFilter(UINT nID)
  {
    if (m_strFilter.LoadString(nID) && !m_strFilter.IsEmpty())
      ModifyString();
  }

  // Set the filter string to use
  // lpsz - The filter string
  void SetFilter(LPCTSTR lpsz)
  {    
    m_strFilter = lpsz;
    if (!m_strFilter.IsEmpty())
      ModifyString();
  }
private:
  // Replace '|' with '\0'
  void ModifyString(void)
  {
    // Get a pointer to the string buffer
    LPTSTR psz = m_strFilter.GetBuffer(0);
    // Replace '|' with '\0'
    while ((psz = _tcschr(psz, '|')) != NULL)
      *psz++ = '\0';
  }
};

# endif 
