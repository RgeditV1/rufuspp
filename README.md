# rufuspp
A rufus port for linux system written in c++

## Dependencias

- Esta Repo utiliza el wrapper `Bit7z` para el manejo de archivos iso, el cual requiere la libreria `7z` instalada en el sistema
- Para el manejo de archivos iso se utiliza el comando `isoinfo` que viene de `cdrtools`, el cual se puede instalar con el comando `sudo pacman -S cdrtools` o `sudo apt install cdrtools` o `sudo dnf install cdrtools`

## Como compilar

- `mkdir build`
- `cd build`
- `cmake ..`
- `make`