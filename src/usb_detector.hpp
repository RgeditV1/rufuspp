#pragma once

#include <cstdint>
#include <string>
#include <vector>

class UsbDetector {
public:
  struct Device {
    // Informacion del dispositivo Duhhhh!!!
    std::string path;
    std::string name;
    std::string manufacturer;
    std::string mountPoint;
    std::string fileSystem;
    std::string size;
    std::string label;
    std::string deviceType; // e.g(USB Flash Drive, USB Hard Drive)
  };

protected:
  struct Size {
    static const uint64_t KB = 1024;
    static const uint64_t MB = 1024 * KB;
    static const uint64_t GB = 1024 * MB;
    static const uint64_t TB = 1024 * GB;
    static const uint64_t PB = 1024 * TB;
  };

  /* Convierte el tamaño de bloques a un tamaño legible
   * @param size_blocks: tamaño del dispositivo en bloques de 512 bytes
   * @return: std::string: tamaño del dispositivo en un formato legible
   */
  std::string human_readable_size(const char *size_blocks);

  /* Obtiene el punto de montaje del dispositivo
   * @param devnode: ruta del dispositivo
   * @return: std::string: punto de montaje del dispositivo
   */
  std::string get_mount_point(const std::string &devnode);

public:
  UsbDetector() = default;
  // ~UsbDetector();

  // lista los dispositivos USB
  std::vector<Device> listDevices();
};