#include "usb_detector.hpp"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <libudev.h>
#include <sstream>
#include <vector>

// UsbDetector::~UsbDetector() {}

std::vector<UsbDetector::Device> UsbDetector::listDevices() {
  std::vector<Device> device;
  /*
   * Con esto lo que quiero hacer es listar los dispositivos USB
   * y obtener la información de cada uno.
   */

  struct udev *udev = udev_new();
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);
  // solo dispositivos de bloque e.g(/dev/sda, /dev/sdb) disco y particiones
  udev_enumerate_add_match_subsystem(enumerate, "block");
  udev_enumerate_scan_devices(enumerate);
  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

  struct udev_list_entry *dev_list_entry;
  udev_list_entry_foreach(dev_list_entry, devices) {
    Device dev_data;
    const char *syspath = udev_list_entry_get_name(dev_list_entry);

    struct udev_device *dev = udev_device_new_from_syspath(udev, syspath);
    if (!dev)
      continue;

    // solo dispositivos de tipo partition o disk
    const char *devtype = udev_device_get_devtype(dev);
    std::string type_str = devtype ? devtype : "";
    if (type_str != "partition" && type_str != "disk") {
      udev_device_unref(dev);
      continue;
    }
    dev_data.deviceType = type_str;

    // == DATOS ==
    // nos da la ruta e.g(/dev/sda, sdb1 etc..)
    const char *devnode = udev_device_get_devnode(dev);
    if (devnode)
      dev_data.path = devnode;

    // nos da el nombre del dispositivo e.g(sda, sdb1 etc..)
    const char *sysname = udev_device_get_sysname(dev);
    if (sysname)
      dev_data.name = sysname;

    // nos da el tamaño del dispositivo en bloques de 512 bytes
    const char *size_blocks = udev_device_get_sysattr_value(dev, "size");
    if (size_blocks) {
      dev_data.size = human_readable_size(size_blocks);
    }

    // nos da el fabricante
    struct udev_device *usb_dev =
        udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
    // si no es un dispositivo usb, lo saltamos
    if (!usb_dev) {
      udev_device_unref(dev);
      continue;
    }

    const char *vendor =
        udev_device_get_property_value(usb_dev, "ID_VENDOR_FROM_DATABASE");
    if (!vendor)
      vendor = udev_device_get_property_value(usb_dev, "ID_VENDOR");
    if (vendor)
      dev_data.manufacturer = vendor;

    if (devnode)
      dev_data.mountPoint = get_mount_point(devnode);

    const char *fs = udev_device_get_property_value(dev, "ID_FS_TYPE");
    if (fs)
      dev_data.fileSystem = fs;

    const char *label = udev_device_get_property_value(dev, "ID_FS_LABEL");
    if (label)
      dev_data.label = label;

    // guardar en vector
    device.push_back(dev_data);

    udev_device_unref(dev);
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
  return device;
}

std::string UsbDetector::human_readable_size(const char *size_blocks) {
  if (!size_blocks)
    return "0 B";

  uint64_t blocks = std::strtoull(size_blocks, nullptr, 10);
  uint64_t size = blocks * 512ULL;

  if (size < Size::KB) {
    return std::to_string(size) + " B";
  } else if (size < Size::MB) {
    return std::to_string(size / Size::KB) + " KB";
  } else if (size < Size::GB) {
    return std::to_string(size / Size::MB) + " MB";
  } else if (size < Size::TB) {
    return std::to_string(size / Size::GB) + " GB";
  } else if (size < Size::PB) {
    return std::to_string(size / Size::TB) + " TB";
  }

  return std::to_string(size / Size::PB) + " PB";
}

std::string UsbDetector::get_mount_point(const std::string &devnode) {
  std::ifstream file("/proc/self/mounts");
  std::string line;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string device, mountpoint;

    iss >> device >> mountpoint;

    if (device == devnode)
      return mountpoint;
  }

  return "";
}