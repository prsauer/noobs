#include <iostream>
#include <obs.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

void log_handler(int lvl, const char *msg, va_list args, void *p) {
  // Use the passed log path parameter
  static std::string log_filename;
  static bool filename_initialized = false;
  
  if (!filename_initialized) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream filename_stream;
    filename_stream << "ObsEngine-" << std::put_time(std::localtime(&time_t), "%Y-%m-%d") << ".log";
    
    // Use the provided directory path and append the filename
    std::string log_dir = static_cast<const char*>(p);

    // Ensure the directory path ends with a separator
    if (!log_dir.empty() && log_dir.back() != '\\' && log_dir.back() != '/') {
      log_dir += "\\";
    }

    log_filename = log_dir + filename_stream.str();
    filename_initialized = true;
  }
  
  // Get current timestamp
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    now.time_since_epoch()) % 1000;
  
  // Convert log level to string
  const char* level_str;
  switch (lvl) {
    case LOG_ERROR:   level_str = "ERROR"; break;
    case LOG_WARNING: level_str = "WARN";  break;
    case LOG_INFO:    level_str = "INFO";  break;
    case LOG_DEBUG:   level_str = "DEBUG"; break;
    default:          level_str = "UNKNOWN"; break;
  }
  
  // Format the log message
  char buffer[4096];
  vsnprintf(buffer, sizeof(buffer), msg, args);
  
  // Create timestamp string
  std::stringstream timestamp;
  timestamp << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
  timestamp << "." << std::setfill('0') << std::setw(3) << ms.count();
  
  std::string timestamp_str = "[" + timestamp.str() + "] [" + level_str + "] ";
  
  // Split the message by newlines and add timestamp to each line
  std::string message(buffer);
  std::istringstream iss(message);
  std::string line;
  std::string formatted_output;
  
  while (std::getline(iss, line)) {
    formatted_output += timestamp_str + line + "\n";
  }
  
  // Remove the last newline if it exists
  if (!formatted_output.empty() && formatted_output.back() == '\n') {
    formatted_output.pop_back();
  }
  
  // Write to log file
  std::ofstream logFile(log_filename, std::ios::app);
  if (logFile.is_open()) {
    logFile << formatted_output << std::endl;
    logFile.close();
  }
}