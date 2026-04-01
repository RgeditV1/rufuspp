#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

/**
 * @brief Executes a system command and handles errors.
 *
 * @param cmd The command string to execute.
 * @return true if the command was successful (exit status 0).
 * @return false otherwise.
 */
inline bool execute(const std::string &cmd) {
  int result = std::system(cmd.c_str());
  if (result != 0) {
    std::cerr << "Error: Command failed: " << cmd << " (status: " << result
              << ")" << std::endl;
    return false;
  }
  return true;
}
