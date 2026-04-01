
#pragma once

#include "../execute.hpp"
#include "../usb_detector.hpp"
#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <linux/fs.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

class FNtfs;
class FFat32;

class Format {
public:
  enum class PartitionTable : uint8_t {
    GPT,
    MBR,
  };

  enum class MakeType : uint8_t {
    FAT32,
    NTFS,
  };

  Format() = default;
  virtual ~Format() = default;

  // Lógica común de formateo (DRY)
  void applyFormat(const MakeType &type, const PartitionTable &partition,
                   UsbDetector::Device &device);

  virtual bool applyFileSystem(const MakeType &f_type,
                               const std::string &path) = 0;

  // Desmontar el disco y todas sus particiones de forma recursiva
  bool unmountAllPartitions(const std::string &diskPath);

  // Determinar la ruta de la partición (ej. /dev/sdb1 o /dev/nvme0n1p1)
  std::string getPartitionPath(const std::string &diskPath);

  // Crear una partición primaria que ocupe todo el disco
  bool createPartition(const std::string &diskPath);

protected:
  bool is_done = false;

  inline void refreshPartitions(const std::string &devPath) {
    int fd = open(devPath.c_str(), O_RDONLY);
    if (fd >= 0) {
      ioctl(fd, BLKRRPART, NULL); // Re-read partition table
      close(fd);
    }
  }

  inline bool unmountDevice(UsbDetector::Device &device) {
    if (device.mountPoint.empty()) {
      std::cout << "Device " << device.path
                << " is not mounted, skipping umount." << std::endl;
      return true;
    }
    if (!execute("sudo umount " + device.path)) {
      return false;
    }
    std::cout << "Device " << device.path << " unmounted" << std::endl;
    return true;
  }

  inline bool applyPartitionTable(const PartitionTable &p_type,
                                  UsbDetector::Device &device) {
    if (p_type == PartitionTable::GPT) {
      if (!execute("sudo parted -s " + device.path + " mklabel gpt")) {
        return false;
      }
      std::cout << "GPT partition table applied to " << device.path
                << std::endl;
    } else if (p_type == PartitionTable::MBR) {
      if (!execute("sudo parted " + device.path + " mklabel msdos")) {
        return false;
      }
      std::cout << "MBR partition table applied to " << device.path
                << std::endl;
    }
    return true;
  }
};

class FFat32 : public Format {
public:
  FFat32() = default;

  bool applyFileSystem(const MakeType &f_type,
                       const std::string &path) override;
};

class FNtfs : public Format {
public:
  FNtfs() = default;

  bool applyFileSystem(const MakeType &f_type,
                       const std::string &path) override;
};