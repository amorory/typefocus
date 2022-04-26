#define _WIN32_WINNT 0x0500
#ifndef UNICODE
#define UNICODE
#endif

#include <magnification.h>

#include <cstdlib>
#include <iostream>
#include <thread>

#include "MainWindow.hpp"

const int LENS_WIDTH = 300;
const int LENS_HEIGHT = 100;
const float MAGFACTOR = 2.0f;
const int OFFSET =
    GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION) +
    GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYEDGE) +
    GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CYCURSOR);

HWND hwndHost;
HWND hwndMag;

bool SetMagnificationFactor();
void CALLBACK UpdateMagWindow(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                              DWORD dwTime);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  if (!MagInitialize()) {
    return 0;
  }

  // Create the host window
  MainWindow winMain;

  if (!winMain.Create(L"", WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_VISIBLE,
                      WS_EX_LAYERED | WS_EX_TRANSPARENT, 0, 0, 0, 0)) {
    return 0;
  }

  hwndHost = winMain.Window();

  // Adjust for dpi

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

  SetMagnificationFactor();

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

bool SetMagnificationFactor() {
  MAGTRANSFORM matrix;
  memset(&matrix, 0, sizeof(matrix));
  matrix.v[0][0] = MAGFACTOR;
  matrix.v[1][1] = MAGFACTOR;
  matrix.v[2][2] = 1.0f;

  return MagSetWindowTransform(hwndMag, &matrix);
}

void CALLBACK UpdateMagWindow(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                              DWORD dwTime) {
  // Attach to thread of current window
  DWORD currentThread = GetCurrentThreadId();
  HWND hwndActive = GetForegroundWindow();
  if (hwndActive == hwndMag || hwndActive == hwndHost) {
    hwndActive = GetNextWindow(hwndActive, GW_HWNDNEXT);
  }
  DWORD activeThread = GetWindowThreadProcessId(hwndActive, nullptr);
  if (currentThread != activeThread) {
    AttachThreadInput(currentThread, activeThread, true);
  }

  // Obtain caret location in screen coordinates
  POINT caretPos;
  if (GetCaretPos(&caretPos)) {
    ClientToScreen(hwndActive, &caretPos);
  }

  // Offset by verticle area
  caretPos.y += OFFSET;

  // Calculate a source rectangle that is centered at the mouse coordinates.
  // Size the rectangle so that it fits into the magnifier window (the lens).
  RECT sourceRect;
  sourceRect.left = (caretPos.x - (int)((LENS_WIDTH / 2) / MAGFACTOR));
  sourceRect.top = (caretPos.y - (int)((LENS_HEIGHT / 2) / MAGFACTOR));
  sourceRect.right = sourceRect.left + (LENS_WIDTH / MAGFACTOR);
  sourceRect.bottom = sourceRect.top + (LENS_HEIGHT / MAGFACTOR);

  // Pass the source rectangle to the magnifier control.
  MagSetWindowSource(hwndMag, sourceRect);

  // Move the host window so that the origin of the client area lines up
  // with the origin of the magnified source rectangle.

  SetWindowPos(hwndHost, HWND_TOPMOST, caretPos.x - (LENS_WIDTH / 2),
               caretPos.y - (LENS_HEIGHT / 2), LENS_WIDTH, LENS_HEIGHT,
               SWP_NOACTIVATE);

  // Force the magnifier control to redraw itself.
  InvalidateRect(hwndMag, NULL, TRUE);

  return;
}
