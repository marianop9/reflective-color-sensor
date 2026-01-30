# Blackmagic + STLinkv2 for RP2040

- https://black-magic.org/index.html
- https://github.com/blackmagic-debug/blackmagic
- https://github.com/blackmagic-debug/blackmagic/tree/main/src/platforms
- https://github.com/blackmagic-debug/stlink-tool

Se necesita:
- Blackmagic firmware p/ STLink
- BMDA (la app que se comunica con el STLink)

En teoría no se necesita BMDA; una vez que se carga el firmware en el STLink ya se muestran los dos puertos serie, pero en Windows no se puede conectar directamente con GDB. (e.g., `tar ext \\.\COM11` no functiona). 

Al ejecutar BMDA con el STLink conectado, se forwardea el puerto serie a localhost, y este si se puede acceder desde GDB.
## 1. build Blackmagic stuff

```sh
git clone https://github.com/blackmagic-debug/blackmagic.git
```

Compilar con `meson`. Se compila el firmware para el STLink y la app de Blackmagic (BMDA).

Normalmente se especificaría un "cross-file", que es uno de varios archivos de configuración disponibles en la carpeta `cross-file` del repo. Este especifica detalles como la toolchain, CPU, endianness, y otras cositas. Las mas importantes son `probe` y `targets`:
- `probe` define el debugger que estamos usando 
- `targets` define los distintos MCU que se quieren debuggear

> Todas las opciones de config. se pueden ver en el archivo `meson-options.txt` del repo de Blackmagic.

Como el cross-file predefinido para STLink (`cross-file/stlink.ini`) no incluye el target para la RP2040 (`rp`), se podría editar el archivo y agregarlo manualmente. Como en este caso mi `probe`y `target` son fijos, los especifico manualmente, y uso la cross-file `cross-file/arm-none-eabi.ini`, que solo define las opciones básicas (toolchain, CPU, endianness).

```sh
meson setup build --cross-file cross-file/arm-none-eabi.ini -Dbmd_bootloader=false -Dprobe=stlink -Dtargets=rp
```

Otro detalle importante es que NO se incluye el bootloader de Blackmagic. Ellos ofrecen su propio debugger y su código parece incluir el bootloader especifico para su dispositivo por defecto.

Las instrucciones en https://github.com/blackmagic-debug/blackmagic/tree/main/src/platforms/stlink especifican compilar con `-Dbmd_bootloader=false` para no sobrescribir el bootloader propio del STLink.

Luego se compila:

```sh
meson compile -C build
```

Se deberian obtener dos binarios:
- `blackmagic_stlink_firmware.bin` es el firmware para el STLink
- `blackmagic.exe` es la app de escritorio que expone el puerto localhost al que se accede desde GDB
## 2. load STLink firmware

El STLink chinardo necesita una herramienta adicional para flashear el firmware de Blackmagic.

### 2a. build STLink flashing tool

Compilar la herramienta del siguiente repo: https://github.com/blackmagic-debug/stlink-tool

```sh
git clone https://github.com/jeanthom/stlink-tool
cd stlink-tool
git submodule update --init --depth 1
make
```

En caso de compilar desde Windows, se usa el fork de [sakana280](https://github.com/sakana280/stlink-tool)

```sh
git clone https://github.com/sakana280/stlink-tool
cd stlink-tool
git submodule update --init --depth 1
mingw32-make.exe
```

### 2b. load firmware w/ tool

```
stlink-tool /path/to/blackmagic_stlink_firmware.bin
```

Salida:
```
Firmware version : V2J46S7
Loader version : 14152
ST-Link ID : 4100070002000059534B524E
Firmware encryption key : 2BD5C9391507CCB9D0A10CECC8041617
Current mode : 1
Loaded firmware : .\blackmagic_stlink_firmware.bin, size : 63608 bytes
...............................................................
```

> Aparentemente hay que volver a correr el comando cada vez que se desconecta el STLink (es porque se conservo el bootloader original???).

Una vez que se carga el firmware, se deberían detectar dos puertos serie, tal como se explica en https://black-magic.org/getting-started.html.
## 3. run BMDA

Con el STLink conectado (con el firmware de Blackmagic), se ejecuta la BDMA (e.g., `blackmagic.exe`).

Salida:
```
Found AUX Serial at COM11
Using BMP at COM13
Listening on TCP port: 2000
```

## 4. debug

Se ejecuta GDB, se conecta al remoto, se hace un scan para detectar el micro, se conecta al micro, y se carga el programa y/o se debuggea:

```sh
arm-none-eabi-gdb firmware.elf
```

```
tar ext :2000
```

```
monitor auto_scan
...
Available Targets:
No. Att Driver
 1      RP2040 M0+
 2      RP2040 M0+
 3      RP2040 Rescue (Attach to reset)
...
```

```
attach 1
```

```
load
```

El primer `load` siempre me da un error como:

```
Loading section .boot2, size 0x100 lma 0x10000000
Loading section .text, size 0x915c lma 0x10000100
Error writing data to flash
```

Pero al reintentar el comando funciona.

