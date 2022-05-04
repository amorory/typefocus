#pragma once
#define UNICODE

#include <magnification.h>
#include <oleacc.h>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

struct StateInfo {
  int lens_width = 300;
  int lens_height = 100;
  float magnification_factor = 2.0f;
  bool invert_colors = false;
};

class Program {
  HWND hwnd_host_;
  HWND hwnd_start_;
  HWND hwnd_lens_;
  HWINEVENTHOOK hook_;

  std::mutex location_mutex_;
  std::mutex drawing_mutex_;
  std::future<void> zoom_async_;
  std::thread timer_thread_;

  POINT caret_position_;
  int lens_width_;
  int lens_height_;
  double current_zoom_;

  std::atomic<bool> running_;
  bool visible_caret_;
  bool invert_colors_;
  bool hidden_;
  bool disabled_;

  void SetCaretPosition(long x, long y);
  void SetZoom(double zoom_factor_);
  void ToggleInverted();
  void ToggleVisible(bool visible);
  void ToggleVisible();

  LRESULT CALLBACK WindowProcedure(UINT msg, WPARAM wParam, LPARAM lParam);
  void TimerProcedure(DWORD main_thread_id);
  void HookProcedure(HWND hwnd, DWORD event, LONG object, LONG child);
  static std::function<void(HWND, DWORD, LONG, LONG)> HookBinder;

 public:
  Program(StateInfo settings);
  ~Program();
  void CreateControl();

  HWND GetHwnd() const { return hwnd_host_; };

  static LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam);
  static void CALLBACK TimerCallback(HWND hwnd);
  static void CALLBACK HookCallback(HWINEVENTHOOK hEvent, DWORD event,
                                    HWND hwndMsg, LONG idObject, LONG idChild,
                                    DWORD idThread, DWORD dwmsEventTime);
};