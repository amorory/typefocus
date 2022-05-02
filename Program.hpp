#pragma once
#define UNICODE

#include <magnification.h>
#include <oleacc.h>
#include <windows.h>

#include <functional>

struct StateInfo {
  int lens_width = 300;
  int lens_height = 100;
  float magnification_factor = 2.0f;
  bool invert_colors = false;
};

class Program {
  static Program* host_;

  HWND hwnd_host_;
  HWND hwnd_start_;
  HWND hwnd_lens_;
  HWINEVENTHOOK hook_;
  UINT_PTR timer_;

  POINT caret_position_;
  int lens_width_;
  int lens_height_;
  float current_zoom_;
  bool magnify_;
  bool invert_colors_;

  LRESULT CALLBACK WindowProcedure(UINT msg, WPARAM wParam, LPARAM lParam);
  void TimerProcedure();
  void HookProcedure(HWND hwnd, DWORD event, LONG object, LONG child,
                     DWORD thread);

  static std::function<void(HWND, DWORD, LONG, LONG, DWORD)> HookBinder;

 public:
  Program(StateInfo settings);
  ~Program();
  void CreateControl();

  void SetZoom(float zoom_factor_);

  HWND HwndHost() { return hwnd_host_; };
  HWND HwndLens() { return hwnd_lens_; };

  static LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam);
  static void CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                                     DWORD dwTime);
  static void CALLBACK HookCallback(HWINEVENTHOOK hEvent, DWORD event,
                                    HWND hwndMsg, LONG idObject, LONG idChild,
                                    DWORD idThread, DWORD dwmsEventTime);
};