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

// TODO: too simple. WM_CREATE maybe lost.
# if _MSC_VER >= 1000
#   pragma once
# endif // _MSC_VER >= 1000

# if !defined (_CDYNAMICMESSAGEMAP_INCLUED_HEADER_CXX_)
#  define _CDYNAMICMESSAGEMAP_INCLUED_HEADER_CXX_

# include <WTL\atlapp.h>
# include <list>

template <class T>
struct CDnMsg {
  LRESULT (T::*msg_stuff)(CDnMsg<T>* pMsg, BOOL& bHandled);
  WPARAM wparam;
  LPARAM lparam;
  UINT msg;
  HWND from;
  DWORD mapid;

  CDnMsg (void) {}
  CDnMsg (UINT message, LRESULT (T::*OnMessageMap)(CDnMsg* pMsg, BOOL& bHandled)) {
    msg_stuff = OnMessageMap;
    msg = message;
  }
};

template <class basis = CWindow, class reflect = CWindow>
class CDynamicMessageFilter : CMessageFilter {
public:
  CDynamicMessageFilter (basis *basis_attach_, reflect *reflect_attach_) {
    basis_this  = basis_attach_;
    reflect_this = reflect_attach_;
    CMessageLoop *p = _Module.GetMessageLoop ();
    p->AddMessageFilter (this);
  }
  ~ CDynamicMessageFilter (void) {
    CMessageLoop *p = _Module.GetMessageLoop ();
    if (p != nullptr)
      p->RemoveMessageFilter (this);
    remove_msg_map_all ();
  }
  void add_msg_map (UINT message, LRESULT (reflect::*OnMessageMap)(CDnMsg<reflect>* pMsg, BOOL& bHandled)) {
    m_call_list.push_front (new CDnMsg<reflect>(message, OnMessageMap));
  }
  void remove_msg_map (UINT message) {
    for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++)
      if ((*iter)->msg == pMsg->message)     {
          delete (*iter);
          m_call_list.erase (iter);
          return ;
      }  
  }
  void remove_msg_map_all (void) {
    for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); ) {
        delete (*iter);
        iter = m_call_list.erase (iter);
        return ;
    }  
  }

  basis *basis_this;
  reflect *reflect_this;
  std::list<CDnMsg<reflect> *> m_call_list;

  BOOL PreTranslateMessage (MSG* pMsg) {
    if (pMsg->hwnd == *basis_this) {
      // foreach list
      for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++) {
        if ((*iter)->msg == pMsg->message)     {
          CDnMsg<reflect> tMsg;
          BOOL bHandled = TRUE;
          HRESULT sig;
          tMsg.from = pMsg->hwnd;
          tMsg.lparam = pMsg->lParam;
          tMsg.wparam = pMsg->wParam;

          sig = (reflect_this->*(*iter)->msg_stuff) (& tMsg, bHandled);
          return bHandled;
        }
      }
    }
    return FALSE;
  }
};

template <class basis = CWindow, class reflect = CWindow>
class CDynamicMessageMap : CMessageMap {
public:
  CDynamicMessageMap (basis *basis_attach_, reflect *reflect_attach_) {
    basis_this  = basis_attach_;
    reflect_this = reflect_attach_;
  }
  ~ CDynamicMessageMap (void) {
  }
  void add_msg_map (UINT message, LRESULT (reflect::*OnMessageMap)(CDnMsg<reflect>* pMsg, BOOL& bHandled)) {
    m_call_list.push_back (CDnMsg<reflect>(message, OnMessageMap));
  }
  void remove_msg_map (UINT message) {
    for (std::list<CDnMsg<reflect>>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++)
      if ((*iter).msg == pMsg->message)     {
          m_call_list.erase (iter);
          return ;
      }  
  }

  basis *basis_this;
  reflect *reflect_this;
  std::vector<CDnMsg<reflect>> m_call_list;

	BOOL ProcessWindowMessage_T (_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,
		_In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID = 0)
  {
    if (hWnd == *basis_this) {
      // foreach list
      for (std::vector<CDnMsg<reflect>>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++) {
        if ((*iter).msg == uMsg)     {
          CDnMsg<reflect> tMsg;
          BOOL bHandled = TRUE;
          // HRESULT sig;
          tMsg.from = hWnd;
          tMsg.lparam = lParam;
          tMsg.wparam = wParam;
          tMsg.msg = uMsg;
          tMsg.mapid = dwMsgMapID;
          lResult = (reflect_this->*(*iter).msg_stuff) (& tMsg, bHandled);
          return bHandled;
        }
      }
    }
    return FALSE;
  }
};

template <class basis = CWindow, class reflect = CWindow>
class CDynamicMessageMapWeak  {
public:
  CDynamicMessageMapWeak (basis *basis_attach_, reflect *reflect_attach_) {
    basis_this  = basis_attach_;
    reflect_this = reflect_attach_;
  }
  ~ CDynamicMessageMapWeak (void) {
  }
  void add_msg_map (UINT message, LRESULT (reflect::*OnMessageMap)(CDnMsg<reflect>* pMsg, BOOL& bHandled)) {
    m_call_list.push_front (new CDnMsg<reflect>(message, OnMessageMap));
  }
  void remove_msg_map (UINT message) {
    for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++)
      if ((*iter)->msg == pMsg->message)     {
          delete (*iter);
          m_call_list.erase (iter);
          return ;
      }  
  }
  void remove_msg_map_all (void) {
    for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); ) {
        delete (*iter);
        iter = m_call_list.erase (iter);
        return ;
    }  
  }

  basis *basis_this;
  reflect *reflect_this;
  std::list<CDnMsg<reflect> *> m_call_list;

	BOOL ProcessWindowMessage_T (_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,
		_In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID = 0)
  {
    if (hWnd == *basis_this) {
      // foreach list
      for (std::list<CDnMsg<reflect> *>::iterator iter = m_call_list.begin ();  iter != m_call_list.end (); iter++) {
        if ((*iter)->msg == uMsg)     {
          CDnMsg<reflect> tMsg;
          BOOL bHandled = TRUE;
          HRESULT sig;
          tMsg.from = hWnd;
          tMsg.lparam = lParam;
          tMsg.wparam = wParam;
          tMsg.msg = uMsg;
          tMsg.mapid = dwMsgMapID;
          lResult = (reflect_this->*(*iter)->msg_stuff) (& tMsg, bHandled);
          return bHandled;
        }
      }
    }
    return FALSE;
  }
};

// XXX:bad hook.
#define SET_DYNA_MSG_MAP() \
public: \
BOOL ProcessWindowMessage(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,\
		_In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID = 0) \
{\
  return ProcessWindowMessage_T (hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);\
}
#define SET_DYNA_MSG_MAP2(baseClass) \
public: \
BOOL ProcessWindowMessage(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,\
		_In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID = 0) \
{\
  if (ProcessWindowMessage_T (hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID))\
    return TRUE;\
  else \
    CHAIN_MSG_MAP (baseClass);\
  return FALSE; \
}

# endif 