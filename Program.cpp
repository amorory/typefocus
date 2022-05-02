#include "Program.hpp"

#include <iostream>

Program* Program::host_ = nullptr;

std::function<void(HWND, DWORD, LONG, LONG, DWORD)> Program::HookBinder;

Program::Program(StateInfo settings)
    : lens_width_(settings.lens_width),
      lens_height_(settings.lens_height),
      current_zoom_(settings.magnification_factor),
      invert_colors_(settings.invert_colors) {
  host_ = this;

  HookBinder = std::bind(&Program::HookProcedure, this, std::placeholders::_1,
                         std::placeholders::_2, std::placeholders::_3,
                         std::placeholders::_4, std::placeholders::_5);

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

Program::~Program() {
  UnhookWinEvent(hook_);
  KillTimer(NULL, timer_);
}

void Program::CreateControl() {
  hook_ = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_LOCATIONCHANGE,
                          NULL, HookCallback, 0, 0, WINEVENT_OUTOFCONTEXT);
  hwnd_lens_ = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                            WS_CHILD | MS_CLIPAROUNDCURSOR | WS_VISIBLE, 0, 0,
                            lens_width_, lens_height_, hwnd_host_, nullptr,
                            GetModuleHandle(nullptr), nullptr);

  SetZoom(current_zoom_);
  timer_ = SetTimer(hwnd_host_, 0, 16, TimerCallback);
}

void Program::SetZoom(float zoom_factor) {
  current_zoom_ = zoom_factor;

  // Set to magnify by current factor
  MAGTRANSFORM matrix;
  memset(&matrix, 0, sizeof(matrix));
  matrix.v[0][0] = current_zoom_;
  matrix.v[1][1] = current_zoom_;
  matrix.v[2][2] = 1.0f;

  MagSetWindowTransform(hwnd_lens_, &matrix);
}

LRESULT Program::WindowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd_host_, uMsg, wParam, lParam);
  }
}

void Program::TimerProcedure() {
  if (magnify_) {
    // Make host opaque
    SetLayeredWindowAttributes(hwnd_host_, 0, 255, LWA_ALPHA);

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
  } else {
    // Make host transparent and redraw controller
    SetLayeredWindowAttributes(hwnd_host_, 0, 0, LWA_ALPHA);
    InvalidateRect(hwnd_lens_, NULL, TRUE);
  }
}

void Program::HookProcedure(HWND hwnd, DWORD event, LONG object, LONG child,
                            DWORD thread) {
  if (object == OBJID_CARET) {
    // Turn magnification on/off based on caret visibility
    if (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) {
      magnify_ = true;
    } else if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE) {
      magnify_ = false;
    } else if (event == EVENT_OBJECT_LOCATIONCHANGE) {  // New caret position
      IAccessible* accessible = nullptr;
      VARIANT variant;

      // Retrieve caret location in screen coordinates
      AccessibleObjectFromEvent(hwnd, object, child, &accessible, &variant);
      RECT caret_rect;
      accessible->accLocation(&caret_rect.left, &caret_rect.top,
                              &caret_rect.right, &caret_rect.bottom, variant);

      caret_position_.x = caret_rect.left;
      caret_position_.y =
          caret_rect.top + (caret_rect.bottom / 2);  // Account for caret height

      accessible->Release();
    }
  }
}

LRESULT Program::WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) {
  Program* program = (Program*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (program) {
    return program->WindowProcedure(uMsg, wParam, lParam);
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);  // Program not created yet
}

void Program::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                            DWORD dwTime) {
  Program* program = (Program*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (program) {
    program->TimerProcedure();
  }
}

void CALLBACK Program::HookCallback(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                                    LONG object, LONG child, DWORD thread,
                                    DWORD time) {
  HookBinder(hwnd, event, object, child, thread);
}