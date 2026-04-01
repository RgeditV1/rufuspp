#include "format.hpp"
#include "../execute.hpp"
#include "../usb_detector.hpp"
#include <fstream>
#include <sstream>

void Format::applyFormat(const MakeType &type, const PartitionTable &partition,
                         UsbDetector::Device &device) {
  std::cout << "Starting robust format for " << device.path << std::endl;

  // 1. Desmontar todo lo relacionado con este disco
  unmountAllPartitions(device.path);

  // 2. Limpiar firmas antiguas para que parted no se bloquee
  execute("sudo wipefs -a " + device.path);

  // 3. Aplicar tabla de particiones
  if (applyPartitionTable(partition, device)) {
    // 4. Crear la partición primaria
    if (createPartition(device.path)) {
      refreshPartitions(device.path);
      // Esperar a que el sistema procese los cambios
      usleep(500000);

      // 5. ¡IMPORTANTE! El sistema suele automontar la partición recién creada.
      // Desmontamos de nuevo antes de mkfs.
      std::string partPath = getPartitionPath(device.path);
      unmountAllPartitions(device.path);

      // 6. Formatear la partición
      applyFileSystem(type, partPath);
    }
  }
}

bool Format::unmountAllPartitions(const std::string &diskPath) {
  std::ifstream file("/proc/self/mounts");
  std::string line;
  bool success = true;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string dev, mountpoint;
    iss >> dev >> mountpoint;

    // Si el dispositivo montado empieza con la ruta de nuestro disco (ej. /dev/sdb1 empieza con /dev/sdb)
    if (dev.find(diskPath) == 0) {
      if (!execute("sudo umount -l " + dev)) { // Usamos -l (lazy) para mayor efectividad
        success = false;
      } else {
        std::cout << "Unmounted: " << dev << std::endl;
      }
    }
  }
  return success;
}

std::string Format::getPartitionPath(const std::string &diskPath) {
  if (isdigit(diskPath.back())) {
    return diskPath + "p1";
  }
  return diskPath + "1";
}

bool Format::createPartition(const std::string &diskPath) {
  return execute("sudo parted -s " + diskPath + " mkpart primary 0% 100%");
}

bool FNtfs::applyFileSystem(const MakeType &f_type, const std::string &path) {
  if (f_type == MakeType::NTFS) {
    if (!execute("sudo mkfs.ntfs -f " + path)) {
      return false;
    }
    std::cout << "NTFS formatted success: " << path << std::endl;
  }
  return true;
}

bool FFat32::applyFileSystem(const MakeType &f_type, const std::string &path) {
  if (f_type == MakeType::FAT32) {
    if (!execute("sudo mkfs.vfat -F 32 " + path)) {
      return false;
    }
    std::cout << "FAT32 formatted success: " << path << std::endl;
  }
  return true;
}