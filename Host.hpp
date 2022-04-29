#pragma once

#include "Window.hpp"

class Host : public Window<Host> {
  static Host* host_;

 public:
  Host();
  PCWSTR ClassName() const { return L"TypeFocus"; }
  LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};