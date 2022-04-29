#include "Control.hpp"

#include <magnification.h>
#include <oleacc.h>
#include <windows.h>

#include <iostream>

Control* Control::control_ = nullptr;

Control::Control(HWND parent) {
  control_ = this;
  hwnd_parent_ = parent;
  current_zoom_ = 2.0f;
  lens_width_ = 300;
  lens_height_ = 100;
  magnifying_ = true;
  hook_ = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE,
                          EVENT_OBJECT_LOCATIONCHANGE, NULL,
                          (WINEVENTPROC)CaretHook, 0, 0, WINEVENT_OUTOFCONTEXT);
  hwnd_ = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                       WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE, 0, 0,
                       lens_width_, lens_height_, hwnd_parent_, nullptr,
                       GetModuleHandle(nullptr), nullptr);
  UINT_PTR timerId = SetTimer(hwnd_parent_, 0, 16, Update);
}

Control::~Control() { UnhookWinEvent(hook_); }

void Control::SetZoom(float zoom_factor) { current_zoom_ = zoom_factor; }

void Control::Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
  if (control_->magnifying_) {
    SetLayeredWindowAttributes(control_->hwnd_parent_, 0, 255, LWA_ALPHA);

    int border_width = GetSystemMetrics(SM_CXFIXEDFRAME);
    int caption_height = GetSystemMetrics(SM_CYCAPTION);

    RECT source_rect;
    source_rect.left =
        control_->caret_position_.x -
        (((control_->lens_width_ / 2)) / control_->current_zoom_);
    source_rect.top = control_->caret_position_.y -
                      (((control_->lens_height_ / 2) + caption_height) /
                       control_->current_zoom_);
    source_rect.right =
        source_rect.left + (control_->lens_width_ / control_->current_zoom_);
    source_rect.bottom =
        source_rect.top + (control_->lens_height_ / control_->current_zoom_);

    MagSetWindowSource(control_->hwnd_, source_rect);

    // Move the host window to be centered with the caret
    SetWindowPos(control_->hwnd_parent_, HWND_TOPMOST,
                 control_->caret_position_.x - (control_->lens_width_ / 2),
                 control_->caret_position_.y - (control_->lens_height_ / 2),
                 control_->lens_width_, control_->lens_height_, SWP_NOACTIVATE);

    // Force the magnifier control to redraw itself.
    InvalidateRect(control_->hwnd_, NULL, TRUE);
  } else {
    SetLayeredWindowAttributes(control_->hwnd_parent_, 0, 0, LWA_ALPHA);
  }
}

void CALLBACK Control::CaretHook(HWINEVENTHOOK hEvent, DWORD event,
                                 HWND hwndMsg, LONG idObject, LONG idChild,
                                 DWORD idThread, DWORD dwmsEventTime) {
  if (idObject == OBJID_CARET) {
    if (event == EVENT_OBJECT_SHOW) {
      control_->magnifying_ = true;
    }
    if (event == EVENT_OBJECT_HIDE) {
      control_->magnifying_ = false;
    }
    if (event == EVENT_OBJECT_LOCATIONCHANGE) {
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