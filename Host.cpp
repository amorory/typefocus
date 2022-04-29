#include "Host.hpp"

Host* Host::host_ = nullptr;

Host::Host() { host_ = this; }

LRESULT Host::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return FALSE;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd_, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
      EndPaint(hwnd_, &ps);
      return FALSE;
    }

    default:
      return DefWindowProc(hwnd_, uMsg, wParam, lParam);
  }
  return TRUE;
}
