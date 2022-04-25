#define _WIN32_WINNT 0x0500
#ifndef UNICODE
#define UNICODE
#endif

#include <magnification.h>

#include <cstdlib>
#include <iostream>
#include <thread>

#include "MainWindow.hpp"

const int LENS_WIDTH = 25;
const int LENS_HEIGHT = 25;
const float MAGFACTOR = 2.0f;

HWND hwndHost;
HWND hwndMag;

void CALLBACK UpdateMagWindow(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                              DWORD dwTime);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  if (!MagInitialize()) {
    return 0;
  }

  // Create the host window
  MainWindow winMain;

  if (!winMain.Create(L"TypeFocus", WS_CLIPCHILDREN,
                      WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, 0, 0,
                      0, 0)) {
    return 0;
  }

  hwndHost = winMain.Window();

  // Make the window opaque.
  SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);

  // Create a magnifier control that fills the client area.
  hwndMag =
      CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                   WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE, 0, 0,
                   LENS_WIDTH, LENS_HEIGHT, hwndHost, NULL, hInstance, NULL);
  if (!hwndMag) {
    return FALSE;
  }

  ShowWindow(winMain.Window(), nCmdShow);
  UpdateWindow(winMain.Window());

  // Create a timer to update the control.
  UINT_PTR timerId = SetTimer(winMain.Window(), 0, 16, UpdateMagWindow);

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

void CALLBACK UpdateMagWindow(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                              DWORD dwTime) {
  // Get the mouse coordinates.
  POINT mousePoint;
  GetCursorPos(&mousePoint);

  // Calculate a source rectangle that is centered at the mouse coordinates.
  // Size the rectangle so that it fits into the magnifier window (the lens).
  RECT sourceRect;
  int borderWidth = GetSystemMetrics(SM_CXFIXEDFRAME);
  int captionHeight = GetSystemMetrics(SM_CYCAPTION);
  sourceRect.left = (mousePoint.x - (int)((LENS_WIDTH / 2) / MAGFACTOR)) +
                    (int)(borderWidth / MAGFACTOR);
  sourceRect.top = (mousePoint.y - (int)((LENS_HEIGHT / 2) / MAGFACTOR)) +
                   (int)(captionHeight / MAGFACTOR) +
                   (int)(borderWidth / MAGFACTOR);
  sourceRect.right = LENS_WIDTH;
  sourceRect.bottom = LENS_HEIGHT;

  // Pass the source rectangle to the magnifier control.
  MagSetWindowSource(hwndMag, sourceRect);

  // Move the host window so that the origin of the client area lines up
  // with the origin of the magnified source rectangle.
  MoveWindow(hwndHost, (mousePoint.x - LENS_WIDTH / 2),
             (mousePoint.y - LENS_HEIGHT / 2), LENS_WIDTH, LENS_HEIGHT, FALSE);

  // Force the magnifier control to redraw itself.
  InvalidateRect(hwndMag, NULL, TRUE);

  return;
}
// int main() {
// MagInitialize();
// while (true) {
//   DWORD currentThread = GetCurrentThreadId();
//   HWND active = GetForegroundWindow();
//   DWORD activeThread = GetWindowThreadProcessId(active, nullptr);
//   if (currentThread != activeThread) {
//     AttachThreadInput(currentThread, activeThread, true);
//   }

//   POINT coordinates;
//   if (GetCaretPos(&coordinates)) {
//     ClientToScreen(active, &coordinates);
//     std::cout << coordinates.x << ", " << coordinates.y << std::endl;
//     Sleep(1000);
//   }

//   RECT area{coordinates.x - 25, coordinates.y - 10, coordinates.x + 25,
//             coordinates.y + 10};
//   // MagSetWindowSource(active, area);
//   // float mat[3][3] ={{2.0, 0.0, 0.0}, {0.0, 2.0, 0.0}, {0.0,
//   0.0, 1.0}};
//   // MagSetWindowTransform(active, &MAGTRANSFORM(mat));
// }
// }
