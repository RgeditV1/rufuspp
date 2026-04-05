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

  /**
   * @brief Ejecuta un comando del sistema y captura su salida estándar.
   * @param cmd Comando a ejecutar.
   * @return std::string Salida del comando.
   * @throw std::runtime_error si no se puede abrir el pipe del comando.
   */
  inline std::string get_execute(const std::string &cmd) {
    std::string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      throw std::runtime_error("Error: No se pudo ejecutar el comando");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);
    return result;
  }