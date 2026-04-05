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
    std::string bootType;     /**< Tipo de arranque (UEFI, BIOS) */
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

  /**
   * @brief Verifica si la imagen es arrancable via UEFI.
   * @param isoPath Ruta del archivo ISO.
   * @param uefi_files Vector de archivos UEFI a buscar.
   * @return true si se detectan todos los archivos UEFI necesarios.
   */
  bool isUefiBootable(const std::string &isoPath, const std::vector<std::string> &uefi_files);

  /**
   * @brief Verifica si la imagen es arrancable via BIOS (Legacy).
   * @param isoPath Ruta del archivo ISO.
   * @param bios_files Vector de archivos BIOS a buscar.
   * @return true si se detectan todos los archivos BIOS necesarios.
   */
  bool isBiosBootable(const std::string &isoPath, const std::vector<std::string> &bios_files);
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
 * @brief Estructura que define las firmas de archivos críticas para imágenes de Windows.
 */
struct WindowsSignatures {
    // Archivos obligatorios para arranque UEFI
    static inline const std::vector<std::string> uefi_files = {
        "efi/boot/bootx64.efi", 
        "efi/boot/bootia32.efi", // Soporte para tablets/laptops de 32 bits
        "bootmgfw.efi"
    };

    // Archivos obligatorios para arranque BIOS (Legacy)
    static inline const std::vector<std::string> bios_files = {
        "bootmgr",
        "boot/bcd"
    };

    // Archivos de datos (independientes del arranque)
    static inline const std::vector<std::string> data_files = {
        "sources/install.wim", 
        "sources/install.esd"
    };
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