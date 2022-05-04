#define _WIN32_WINNT 0x0500

#include "Program.hpp"

void CALLBACK Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  MagInitialize();
  CoInitializeEx(NULL, COINIT_MULTITHREADED);

  // Create the host window
  Program program;

  // Create a magnifier control that fills the client area.
  program.CreateControl();

  
  ShowWindow(program.GetHwnd(), nCmdShow);
  UpdateWindow(program.GetHwnd());
  RegisterHotKey(program.GetHwnd(), 0, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4D);
  RegisterHotKey(program.GetHwnd(), 1, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4C);
  RegisterHotKey(program.GetHwnd(), 2, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4B);
  RegisterHotKey(program.GetHwnd(), 3, 0x4000 | MOD_ALT | MOD_SHIFT, 0x50);
  RegisterHotKey(program.GetHwnd(), 4, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4E);

  // Main message loop
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Shut down.
  MagUninitialize();
  CoUninitialize();
  return (int)msg.wParam;
}
