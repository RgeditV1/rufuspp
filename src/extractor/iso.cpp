#include "iso.hpp"
#include "../execute.hpp"
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <climits>
#include <unistd.h>

/**
 * @brief Resuelve la ruta de 7z.so en tiempo de ejecución.
 *
 * Orden de búsqueda:
 *   1. Directorio del propio ejecutable (copia de thirdparty copiada por CMake).
 *   2. Rutas estándar del sistema (p7zip / 7zip).
 * @return Ruta absoluta al 7z.so encontrado.
 * @throws std::runtime_error si no se pudo localizar la biblioteca.
 */
static std::string find7zLib() {
    // 1. Junto al ejecutable (copia local del thirdparty)
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0) {
        exePath[len] = '\0';
        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
        std::filesystem::path local = exeDir / "7z.so";
        if (std::filesystem::exists(local))
            return local.string();
    }

    // 2. Rutas estándar del sistema
    static const char* systemPaths[] = {
        "/usr/lib/7zip/7z.so",
        "/usr/lib/p7zip/7z.so",
        "/usr/lib64/7zip/7z.so",
        "/usr/lib64/p7zip/7z.so",
        "/usr/lib64/7z.so",
        nullptr
    };
    for (const char** p = systemPaths; *p != nullptr; ++p) {
        if (std::filesystem::exists(*p))
            return *p;
    }

    throw std::runtime_error(
        "No se encontró 7z.so. Instala p7zip/7zip o coloca 7z.so junto al ejecutable.");
}

bool Iso::addIsoInfo(const std::string &isoPath, IsoType type) {
  // construimos los comandos con iso info

  this->info.isoPath = isoPath;
  this->info.type = runValidation(isoPath, type);

  if (this->info.type != "ERROR") {

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
      this->info.volume_id = result.substr(start, end - start);
    }

    check_boot_compatibility(isoPath);

    this->myIso.push_back(this->info);
    return true;
  }
  return false;
}

bool Iso::checkSignature(const std::string &isoPath,
                         const std::vector<std::string> &signatures,
                         const bit7z::BitInFormat &format) {

  try {
    bit7z::Bit7zLibrary lib{find7zLib()};
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
    if (isWindows(isoPath)) {
      return "WINDOWS";
    } else if (isArch(isoPath)) {
      return "ARCH";
    } else if (isDebianUbuntu(isoPath)) {
      return "DEBIAN/UBUNTU";
    }
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
  if (checkSignature(isoPath, WindowsSignatures::data_files,
                     bit7z::BitFormat::Udf)) {
    return true;
  }
  return checkSignature(isoPath, WindowsSignatures::data_files,
                        bit7z::BitFormat::Iso);
}

bool Iso::isDebianUbuntu(const std::string &isoPath) {
  std::cout << "[DEBUG] Checking for Debian/Ubuntu signatures..." << std::endl;
  return checkSignature(isoPath, DebianUbuntuSignatures::files,
                        bit7z::BitFormat::Iso);
}

bool Iso::isArch(const std::string &isoPath) {
  std::cout << "[DEBUG] Checking for Arch Linux signatures..." << std::endl;
  return checkSignature(isoPath, ArchSignatures::files,
                        bit7z::BitFormat::Iso);
}

void Iso::check_boot_compatibility(const std::string &isoPath) {
  auto scan = [&](const bit7z::BitInFormat &format, bool &has_uefi,
                  bool &has_bios) {
    try {
      bit7z::Bit7zLibrary lib{find7zLib()};
      bit7z::BitArchiveReader reader{lib, isoPath, format};

      for (const auto &item : reader.items()) {
        std::string p = item.path();

        // Normalización de ruta
        std::replace(p.begin(), p.end(), '\\', '/');
        if (!p.empty() && p[0] == '/')
          p.erase(0, 1);
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);

        // --- DETECCIÓN UEFI ---
        if (!has_uefi &&
            (p.find("efi/boot/bootx64.efi") != std::string::npos ||
             p.find("efi/boot/bootia32.efi") != std::string::npos ||
             p.find("efi/boot/bootaa64.efi") != std::string::npos ||
             p.find("efi/boot/grubx64.efi") != std::string::npos)) {
          has_uefi = true;
        }

        // --- DETECCIÓN BIOS (Legacy) ---
        if (!has_bios &&
            (p.find("bootmgr") != std::string::npos ||
             p.find("boot/bcd") != std::string::npos ||
             p.find("isolinux/isolinux.bin") != std::string::npos ||
             p.find("boot/grub/i386-pc") != std::string::npos)) {
          has_bios = true;
        }

        if (has_uefi && has_bios)
          break;
      }
    } catch (const bit7z::BitException &ex) {
      std::cerr << "[Bit7z Error] " << ex.what() << std::endl;
    }
  };

  bool has_uefi = false;
  bool has_bios = false;

  // Preferimos UDF (común en ISOs Windows), pero hacemos fallback a ISO si falta.
  scan(bit7z::BitFormat::Udf, has_uefi, has_bios);
  if (!(has_uefi && has_bios)) {
    scan(bit7z::BitFormat::Iso, has_uefi, has_bios);
  }

  if (has_uefi && has_bios) {
    this->info.bootType = "Dual (UEFI + BIOS)";
  } else if (has_uefi) {
    this->info.bootType = "UEFI";
  } else if (has_bios) {
    this->info.bootType = "BIOS/Legacy";
  } else {
    this->info.bootType = "Unknown / No Booteable";
  }
}
