#!/usr/bin/env python3
import os
import subprocess

def instalar_7zip():
    distro = ""
    if os.path.exists("/etc/os-release"):
        with open("/etc/os-release") as f:
            for line in f:
                if line.startswith("ID="):
                    distro = line.strip().split("=")[1].strip('"')
                    break

    if distro == "arch":
        print("Detectado Arch Linux...")
        subprocess.run(["sudo", "pacman", "-Sy", "--noconfirm", "p7zip"])
    elif distro in ("debian", "ubuntu"):
        print("Detectado Debian/Ubuntu...")
        subprocess.run(["sudo", "apt", "update"])
        subprocess.run(["sudo", "apt", "install", "-y", "p7zip-full"])
    elif distro == "fedora":
        print("Detectado Fedora...")
        subprocess.run(["sudo", "dnf", "install", "-y", "p7zip", "p7zip-plugins"])
    else:
        print(f"Distribución no soportada: {distro}")
        return

    print("Instalación de 7zip completada.")

if __name__ == "__main__":
    instalar_7zip()
