
#include "framework.h"
#include "Switch Configurator.h"

#include <string>
#include <filesystem>
#include <wrl.h>
#include "WebView2.h"

#define MAX_LOADSTRING 100

// Variabili globali:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// WebView2 globals
ICoreWebView2Controller* g_webViewController = nullptr;
ICoreWebView2* g_webView = nullptr;

// Dichiarazioni in avanti delle funzioni incluse in questo modulo di codice:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitWebView(HWND hWnd);
std::wstring        GetUiFileUrl();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_SWITCHCONFIGURATOR, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SWITCHCONFIGURATOR));

  MSG msg;

  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  if (g_webView)
  {
    g_webView->Release();
    g_webView = nullptr;
  }

  if (g_webViewController)
  {
    g_webViewController->Release();
    g_webViewController = nullptr;
  }

  return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SWITCHCONFIGURATOR));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SWITCHCONFIGURATOR);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance;

  HWND hWnd = CreateWindowW(
    szWindowClass,
    szTitle,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0,
    CW_USEDEFAULT, 0,
    nullptr,
    nullptr,
    hInstance,
    nullptr
  );

  if (!hWnd)
  {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  InitWebView(hWnd);

  return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_SIZE:
    if (g_webViewController)
    {
      RECT bounds;
      GetClientRect(hWnd, &bounds);
      g_webViewController->put_Bounds(bounds);
    }
    return 0;

  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);

    switch (wmId)
    {
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  break;

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
  }
  break;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);

  switch (message)
  {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }

  return (INT_PTR)FALSE;
}

std::wstring GetUiFileUrl()
{
  wchar_t exePath[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, exePath, MAX_PATH);

  std::wstring path = exePath;

  // Rimuove nome exe
  size_t pos = path.find_last_of(L"\\/");
  if (pos != std::wstring::npos)
    path = path.substr(0, pos);

  // Risali: Debug -> x64
  for (int i = 0; i < 2; i++)
  {
    pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
      path = path.substr(0, pos);
  }

  // Ora siamo nella cartella del progetto
  path += L"\\ui\\index.html";

  // Converti in file:///
  std::wstring url = L"file:///";

  for (wchar_t ch : path)
  {
    if (ch == L'\\')
      url += L'/';
    else if (ch == L' ')
      url += L"%20";
    else
      url += ch;
  }

  return url;
}
void InitWebView(HWND hWnd)
{
  CreateCoreWebView2EnvironmentWithOptions(
    nullptr,
    nullptr,
    nullptr,
    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
      {
        if (FAILED(result) || env == nullptr)
        {
          MessageBoxW(hWnd, L"Creazione ambiente WebView2 fallita.", L"Errore", MB_ICONERROR);
          return E_FAIL;
        }

  return env->CreateCoreWebView2Controller(
    hWnd,
    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
      [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
      {
        if (FAILED(result) || controller == nullptr)
        {
          MessageBoxW(hWnd, L"Creazione controller WebView2 fallita.", L"Errore", MB_ICONERROR);
          return E_FAIL;
        }

  g_webViewController = controller;
  g_webViewController->AddRef();

  g_webViewController->get_CoreWebView2(&g_webView);

  RECT bounds;
  GetClientRect(hWnd, &bounds);
  g_webViewController->put_Bounds(bounds);
  g_webViewController->put_IsVisible(TRUE);

  g_webView->add_WebMessageReceived(
    Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
      {
        LPWSTR msg = nullptr;

  if (SUCCEEDED(args->TryGetWebMessageAsString(&msg)) && msg)
  {
    std::wstring received = msg;

    if (received == L"ping")
    {
      sender->PostWebMessageAsString(L"pong dal backend Win32");
    }
    else
    {
      std::wstring reply = L"ricevuto: " + received;
      sender->PostWebMessageAsString(reply.c_str());
    }

    CoTaskMemFree(msg);
  }

  return S_OK;
      }).Get(),
        nullptr);

  std::wstring url = GetUiFileUrl();
  g_webView->Navigate(url.c_str());

  return S_OK;
      }).Get());
      }).Get());
}
