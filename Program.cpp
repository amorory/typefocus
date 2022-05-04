#include "Program.hpp"

#include <iostream>

std::function<void(HWND, DWORD, LONG, LONG)> Program::HookBinder;

Program::Program()
    : disabled_(false), hidden_(true), running_(true), current_zoom_(1.5) {
  HookBinder = std::bind(&Program::HookProcedure, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3,
                         std::placeholders::_4);
  lens_height_ = GetSystemMetrics(SM_CYCURSOR) * 2;
  lens_width_ = lens_height_ * 3;

  WNDCLASSEX wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = 0;
  wcex.lpfnWndProc = WindowCallback;
  wcex.hInstance = GetModuleHandle(NULL);
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
  wcex.lpszClassName = L"TypeFocus";

  RegisterClassEx(&wcex);

  hwnd_host_ =
      CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, L"TypeFocus", L"",
                     WS_CLIPCHILDREN | WS_POPUPWINDOW | WS_VISIBLE, 0, 0, 0, 0,
                     NULL, NULL, GetModuleHandle(NULL), NULL);

  // Store this within window
  SetWindowLongPtr(hwnd_host_, GWLP_USERDATA, (LONG_PTR)this);
  SetLayeredWindowAttributes(hwnd_host_, 0, 255, LWA_ALPHA);
}

void Program::CreateControl() {
  hook_ = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_LOCATIONCHANGE,
                          NULL, HookCallback, 0, 0, WINEVENT_OUTOFCONTEXT);
  hwnd_lens_ = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                            WS_CHILD | WS_VISIBLE | MS_SHOWMAGNIFIEDCURSOR, 0,
                            0, lens_width_, lens_height_, hwnd_host_, nullptr,
                            GetModuleHandle(nullptr), nullptr);
  zoom_async_ = std::async(&Program::SetZoom, this, current_zoom_);
  ToggleVisible();
  TimerCallback(hwnd_host_);
}

Program::~Program() { UnhookWinEvent(hook_); }

void Program::SetCaretPosition(long x, long y) {
  caret_position_.x = x;
  caret_position_.y = y;
}

void Program::SetZoom(double zoom_factor) {
  // Set to magnify by current factor
  MAGTRANSFORM matrix;
  memset(&matrix, 0, sizeof(matrix));
  matrix.v[0][0] = zoom_factor;
  matrix.v[1][1] = zoom_factor;
  matrix.v[2][2] = 1.0;

  current_zoom_ = zoom_factor;
  MagSetWindowTransform(hwnd_lens_, &matrix);
}

void Program::ToggleInverted() {
  invert_colors_ = !invert_colors_;
  if (invert_colors_) {
    SetWindowLongPtr(
        hwnd_lens_, GWL_STYLE,
        WS_CHILD | WS_VISIBLE | MS_SHOWMAGNIFIEDCURSOR | MS_INVERTCOLORS);
  } else {
    SetWindowLongPtr(hwnd_lens_, GWL_STYLE,
                     WS_CHILD | WS_VISIBLE | MS_SHOWMAGNIFIEDCURSOR);
  }
}

void Program::ToggleVisible(bool visible) {
  if (visible == hidden_) {
    ToggleVisible();
  }
}

void Program::ToggleVisible() {
  hidden_ = !hidden_;
  SetLayeredWindowAttributes(hwnd_host_, 0, hidden_ ? 0 : 255, LWA_ALPHA);
}

LRESULT Program::WindowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_HOTKEY:
      switch (wParam) {
        case 0:  // shift + alt + m
          disabled_ = !disabled_;
          ToggleVisible();
          break;
        case 1:  // shift + alt + l
          zoom_async_.get();
          zoom_async_ = std::async(std::launch::async, &Program::SetZoom, this,
                                   current_zoom_ / 0.75);
          break;
        case 2:  // shift + alt + k
          zoom_async_.get();
          zoom_async_ = std::async(std::launch::async, &Program::SetZoom, this,
                                   current_zoom_ * 0.75);
          break;
        case 3:  // shift + alt + p
          running_ = false;
          DestroyWindow(hwnd_host_);
          break;
        case 4:  // shift + alt + n
          ToggleInverted();
          break;
      }
      return 0;
    case WM_DESTROY:
      running_ = false;
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd_host_, uMsg, wParam, lParam);
  }
}

LRESULT Program::WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) {
  Program *program = (Program *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (program) {
    return program->WindowProcedure(uMsg, wParam, lParam);
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);  // Program not created yet
}

void Program::TimerProcedure(DWORD main_thread_id) {
  AttachThreadInput(main_thread_id, GetCurrentThreadId(), TRUE);
  while (running_) {  // Loop restarts every 16ms
    auto target_time =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(16);
    if (visible_caret_ && !hidden_) {
      // Center source rectangle around caret proportional to zoom factor
      RECT source_rect;
      source_rect.left =
          caret_position_.x - (int)((lens_width_ / 2) / current_zoom_);
      source_rect.top =
          caret_position_.y - (int)((lens_height_ / 2) / current_zoom_);
      source_rect.right = source_rect.left + (lens_width_ / current_zoom_);
      source_rect.bottom = source_rect.top + (lens_height_ / current_zoom_);

      MagSetWindowSource(hwnd_lens_, source_rect);

      // Center host window around caret
      SetWindowPos(hwnd_host_, HWND_TOPMOST,
                   caret_position_.x - (lens_width_ / 2),
                   caret_position_.y - (lens_height_ / 2), lens_width_,
                   lens_height_, SWP_NOACTIVATE);

      // Redraw magnification controller
      InvalidateRect(hwnd_lens_, NULL, TRUE);
    }
    std::this_thread::sleep_until(target_time);
  }
}

void Program::TimerCallback(HWND hwnd) {
  Program *program = (Program *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  DWORD thread_id = GetCurrentThreadId();
  std::thread(&Program::TimerProcedure, program, thread_id).detach();
}

void Program::HookProcedure(HWND hwnd, DWORD event, LONG object, LONG child) {
  if (!disabled_ && object == OBJID_CARET) {
    // Turn magnification on/off based on caret visibility
    if (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) {
      ToggleVisible(true);
      visible_caret_ = true;
    } else if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE) {
      ToggleVisible(false);
      visible_caret_ = false;
    } else if (event == EVENT_OBJECT_LOCATIONCHANGE) {  // New caret position
      IAccessible *accessible = nullptr;
      VARIANT variant;

      // Retrieve caret location in screen coordinates
      AccessibleObjectFromEvent(hwnd, object, child, &accessible, &variant);
      long caret_x;
      long caret_y;
      long caret_width;
      long caret_height;

      accessible->accLocation(&caret_x, &caret_y, &caret_width, &caret_height,
                              variant);

      SetCaretPosition(caret_x + caret_width, caret_y + (caret_height / 2));

      accessible->Release();
    }
  }
}

void CALLBACK Program::HookCallback(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                                    LONG object, LONG child, DWORD thread,
                                    DWORD time) {
  HookBinder(hwnd, event, object, child);
}