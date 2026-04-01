/*
 * Este archivo se encarga de manejar las validaciones de archios iso
 * De esta manera detectara que tipo de imagen estamos usando
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

class Iso {
private:
#ifdef ISO_TYPE
  // Enum para identificar el tipo de imagen
  // TODO: Agregar mas tipos de imagenes en el futuro
  enum class IsoType { WINDOWS, UBUNTU, ARCH, DEBIAN, FEDORA, MAC, UNKNOWN };
#endif

  struct IsoInfo {
    std::string volume_id; // == name
    std::string publisher;
    std::string isoPath;
    std::string architecture;
    std::string getType() const {
      std::string p = publisher;
      std::transform(p.begin(), p.end(), p.begin(),
                     [](unsigned char c) { return std::toupper(c); });
      return p.find("MICROSOFT") != std::string::npos   ? "WINDOWS"
             : p.find("CANONICAL") != std::string::npos ? "UBUNTU"
             : p.find("ARCH") != std::string::npos      ? "ARCH"
             : p.find("DEBIAN") != std::string::npos    ? "DEBIAN"
             : p.find("FEDORA") != std::string::npos    ? "FEDORA"
             : p.find("APPLE") != std::string::npos     ? "MAC"
                                                        : "UNKNOWN";
    }
  };

  std::vector<IsoInfo> myIso;

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

public:
  Iso() = default;
  ~Iso() = default;

  void addIsoInfo(const std::string &isoPath);
  inline std::vector<IsoInfo> getIsoInfo() const { return myIso; }

  void validateIso();
};