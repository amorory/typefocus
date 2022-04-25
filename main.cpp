#define _WIN32_WINNT 0x0500
#ifndef UNICODE
#define UNICODE
#endif

#include <cstdlib>
#include <iostream>

#include "MainWindow.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  MainWindow win;

  if (!win.Create(L"Learn to Program Windows", WS_OVERLAPPEDWINDOW)) {
    return 0;
  }

  ShowWindow(win.Window(), nCmdShow);

  // Run the message loop.

  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
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
//   // float mat[3][3] ={{2.0, 0.0, 0.0}, {0.0, 2.0, 0.0}, {0.0, 0.0, 1.0}};
//   // MagSetWindowTransform(active, &MAGTRANSFORM(mat));
// }
// }
