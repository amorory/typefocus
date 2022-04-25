#pragma once
#include "BaseWindow.hpp"

class MainWindow : public BaseWindow<MainWindow> {
 public:
  PCWSTR ClassName() const { return L"MainWindow"; }
  LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};