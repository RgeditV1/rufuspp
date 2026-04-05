#!/usr/bin/env python3
import subprocess
import sys
import os
import shutil
"""
Script para instalar cdrtools en diferentes distribuciones de Linux
Soporta: Arch Linux, Fedora, Debian y Ubuntu
"""



def detect_distro():
    """Detecta la distribución de Linux"""
    try:
        distro = ""
        distro_like = ""
        with open('/etc/os-release', 'r') as f:
            for line in f:
                if line.startswith("ID="):
                    distro = line.strip().split("=")[1].strip('"').lower()
                elif line.startswith("ID_LIKE="):
                    distro_like = line.strip().split("=")[1].strip('"').lower()
        return distro, distro_like
    except FileNotFoundError:
        pass
    return "", ""


def install_cdrtools(distro, distro_like):
    """Instala cdrtools según la distribución"""
    def run(cmd):
        subprocess.run(cmd, check=True)

    def ensure_sudo():
        if os.geteuid() == 0:
            return
        if shutil.which("sudo") is None:
            print("✗ Se requieren permisos de administrador (sudo) o ejecutar como root")
            sys.exit(1)

    def has_cmd(cmd):
        return shutil.which(cmd) is not None

    ensure_sudo()

    is_arch = distro in ("arch", "archlinux") or "arch" in distro_like
    is_debian = distro in ("debian", "ubuntu") or "debian" in distro_like or "ubuntu" in distro_like
    is_fedora = distro == "fedora" or "fedora" in distro_like or "rhel" in distro_like

    try:
        if is_arch:
            print("Instalando cdrtools en Arch Linux...")
            try:
                run(['sudo', 'pacman', '-S', '--noconfirm', 'cdrtools'])
            except subprocess.CalledProcessError:
                print("cdrtools no disponible, instalando genisoimage/xorriso...")
                run(['sudo', 'pacman', '-S', '--noconfirm', 'cdrkit', 'xorriso'])

        elif is_fedora:
            print("Instalando cdrtools en Fedora...")
            # Fedora no provee cdrtools; usar genisoimage/xorriso
            run(['sudo', 'dnf', 'install', '-y', 'genisoimage', 'xorriso'])
            if not has_cmd("isoinfo"):
                # Intentar cdrkit si isoinfo no está disponible
                run(['sudo', 'dnf', 'install', '-y', 'cdrkit'])

        elif is_debian:
            print("Instalando cdrtools en Debian/Ubuntu...")
            run(['sudo', 'apt', 'update'])
            try:
                run(['sudo', 'apt', 'install', '-y', 'cdrtools'])
            except subprocess.CalledProcessError:
                print("cdrtools no disponible, instalando genisoimage/xorriso...")
                run(['sudo', 'apt', 'install', '-y', 'genisoimage', 'xorriso'])
                if not has_cmd("isoinfo"):
                    run(['sudo', 'apt', 'install', '-y', 'cdrkit'])

        else:
            print("Distribución no soportada")
            return False

        # Verificación mínima
        if not has_cmd("mkisofs") and not has_cmd("genisoimage") and not has_cmd("xorriso"):
            print("✗ No se encontró herramienta ISO (mkisofs/genisoimage/xorriso)")
            return False
        if not has_cmd("isoinfo"):
            print("✗ No se encontró isoinfo (cdrtools/cdrkit)")
            return False

        print("✓ cdrtools instalado exitosamente")
        return True
    
    except subprocess.CalledProcessError as e:
        print(f"✗ Error durante la instalación: {e}")
        return False
    except PermissionError:
        print("✗ Se requieren permisos de administrador (sudo)")
        return False


def main():
    distro, distro_like = detect_distro()
    
    if not distro:
        print("No se pudo detectar la distribución de Linux")
        sys.exit(1)
    
    print(f"Distribución detectada: {distro}")
    
    if install_cdrtools(distro, distro_like):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
