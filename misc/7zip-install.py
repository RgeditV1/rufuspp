#!/usr/bin/env python3
import os
import subprocess
import sys
import shutil

def instalar_7zip():
    distro = ""
    distro_like = ""
    if os.path.exists("/etc/os-release"):
        with open("/etc/os-release") as f:
            for line in f:
                if line.startswith("ID="):
                    distro = line.strip().split("=")[1].strip('"')
                elif line.startswith("ID_LIKE="):
                    distro_like = line.strip().split("=")[1].strip('"')

    def run(cmd):
        subprocess.run(cmd, check=True)

    def is_root():
        return os.geteuid() == 0

    if not is_root():
        # This script expects sudo for package installation
        if shutil.which("sudo") is None:
            print("Se requiere sudo o ejecutar como root.")
            sys.exit(1)

    is_arch = distro in ("arch", "archlinux") or "arch" in distro_like
    is_debian = distro in ("debian", "ubuntu") or "debian" in distro_like or "ubuntu" in distro_like
    is_fedora = distro == "fedora" or "fedora" in distro_like or "rhel" in distro_like

    def has_7z_library():
        return (
            os.path.exists("/usr/lib/7zip/7z.so") or
            os.path.exists("/usr/lib/p7zip/7z.so") or
            os.path.exists("/usr/lib64/7zip/7z.so") or
            os.path.exists("/usr/lib64/p7zip/7z.so") or
            os.path.exists("/usr/lib64/7z.so")
        )

    if is_arch:
        print("Detectado Arch Linux...")
        try:
            run(["sudo", "pacman", "-Sy", "--noconfirm", "7zip"])
        except subprocess.CalledProcessError:
            run(["sudo", "pacman", "-Sy", "--noconfirm", "p7zip"])
        if not has_7z_library():
            run(["sudo", "pacman", "-Sy", "--noconfirm", "p7zip"])
    elif is_debian:
        print("Detectado Debian/Ubuntu...")
        run(["sudo", "apt", "update"])
        try:
            run(["sudo", "apt", "install", "-y", "7zip"])
        except subprocess.CalledProcessError:
            run(["sudo", "apt", "install", "-y", "p7zip-full"])
        if not has_7z_library():
            run(["sudo", "apt", "install", "-y", "p7zip-full"])
    elif is_fedora:
        print("Detectado Fedora...")
        try:
            run(["sudo", "dnf", "install", "-y", "7zip"])
        except subprocess.CalledProcessError:
            run(["sudo", "dnf", "install", "-y", "p7zip", "p7zip-plugins"])
        if not has_7z_library():
            run(["sudo", "dnf", "install", "-y", "p7zip", "p7zip-plugins"])
    else:
        print(f"Distribución no soportada: {distro}")
        return

    print("Instalación de 7zip completada.")

if __name__ == "__main__":
    instalar_7zip()
