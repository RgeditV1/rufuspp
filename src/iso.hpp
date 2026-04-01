/*
 * Este archivo se encarga de manejar las validaciones de archios iso
 * De esta manera detectara que tipo de imagen estamos usando
 */

#pragma once
#include <bit7z/bit7z.hpp>
#include <stdexcept>
#include <string>
#include <vector>

class Iso {
private:
#ifdef ISO_TYPE
  // Enum para identificar el tipo de imagen
  // TODO: Agregar mas tipos de imagenes en el futuro
  enum class IsoType { WINDOWS, UBUNTU, ARCH, DEBIAN, FEDORA, MAC, UNKNOWN };
#endif

  struct IsoInfo {
    std::string volume_id; // label
    std::string publisher;
    std::string isoPath;
    std::string architecture;
    std::string type;
  };

  std::vector<IsoInfo> myIso;

  inline std::string get_execute(const std::string &cmd) {
    std::string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
      throw std::runtime_error("Error: No se pudo ejecutar el comando");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);
    return result;
  }

  std::string getType(const std::string &publisher,
                      const std::string &volume_id);

public:
  Iso() = default;
  ~Iso() = default;

  /**
   * @brief Agrega la informacion de la iso
   * @param isoPath Ruta del archivo iso
   * @return string con la informacion de la iso
   */
  std::string addIsoInfo(const std::string &isoPath);

  /**
   * @brief Obtiene la informacion de la iso
   * @return vector de IsoInfo
   */
  inline std::vector<IsoInfo> getIsoInfo() const { return myIso; }

  /**
   * @brief Verifica si la iso contiene el archivo especificado
   * @param isoPath Ruta del archivo iso
   * @return true si contiene el archivo, false en caso contrario
   */
  bool isWindows(const std::string &isoPath);
};