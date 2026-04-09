# rufuspp
A rufus port for linux system written in c++

## Dependencias

- Esta Repo utiliza el wrapper `Bit7z` para el manejo de archivos iso. La libreria `7z.so` se incluye en `thirdparty/7zip/7z.so` y es copiada automaticamente junto al ejecutable al compilar (no se requiere instalacion del sistema). Como alternativa de fallback, tambien se busca en las rutas estandar del sistema (`/usr/lib/7zip/7z.so`, `/usr/lib/p7zip/7z.so`, etc.)
- Para el manejo de archivos iso se utiliza el comando `isoinfo` que viene de `cdrtools`, el cual se puede instalar con el comando `sudo pacman -S cdrtools` o `sudo apt install cdrtools` o `sudo dnf install cdrtools`

## Como compilar

- `mkdir build`
- `cd build`
- `cmake ..`
- `make`