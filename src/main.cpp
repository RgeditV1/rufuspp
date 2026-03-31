#include "storage/format.hpp"
#include "usb_detector.hpp"
#include <iostream>
#include <memory>

int main() {
  auto usb_detector = std::make_unique<UsbDetector>();
  auto devices = usb_detector->listDevices();

  if (devices.empty()) {
    std::cout << "No USB devices found." << std::endl;
    return 0;
  }

  // Por seguridad, solo formateamos el primer dispositivo encontrado
  // En una versión real, esto vendría de la elección del usuario en la UI
  auto &target = devices[0];
  std::cout << "Target Device: " << target.path << " (" << target.name << ")"
            << std::endl;

  FFat32 format;
  format.applyFormat(Format::MakeType::FAT32, Format::PartitionTable::GPT,
                     target);
  return 0;
}