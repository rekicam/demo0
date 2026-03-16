# include "stdwtl.h"
# define CMAINFRAME_IMPL
# include "CMainFrame.hxx"
# include "CMessageIdleFastLoop.hxx"
#include "..\gba\gba.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
  // Add ATL MessageLoop map .
  CMessageIdleFastLoop theLoop;
  _Module.AddMessageLoop (& theLoop);

  // Create main window
  CMainFrame wndMain;

  HANDLE hRichHandle =  ::LoadLibrary (CRichEditCtrl::GetLibraryName ());
  ATLASSERT (hRichHandle != NULL);

  HWND hWindow = wndMain.CreateEx (NULL, CRect (0, 0, 48, 32), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, WS_EX_TRANSPARENT |  WS_EX_ACCEPTFILES | WS_EX_OVERLAPPEDWINDOW);
  ATLASSERT (hWindow != NULL);

  wndMain.SetWindowText (_T ("ArchBoyAdvance-CoreLibrary"));
  
  wndMain.CenterWindow (NULL);
  wndMain.ResizeClient (240, 160);
  wndMain.ShowWindow (SW_SHOWNORMAL); 

  // init loading window.
  //CLoading_desc ld;
  //initLoadingWindow (& ld);
  // Create Direct, misc object 
  //CDirectDraw :: InitDirect3D ();   // Direct3D init
  //CGameInput :: InitDInput8 ();   // DInput8 init.
  //CDirectMusic2 :: InitDirectMusic (wndMain); // DirectMusic, DirectSound init.
  //wndMain.internalObjectInit ();



  // Set Process Pri.
  /////SetPriorityClass (GetCurrentProcess (), HIGH_PRIORITY_CLASS);
  //SetProcessPriorityBoost (GetCurrentProcess (), TRUE);
  
  //wndMain.CreateMidiPiano ();

  //SetEvent (ld.SignalLoadingEvent);
  //WaitForSingleObject (ld.ThreadWaitHandle, INFINITE);

  //ReleaseCLoading_desc (& ld);
#if 0
  wndMain.ResizeClient (360, 240);
  
  HWND hForeWnd = NULL; 
  HWND hWnd= wndMain; 
  DWORD dwForeID; 
  DWORD dwCurID; 
 
  hForeWnd =  GetForegroundWindow(); 
  dwCurID =  GetCurrentThreadId(); 
  dwForeID =  GetWindowThreadProcessId (hForeWnd, NULL); 
  AttachThreadInput (dwCurID, dwForeID, TRUE); 
  wndMain.CenterWindow (NULL);
  ShowWindow (hWnd, SW_SHOWNORMAL); 
  SetWindowPos (hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE); 
  SetWindowPos (hWnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE); 
  SetForegroundWindow (hWnd); 
  AttachThreadInput( dwCurID, dwForeID, FALSE);
  SetFocus (hWnd);
  SetWindowPos (hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE); 
  SetFocus (hWnd);
  SetWindowPos (hWnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE); 
#endif 

  int nRet = theLoop.Run();

  _Module.RemoveMessageLoop();


  // Uninit Direct3D draw base 
  //CDirectDraw :: UninitDirect3D ();
  //CDirectMusic2 :: UninitDirectMusic ();
  //CDirectInput :: UninitDInput8 ();
  FILE *fp = _tfopen (wndMain.m_PathSave, _T ("wb+"));
  if (fp != NULL) {
    gba_save_battery (wndMain.m_GBA, fp);
    fclose (fp);
  }
  return nRet;
}

#if 0
#define _tWinMain WinMain_0
#define _tmain _tmain_0
#endif 
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
  HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//  HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
  ATLASSERT(SUCCEEDED(hRes));

  // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
  ::DefWindowProc(NULL, 0, 0, 0L);

  AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);  // add flags to support other controls

  hRes = _Module.Init(NULL, hInstance);
  ATLASSERT(SUCCEEDED(hRes));

  // GetExceptionInformation
  // EXCEPTION_EXECUTE_HANDLER
  int nRet = -1;
  // Install SEH Callback ..
  //__try {
      nRet = Run(lpstrCmdLine, nCmdShow);
  //} __except (EXCEPTION_EXECUTE_HANDLER) {
    // TODO: do some cleanup work...
    // Make crash report...
    //CString cString;
    // DWORD dwExceptCode = :: GetExceptionCode ();
    //_EXCEPTION_POINTERS *ExceptInfos = GetExceptionInformation ();
    






  //}





  _Module.Term();
  ::CoUninitialize();

  return nRet;
}

int _tmain (void) {
  MessageBoxA (NULL, "This is just an incomplete demo demonstration based on ATL/WTL. Programs, mainly for the provision of GBA core libraries and debug", "moecmks", MB_OK);
  _tWinMain (GetModuleHandle (nullptr), nullptr, nullptr, 0);
  return 0;
}

/*
extern "C" void WinMainCRTStartup();

__declspec (dllexport)
  void __cdecl crt_init (void) 
{
  WinMainCRTStartup();
}
*/