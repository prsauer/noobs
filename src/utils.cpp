#include <windows.h>
#include <iostream>

void print_cwd() {
  char cwd[MAX_PATH];
  GetCurrentDirectoryA(MAX_PATH, cwd);
  std::cout << "Current directory: " << cwd << std::endl;
}

void set_cwd(const char* path) {
  auto success = SetCurrentDirectoryA(path);
  
  if (!success) 
    throw "Failed to set CWD";
}

void print_exe_path() {
  wchar_t path_utf16[MAX_PATH];
  GetModuleFileNameW(NULL, path_utf16, MAX_PATH);
  std::wcout << L"Executable path: " << path_utf16 << std::endl;
}