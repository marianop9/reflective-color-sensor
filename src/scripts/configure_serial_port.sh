#!/bin/sh

# para usar con WSL (Debian)
# al conectar el puerto serie con usbipd, el puerto serie pertenece al grupo root
# paso el puerto al grupo dialout (el usuario debe pertenecer a ese grupo)
sudo chgrp dialout /dev/ttyACM0
# da acceso R/W del puerto a los usuarios del grupo (dialout)
sudo chmod 660 /dev/ttyACM0