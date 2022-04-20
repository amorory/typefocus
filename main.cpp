#define _WIN32_WINNT 0x0500
#ifndef UNICODE
#define UNICODE
#endif

#include <magnification.h>
#include <windows.h>

#include <cstdlib>
#include <iostream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
  // Register the window class.
  const wchar_t ZOOM_AREA[] = L"Zoom Area Class";

  WNDCLASS wc = {};

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = ZOOM_AREA;

  RegisterClass(&wc);

  // Create the window.

  HWND hwnd =
      CreateWindowEx(0,                    // Optional window styles.
                     ZOOM_AREA,            // Window class
                     L"Type Focus",        // Window text
                     WS_OVERLAPPEDWINDOW,  // Window style

                     // Size and position
                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                     NULL,       // Parent window
                     NULL,       // Menu
                     hInstance,  // Instance handle
                     NULL        // Additional application data
      );

  if (hwnd == NULL) {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  // Run the message loop.

  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
    case WM_CLOSE:
      if (MessageBox(hwnd, L"Really quit?", L"My application", MB_OKCANCEL) ==
          IDOK) {
        DestroyWindow(hwnd);
      }
      // Else: User canceled. Do nothing.
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      // All painting occurs here, between BeginPaint and EndPaint.

      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

      EndPaint(hwnd, &ps);
    }
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
