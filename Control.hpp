#include <map>

#include "Host.hpp"

class Control {
 private:
  static Control* control_;
  HWND hwnd_;
  HWND hwnd_parent_;
  HWINEVENTHOOK hook_;
  float current_zoom_;
  int lens_width_;
  int lens_height_;
  POINT caret_position_;
  bool magnifying_;

 public:
  Control(HWND host);
  ~Control();
  HWND GetHwnd() const { return hwnd_; }
  void SetZoom(float zoom_factor_);
  void Update(long x, long y);
  static void CALLBACK Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent,
                              DWORD dwTime);
  static void CALLBACK CaretHook(HWINEVENTHOOK hEvent, DWORD event,
                                 HWND hwndMsg, LONG idObject, LONG idChild,
                                 DWORD idThread, DWORD dwmsEventTime);
};