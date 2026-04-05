#include "iso.hpp"
#include "../execute.hpp"
#include <iostream>

std::string Iso::addIsoInfo(const std::string &isoPath, IsoType type) {
  // construimos los comandos con iso info

  IsoInfo info;
  info.isoPath = isoPath;
  info.type = runValidation(isoPath, type);

  if (info.type != "ERROR") {
    std::string command =
        "isoinfo -d -i '" + isoPath + "' | grep -E 'Volume id'";
    std::string result = get_execute(command);
    if (result.empty()) {
      throw std::runtime_error(
          "Error: No se pudo obtener la informacion del iso");
    }

    // 1. Extraer Volume id
    size_t vPos = result.find("Volume id:");
    if (vPos != std::string::npos) {
      size_t start = vPos + 11;              // 11 es el largo de "Volume id: "
      size_t end = result.find('\n', start); // Buscamos el final de esa línea
      info.volume_id = result.substr(start, end - start);
    }
    myIso.push_back(info);
    return "OK";
  }
  return "ERROR";
}

bool Iso::checkSignature(const std::string &isoPath,
                         const std::vector<std::string> &signatures,
                         const bit7z::BitInFormat &format) {

  try {
    bit7z::Bit7zLibrary lib{"/usr/lib/7zip/7z.so"};
    bit7z::BitArchiveReader reader{lib, isoPath, format};

    for (const auto &item : reader.items()) {
      std::string p = item.path();

      // Normalización de ruta
      std::replace(p.begin(), p.end(), '\\', '/');
      if (!p.empty() && p[0] == '/')
        p.erase(0, 1);
      std::transform(p.begin(), p.end(), p.begin(), ::tolower);

      for (const auto &sig : signatures) {
        std::string lowSig = sig;
        std::transform(lowSig.begin(), lowSig.end(), lowSig.begin(), ::tolower);

        // Si la firma termina en '/', buscamos si la ruta empieza con eso (es
        // una carpeta)
        if (lowSig.back() == '/') {
          if (p.find(lowSig) == 0)
            return true;
        }
        // Si no, buscamos coincidencia exacta de archivo
        else if (p == lowSig) {
          return true;
        }
      }
    }
  } catch (const bit7z::BitException &ex) {
    std::cerr << "[Bit7z Error] " << ex.what() << std::endl;
  }
  return false;
}

std::string Iso::runValidation(const std::string &isoPath, IsoType type) {
  switch (type) {
  case IsoType::AUTO:
    if (isWindows(isoPath))
      return "WINDOWS";
    if (isArch(isoPath))
      return "ARCH";
    if (isDebianUbuntu(isoPath))
      return "DEBIAN/UBUNTU";
    return "ERROR";
  case IsoType::WINDOWS:
    return isWindows(isoPath) ? "WINDOWS" : "ERROR";
  case IsoType::ARCH:
    return isArch(isoPath) ? "ARCH" : "ERROR";
  case IsoType::DEBIAN_UBUNTU:
    return isDebianUbuntu(isoPath) ? "DEBIAN/UBUNTU" : "ERROR";
  default:
    return "ERROR";
  }
}

bool Iso::isWindows(const std::string &isoPath) {
  std::cout << "[DEBUG] Checking for Windows signatures..." << std::endl;
  // Prefer UDF (common on Windows ISOs), but fall back to ISO if UDF isn't supported
  if (checkSignature(isoPath, WindowsSignatures::files,
                     bit7z::BitFormat::Udf)) {
    return true;
  }
  return checkSignature(isoPath, WindowsSignatures::files,
                        bit7z::BitFormat::Iso);
}

bool Iso::isDebianUbuntu(const std::string &isoPath) {
  std::cout << "[DEBUG] Checking for Debian/Ubuntu signatures..." << std::endl;
  return checkSignature(isoPath, DebianUbuntuSignatures::files,
                        bit7z::BitFormat::Iso);
}

bool Iso::isArch(const std::string &isoPath) {
  std::cout << "[DEBUG] Checking for Arch Linux signatures..." << std::endl;
  // Arch usa ISO9660 para sus imágenes oficiales
  return checkSignature(isoPath, ArchSignatures::files, bit7z::BitFormat::Iso);
}