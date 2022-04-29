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

const int LENS_WIDTH = 300;
const int LENS_HEIGHT = 100;
const float MAGFACTOR = 2.0f;
const int OFFSET =
    GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) +
    GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYEDGE) +
    GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CYCURSOR);

HWND hwndHost;
HWND hwndMag;

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
  control.SetZoom(2.0f);

  ShowWindow(host.GetHwnd(), nCmdShow);
  UpdateWindow(host.GetHwnd());

  // Create a timer to update the control.
  // UINT_PTR timerId = SetTimer(host.GetHwnd(), 0, 16, Update);

  hwndHost = host.GetHwnd();
  hwndMag = control.GetHwnd();

  

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

// void CALLBACK Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
//   // Attach to thread of current window
//   DWORD currentThread = GetCurrentThreadId();
//   HWND hwndActive = GetForegroundWindow();
//   DWORD activeThread = GetWindowThreadProcessId(hwndActive, nullptr);
//   if (currentThread != activeThread) {
//     AttachThreadInput(currentThread, activeThread, true);
//   }

//   // Obtain caret location in screen coordinates
//   POINT caretPos;
//   if (GetCaretPos(&caretPos)) {
//     ClientToScreen(hwndActive, &caretPos);
//   }

//   // Offset by verticle area
//   caretPos.y += OFFSET;

//   // Calculate the source rectangle to be magnified
//   RECT sourceRect;
//   sourceRect.left = (caretPos.x - (int)((LENS_WIDTH / 2) / MAGFACTOR));
//   sourceRect.top = (caretPos.y - (int)((LENS_HEIGHT / 2) / MAGFACTOR));
//   sourceRect.right = sourceRect.left + (LENS_WIDTH / MAGFACTOR);
//   sourceRect.bottom = sourceRect.top + (LENS_HEIGHT / MAGFACTOR);

//   // Pass the source rectangle to the magnifier control.
//   MagSetWindowSource(hwndMag, sourceRect);

//   // Move the host window to be centered with the caret
//   SetWindowPos(hwndHost, HWND_TOPMOST, caretPos.x - (LENS_WIDTH / 2),
//                caretPos.y - (LENS_HEIGHT / 2), LENS_WIDTH, LENS_HEIGHT,
//                SWP_NOACTIVATE);

//   // Force the magnifier control to redraw itself.
//   InvalidateRect(hwndMag, NULL, TRUE);

//   return;
// }