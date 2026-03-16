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

# if !defined (_CDOCKINGTABCHILD_INCLUED_HEADER_CXX_)
#  define _CDOCKINGTABCHILD_INCLUED_HEADER_CXX_

# include "stdwtl.h"
# include "CDockingTabContainer.hxx"


namespace dockwins
{
template < class T,
         class TBase = CWindow,
         class TDockingWinTraits = CVC7LikeExDockingContainerWindowTraits>
class CDockingTabChild : public dockwins::CBoxedDockingWindowImpl< T,
                TBase, TDockingWinTraits >   
{
  // CDockingTabChild (){}
  //~CDockingTabChild(){}
};
}//namespace dockwins

# endif // !defined(_CDOCKINGTABCHILD_INCLUED_HEADER_CXX_)