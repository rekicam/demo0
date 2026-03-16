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

# if !defined (_CMESSAGEIDLEFASTLOOP_INCLUED_HEADER_CXX_)
#  define _CMESSAGEIDLEFASTLOOP_INCLUED_HEADER_CXX_

# include "stdwtl.h"

// ATL 's CMessageLoop very slow
class CMessageIdleFastLoop : public CMessageLoop {
public :
  int Run()
	{
		for(;;)
		{
      while (:: PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
      {
        if (m_msg.message == WM_QUIT)
          return TRUE;
			  if(!PreTranslateMessage(&m_msg))
			  {
				  ::TranslateMessage(&m_msg);
				  ::DispatchMessage(&m_msg);
			  }
      }
      // scan all idle 
      CMessageLoop :: OnIdle (0);
    }
  }
};

# endif // !defined(_CMESSAGEIDLEFASTLOOP_INCLUED_HEADER_CXX_)