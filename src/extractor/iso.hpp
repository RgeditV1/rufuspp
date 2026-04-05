/**
 * @file iso.hpp
 * @brief Clase encargada de manejar las validaciones y metadatos de archivos
 * ISO.
 */

#pragma once
#include <bit7z/bit7z.hpp>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @class Iso
 * @brief Proporciona funcionalidades para identificar el tipo de ISO y extraer
 * su información.
 */
class Iso {
public:
  /**
   * @enum IsoType
   * @brief Enumeración de los tipos de sistemas operativos soportados para
   * validación.
   */
  enum class IsoType { AUTO, WINDOWS, ARCH, DEBIAN_UBUNTU };

private:
  /**
   * @struct IsoInfo
   * @brief Estructura que almacena los metadatos extraídos de una imagen ISO.
   */
  struct IsoInfo {
    std::string volume_id;    /**< Identificador de volumen (Label) */
    std::string publisher;    /**< Publicador de la imagen */
    std::string isoPath;      /**< Ruta local del archivo ISO */
    std::string architecture; /**< Arquitectura detectada */
    std::string type;         /**< Tipo de sistema operativo (string) */
  };

  std::vector<IsoInfo> myIso; /**< Lista de ISOs procesadas */

  /**
   * @brief Verifica la existencia de archivos específicos dentro de la ISO
   * usando bit7z.
   * @param isoPath Ruta del archivo ISO.
   * @param signatures Vector de rutas de archivos a buscar como firmas.
   * @param format Formato de archivo (ISO, UDF, etc).
   * @return true si se encuentra al menos una de las firmas.
   */
  bool checkSignature(const std::string &isoPath,
                      const std::vector<std::string> &signatures,
                      const bit7z::BitInFormat &format);

  /**
   * @brief Lógica interna para decidir qué validador ejecutar según el tipo
   * solicitado.
   * @param isoPath Ruta del archivo ISO.
   * @param type Tipo de ISO contra el que se desea validar.
   * @return std::string Nombre del tipo detectado o "ERROR".
   */
  std::string runValidation(const std::string &isoPath,
                            IsoType type = IsoType::AUTO);

protected:
  /**
   * @brief Valida si la imagen corresponde a Windows (UDF/ISO).
   * @param isoPath Ruta del archivo ISO.
   * @return true si se detectan firmas de Windows.
   */
  bool isWindows(const std::string &isoPath);

  /**
   * @brief Valida si la imagen corresponde a una derivada de Debian o Ubuntu.
   * @param isoPath Ruta del archivo ISO.
   * @return true si se detectan firmas de Debian/Ubuntu.
   */
  bool isDebianUbuntu(const std::string &isoPath);

  /**
   * @brief Valida si la imagen corresponde a Arch Linux.
   * @param isoPath Ruta del archivo ISO.
   * @return true si se detectan firmas de Arch Linux.
   */
  bool isArch(const std::string &isoPath);

  /**
   * @struct WindowsSignatures
   * @brief Firmas de archivos críticas para imágenes de Windows.
   */
  struct WindowsSignatures {
    static inline const std::vector<std::string> files = {
        "sources/install.wim", "sources/install.esd", "sources/boot.wim",
        "boot/bcd"};
  };

  /**
   * @struct DebianUbuntuSignatures
   * @brief Firmas de archivos críticas para imágenes de Debian/Ubuntu.
   */
  struct DebianUbuntuSignatures {
    static inline const std::vector<std::string> files = {
        ".disk/info", "casper/", "debian", ".disk/base_installable"};
  };

  /**
   * @struct ArchSignatures
   * @brief Firmas de archivos críticas para imágenes de Arch Linux.
   */
  struct ArchSignatures {
    static inline const std::vector<std::string> files = {
        "arch/airootfs.sfs", "arch/x86_64/airootfs.sfs", "arch/version"};
  };

public:
  /**
   * @brief Constructor por defecto.
   */
  Iso() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~Iso() = default;

  /**
   * @brief Método principal para procesar una ISO. Valida el tipo y extrae
   * metadatos.
   * @param isoPath Ruta del archivo ISO.
   * @param type Tipo de sistema operativo que se espera validar.
   * @return std::string "OK" si la validación es exitosa, "ERROR" en caso
   * contrario.
   */
  std::string addIsoInfo(const std::string &isoPath, IsoType type);

  /**
   * @brief Obtiene la lista de metadatos de las ISOs validadas correctamente.
   * @return std::vector<IsoInfo> Vector con la información de las ISOs.
   */
  inline std::vector<IsoInfo> getIsoInfo() const { return myIso; }
};