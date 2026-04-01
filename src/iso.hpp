/*
 * Este archivo se encarga de manejar las validaciones de archios iso
 * De esta manera detectara que tipo de imagen estamos usando
 */

#pragma once

// #include <iostream>
#include <string>
#include <vector>

class Iso {
private:
  enum class IsoType { WINDOWS, LINUX, MAC };

  struct IsoInfo {
    std::string name;
    std::string version;
    std::string isoPath;
    std::string architecture;
    IsoType type;
  };

  std::vector<IsoInfo> myIso;

public:
  Iso() = default;
  ~Iso() = default;

  void addIsoInfo(const IsoInfo &iso);
  std::vector<IsoInfo> getIsoInfo() const;

  static void validateIso();
};