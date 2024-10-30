add_library(mylib-lcd
  lcd.c
)

target_include_directories(mylib-lcd
  PUBLIC include
  PRIVATE .
)

# Incluye el directorio "include" al compilar con el flag -I[...]/lcd/include/ porque es PUBLICO
# La carpeta privada (que es la carpeta donde esta internal.h) NO se agrega al comando de compilacion.

# Entonces desde otro ejecutable o libreria que use target_link_libraries a esta libreria (LCD), al 
# compilar va a agregar esta carpeta ./include a los path de busqueda (-I).

# Por ejemplo esta libreria se usa desde el ejecutable principal (main), entonces desde main.c puedo
# incluir directamente las cabeceras publicas (que se encuentran en ./include/):
# #include "lcd.h"
# Pero NO puedo incluir las que esta como privadas (en la carpeta actual):
# # include "internal.h" // no funciona porque no la encuentra en los path que de busqueda que se pasan al compilar!

# Tener en cuenta que al estar dentro de la jerarquia de carpetas del ejecutable principal (main), desde main aún se 
# puede incluir ambas cabeceras utilizando el path relativo:
# #include "lib/lcd/internal.h"
# #include "lib/lcd/lcd.h"
# Ambas funcionan porque en este caso usa el camino relativo desde el lugar donde se compila:
# / (donde se compila)
#   src/
#   lib/

# (Probablemente tambien se pueda usar relativo desde main)
