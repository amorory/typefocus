#include "Control.hpp"

#include <magnification.h>
#include <oleacc.h>
#include <windows.h>

Control* Control::control_ = nullptr;

Control::Control(HWND parent) {
  control_ = this;
  hwnd_parent_ = parent;
  lens_width_ = 300;
  lens_height_ = 100;
  magnifying_ = true;
  caret_position_.x = 0;
  caret_position_.y = 0;
  hook_ =
      SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_LOCATIONCHANGE, NULL,
                      (WINEVENTPROC)HookProcess, 0, 0, WINEVENT_OUTOFCONTEXT);
  hwnd_ = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                       WS_CHILD | MS_CLIPAROUNDCURSOR | WS_VISIBLE, 0, 0,
                       lens_width_, lens_height_, hwnd_parent_, nullptr,
                       GetModuleHandle(nullptr), nullptr);
  SetZoom(2.0f);
  UINT_PTR timerId = SetTimer(hwnd_parent_, 0, 16, TimerProcess);
}

Control::~Control() { UnhookWinEvent(hook_); }

void Control::SetZoom(float zoom_factor) {
  current_zoom_ = zoom_factor;

  MAGTRANSFORM matrix;
  memset(&matrix, 0, sizeof(matrix));
  matrix.v[0][0] = current_zoom_;
  matrix.v[1][1] = current_zoom_;
  matrix.v[2][2] = 1.0f;

  MagSetWindowTransform(hwnd_, &matrix);
}

void Control::Update() {
  if (magnifying_) {
    SetLayeredWindowAttributes(hwnd_parent_, 0, 255, LWA_ALPHA);

    RECT source_rect;
    source_rect.left =
        caret_position_.x - (int)((lens_width_ / 2) / current_zoom_);
    source_rect.top =
        caret_position_.y - (int)((lens_height_ / 2) / current_zoom_);
    source_rect.right = source_rect.left + (lens_width_ / current_zoom_);
    source_rect.bottom = source_rect.top + (lens_height_ / current_zoom_);

    MagSetWindowSource(hwnd_, source_rect);

    // Move the host window to be centered with the caret
    SetWindowPos(hwnd_parent_, HWND_TOPMOST,
                 caret_position_.x - (lens_width_ / 2),
                 caret_position_.y - (lens_height_ / 2), lens_width_,
                 lens_height_, SWP_NOACTIVATE);

    // Force the magnifier control to redraw itself.
    InvalidateRect(hwnd_, NULL, TRUE);
  } else {
    SetLayeredWindowAttributes(hwnd_parent_, 0, 0, LWA_ALPHA);
    InvalidateRect(hwnd_, NULL, TRUE);
  }
}

void Control::TimerProcess(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                           DWORD dwTime) {
  control_->Update();
}

void CALLBACK Control::HookProcess(HWINEVENTHOOK hEvent, DWORD event,
                                   HWND hwndMsg, LONG idObject, LONG idChild,
                                   DWORD idThread, DWORD dwmsEventTime) {
  if (idObject == OBJID_CARET) {
    if (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) {
      control_->magnifying_ = true;
    } else if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE) {
      control_->magnifying_ = false;
    } else if (event == EVENT_OBJECT_LOCATIONCHANGE) {
      IAccessible* pAcc = nullptr;
      VARIANT varChild;

      HRESULT hr = AccessibleObjectFromEvent(hwndMsg, idObject, idChild, &pAcc,
                                             &varChild);
      if (SUCCEEDED(hr)) {
        RECT caret_rect;
        hr = pAcc->accLocation(&caret_rect.left, &caret_rect.top,
                               &caret_rect.right, &caret_rect.bottom, varChild);
        if (SUCCEEDED(hr)) {
          control_->caret_position_.x = caret_rect.left;
          control_->caret_position_.y =
              caret_rect.top + (caret_rect.bottom / 2);
        }

        pAcc->Release();
      }
    }
  }
}