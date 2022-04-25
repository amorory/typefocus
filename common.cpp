BOOL CreateMagnifier(HINSTANCE hInstance) {
  // Register the host window class.
  WNDCLASSEX wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = 0;
  wcex.lpfnWndProc = HostWndProc;
  wcex.hInstance = hInstance;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
  wcex.lpszClassName = WindowClassName;

  if (RegisterClassEx(&amp; wcex) == 0) return FALSE;

  // Create the host window.
  hwndHost = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
                            WindowClassName, WindowTitle, WS_CLIPCHILDREN, 0, 0,
                            0, 0, NULL, NULL, hInstance, NULL);
  if (!hwndHost) {
    return FALSE;
  }

  // Make the window opaque.
  SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);

  // Create a magnifier control that fills the client area.
  hwndMag =
      CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
                   WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE, 0, 0,
                   LENS_WIDTH, LENS_HEIGHT, hwndHost, NULL, hInstance, NULL);
  if (!hwndMag) {
    return FALSE;
  }

  return TRUE;
}