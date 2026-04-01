#include "iso.hpp"
#define TEST
#ifndef TEST
#include "storage/format.hpp"
#include "usb_detector.hpp"
#endif // TEST
#include <iostream>
#include <memory>

int main() {
#if 0 // testing
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
#endif

  std::unique_ptr<Iso> iso = std::make_unique<Iso>();
  std::cout << "--------------------------------------------------"
            << std::endl;
  std::cout << "Testing Windows ISO: "
            << iso->addIsoInfo("/home/rgedit/Descargas/win.iso",
                               Iso::IsoType::WINDOWS)
            << std::endl; // TODO: Agregar la ruta de la iso como parametro

  for (const auto &iso : iso->getIsoInfo()) {
    std::cout << "--------------------------------------------------"
              << std::endl;
    std::cout << "Volume id: " << iso.volume_id << std::endl;
    std::cout << "Publisher: " << iso.publisher << std::endl;
    std::cout << "Iso path: " << iso.isoPath << std::endl;
    std::cout << "Architecture: " << iso.architecture << std::endl;
    std::cout << "Type: " << iso.type << std::endl;
  }
  return 0;
}