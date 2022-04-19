#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <cstdlib>
#include <iostream>

int main() {
  while (true) {
    DWORD currentThread = GetCurrentThreadId();
    HWND active = GetForegroundWindow();
    DWORD activeThread = GetWindowThreadProcessId(active, nullptr);
    if (currentThread != activeThread) {
      AttachThreadInput(currentThread, activeThread, true);
    }

    POINT coordinates;
    if (GetCaretPos(&coordinates)) {
      ClientToScreen(active, &coordinates);
      std::cout << coordinates.x << ", " << coordinates.y << std::endl;
      Sleep(1000);
    }
  }
}
