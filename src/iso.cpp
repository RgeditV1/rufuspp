#include "iso.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

std::string Iso::addIsoInfo(const std::string &isoPath) {
  // construimos los comandos con iso info

  IsoInfo info;
  info.isoPath = isoPath;

  std::string command =
      "isoinfo -d -i '" + isoPath + "' | grep -E 'Volume id:|Publisher id:'";
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

  // 2. Extraer Publisher
  size_t pPos = result.find("Publisher id:");
  if (pPos != std::string::npos) {
    size_t start = pPos + 14;              // 14 es el largo de "Publisher id: "
    size_t end = result.find('\n', start); // Buscamos el final de esa línea
    info.publisher = result.substr(start, end - start);
  }

  info.type = getType(info.publisher, info.volume_id);

  if (isWindows(isoPath)) {
    myIso.push_back(info);
    return "WINDOWS";
  }
  return "ERROR";
}

std::string Iso::getType(const std::string &publisher,
                         const std::string &volume_id) {
  auto to_upper = [](std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  };

  std::string p = to_upper(publisher);
  std::string v = to_upper(volume_id);

  // Windows
  if (p.find("MICROSOFT") != std::string::npos ||
      v.find("WIN") != std::string::npos ||
      v.find("CCCOMA") != std::string::npos) {
    return "WINDOWS";
  }

  // Ubuntu
  if (p.find("CANONICAL") != std::string::npos ||
      v.find("UBUNTU") != std::string::npos) {
    return "UBUNTU";
  }

  // Arch
  if (p.find("ARCH") != std::string::npos ||
      v.find("ARCH") != std::string::npos) {
    return "ARCH";
  }

  // Debian
  if (p.find("DEBIAN") != std::string::npos ||
      v.find("DEBIAN") != std::string::npos) {
    return "DEBIAN";
  }

  // Fedora
  if (p.find("FEDORA") != std::string::npos ||
      v.find("FEDORA") != std::string::npos) {
    return "FEDORA";
  }

  // Mac
  if (p.find("APPLE") != std::string::npos ||
      v.find("MAC") != std::string::npos) {
    return "MAC";
  }

  return "UNKNOWN";
}

bool Iso::isWindows(const std::string &isoPath) {
  try {
    bit7z::Bit7zLibrary lib{"/usr/lib/7zip/7z.so"};
    bit7z::BitArchiveReader reader{lib, isoPath, bit7z::BitFormat::Iso};

    auto items = reader.items();
    for (const auto &item : items) {
      std::string path = item.path();
      // standard paths for windows installation images
      if (path == "sources/install.wim" || path == "sources/install.esd" ||
          path == "SOURCES/INSTALL.WIM" || path == "SOURCES/INSTALL.ESD") {
        return true;
      }
    }
  } catch (const bit7z::BitException &ex) {
    std::cerr << "Bit7z Error: " << ex.what() << std::endl;
    return false;
  }
  return false;
}
