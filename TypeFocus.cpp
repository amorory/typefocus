#define _WIN32_WINNT 0x0500
#include <magnification.h>
#include <objbase.h>
#include <winreg.h>

#include <cstdlib>
#include <iostream>
#include <thread>

#include "Program.hpp"

void CALLBACK Update(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,
                    int nCmdShow) {
  // HKEY hkey;
  // RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\TypeFocus", 0,
  //                (LPWSTR) typeid(StateInfo).name(), REG_OPTION_NON_VOLATILE,
  //                KEY_ALL_ACCESS | KEY_READ, NULL, &hkey, NULL);
  // LPBYTE registry_data;
  // RegQueryValueEx(hkey, NULL, NULL, NULL, registry_data, NULL);
  StateInfo settings;

  MagInitialize();
  CoInitialize(nullptr);

  // Create the host window
  Program program(settings);

  // Create a magnifier control that fills the client area.
  program.CreateControl();

  ShowWindow(program.GetHwnd(), nCmdShow);
  UpdateWindow(program.GetHwnd());
  RegisterHotKey(program.GetHwnd(), 0, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4D);
  RegisterHotKey(program.GetHwnd(), 1, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4C);
  RegisterHotKey(program.GetHwnd(), 2, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4B);
  RegisterHotKey(program.GetHwnd(), 3, 0x4000 | MOD_ALT | MOD_SHIFT, 0x50);
  RegisterHotKey(program.GetHwnd(), 4, 0x4000 | MOD_ALT | MOD_SHIFT, 0x4E);

  // Create a timer to update the control.
  // UINT_PTR timerId = SetTimer(host.GetHwnd(), 0, 16, Update);

  // Main message loop
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Shut down.
  MagUninitialize();
  CoUninitialize();
  return (int)msg.wParam;
}
