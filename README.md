# LDRSensorColor

## Detección Experimental de Colores Utilizando Sensores LDR
Este proyecto fue desarrollado para el Congreso RPIC Estudiantil en la Universidad Tecnológica Nacional (UTN). Explora el uso de un sensor LDR (Resistor Dependiente de la Luz) en combinación con fuentes de luz controladas para detectar e interpretar colores de manera experimental. Aunque los LDR no son sensores de color por sí mismos, este proyecto demuestra cómo se pueden utilizar técnicas creativas y económicas para aproximarse a esta funcionalidad.

## Descripción
El sistema utiliza un sensor LDR para medir la intensidad de la luz reflejada por un objeto bajo la iluminación de LEDs de colores (rojo, verde y azul). Al analizar las variaciones en las lecturas del LDR según el color de la luz emitida, el proyecto estima propiedades básicas del color del objeto. Este enfoque es ideal para aplicaciones educativas y de bajo costo.

## Características
Uso de un sensor LDR como alternativa económica a sensores de color dedicados.
Implementación en Arduino para un procesamiento sencillo y accesible.
Diseño modular que permite experimentar con diferentes configuraciones de iluminación.
Código abierto y documentado para fines educativos.

## Requisitos
### Hardware
- Microcontrolador: Arduino Uno (o compatible).
- Sensor: 1x LDR (Light Dependent Resistor).
- LEDs: 1x LED rojo, 1x LED verde, 1x LED azul.
- Resistencias:
    1x 10kΩ (para el LDR).
    3x 220Ω (una por cada LED).
- Protoboard y cables de conexión.

### Software
- PICO C SDK
- FreeRTOS