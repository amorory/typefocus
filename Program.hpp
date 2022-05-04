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

  // Sets caret_position_ to (x, y)
  void SetCaretPosition(long x, long y);
  // Sets current_zoom_ and updates magnification matrix
  void SetZoom(double zoom_factor);
  // Sets or removes MS_INVERTCOLORS for the magnification control
  void ToggleInverted();
  // Calls ToggleInverted() if hidden_ does not match visible
  void ToggleVisible(bool visible);
  // Toggles hidden_ and window transparency
  void ToggleVisible();

  // Handles window messages
  LRESULT CALLBACK WindowProcedure(UINT msg, WPARAM wParam, LPARAM lParam);
  // Updates magnification source every 16ms ~ 60hz
  void TimerProcedure(DWORD main_thread_id);
  // Detects caret creation, deletion, and movement
  void HookProcedure(HWND hwnd, DWORD event, LONG object, LONG child);
  // Allows HookProcedure to be called from a static context
  static std::function<void(HWND, DWORD, LONG, LONG)> HookBinder;

 public:
  Program();
  ~Program();
  // Creates the magnification control window
  void CreateControl();

  HWND GetHwnd() const { return hwnd_host_; };

  static LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam);
  static void CALLBACK TimerCallback(HWND hwnd);
  static void CALLBACK HookCallback(HWINEVENTHOOK hEvent, DWORD event,
                                    HWND hwndMsg, LONG idObject, LONG idChild,
                                    DWORD idThread, DWORD dwmsEventTime);
};