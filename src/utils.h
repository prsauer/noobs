#pragma once

void print_cwd();
void print_exe_path();
void set_cwd(const char* path);
void log_handler(int lvl, const char *msg, va_list args, void *p);