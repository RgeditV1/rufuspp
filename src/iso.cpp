#include "iso.hpp"
#include <iostream>
#include <memory>

void Iso::addIsoInfo(const std::string &isoPath) {
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

  myIso.push_back(info);
}

int main() {
  std::unique_ptr<Iso> iso = std::make_unique<Iso>();
  iso->addIsoInfo("/home/rgedit/Descargas/Win10_22H2_Spanish_x64v1.iso");
  for (auto &info : iso->getIsoInfo()) {
    std::cout << info.volume_id << "\n";
    std::cout << info.publisher << "\n";
    std::cout << info.isoPath << "\n";
    std::cout << info.getType() << "\n";
  }
  return 0;
}
