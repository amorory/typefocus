#define _WIN32_WINNT 0x0500
#ifndef UNICODE
#define UNICODE
#endif
#include <magnification.h>
#include <objbase.h>

#include <cstdlib>
#include <iostream>
#include <thread>

#include "Control.hpp"

struct StateInfo {};

void CALLBACK Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  MagInitialize();
  CoInitialize(NULL);

  // Create the host window
  Host host;
  host.Create(L"", WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_VISIBLE,
              WS_EX_LAYERED | WS_EX_TRANSPARENT, 0, 0, 0, 0);
  SetLayeredWindowAttributes(host.GetHwnd(), 0, 255, LWA_ALPHA);

  // Create a magnifier control that fills the client area.
  Control control(host.GetHwnd());

  ShowWindow(host.GetHwnd(), nCmdShow);
  UpdateWindow(host.GetHwnd());

  // Create a timer to update the control.
  // UINT_PTR timerId = SetTimer(host.GetHwnd(), 0, 16, Update);

  // Main message loop
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Shut down.
  MagUninitialize();
  return (int)msg.wParam;
}
