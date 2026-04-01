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
  // Enum para identificar el tipo de imagen
  // TODO: Agregar mas tipos de imagenes en el futuro
  enum class IsoType { WINDOWS, UBUNTU, ARCH, DEBIAN, FEDORA, MAC, UNKNOWN };

  struct IsoInfo {
    std::string name;
    std::string volume_id;
    std::string publisher;
    std::string version;
    std::string isoPath;
    std::string architecture;
    IsoType type =
        publisher.find("Microsoft") != std::string::npos   ? IsoType::WINDOWS
        : publisher.find("Canonical") != std::string::npos ? IsoType::UBUNTU
        : publisher.find("Arch") != std::string::npos      ? IsoType::ARCH
        : publisher.find("Debian") != std::string::npos    ? IsoType::DEBIAN
        : publisher.find("Fedora") != std::string::npos    ? IsoType::FEDORA
        : publisher.find("Apple") != std::string::npos     ? IsoType::MAC
                                                           : IsoType::UNKNOWN;
  };

  std::vector<IsoInfo> myIso;

public:
  Iso() = default;
  ~Iso() = default;

  void addIsoInfo(const IsoInfo &iso);
  std::vector<IsoInfo> getIsoInfo() const;

  virtual void validateIso() = 0;
};