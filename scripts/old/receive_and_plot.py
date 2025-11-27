"""
Recibe datos de medicion de cada color por puerto serie y los grafica.

Primero, recibe las mediciones de cada canal (rojo, verde y azul) sin filtrar, 
y luego las mismas mediciones luego de aplicar un filtro digital pasa-bajos.

El script se configuró para ser ejecutado desde WSL, habiendo configurado previamente 
el acceso y habilitado el acceso desde WSL mediante usbipd-win 
(https://github.com/dorssel/usbipd-win)
"""

import serial
from matplotlib.axes import Axes
import matplotlib.pyplot as plt
import numpy as np

# Initialize serial with no timeout to wait indefinitely for data
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=None)

fs = 500
Ts = 1/fs

def read_sensor_data():
    rgb = []
    ax1: Axes; ax2: Axes
    fig, (ax1, ax2) = plt.subplots(1, 2)
    i = 0
    while i < 6:
        try:
            # Wait indefinitely for a line of data
            line = ser.readline().decode('utf-8').strip()
            if line:
                # Convert to a list of integers if valid
                if line.startswith('['):
                    raw_data = line.strip('[]').split(',')
                    
                    print(f'\tReceived data ({len(raw_data)}): ', raw_data)

                    data = [int(v, 16) for v in raw_data]
                    rgb.append(data)
                    
                    i += 1
                else:
                    print('Received: ', line)

            if i == 6:
                print('Received: ', ser.readline().decode('utf-8').strip())

        except ValueError:
            print("Received malformed data")
    
    print(f'got {len(rgb)} arrays of size ', [len(x) for x in rgb])
    rgb_colors = ['r', 'g', 'b']

    n = range(len(rgb[0]))
    for j in range(0, 6, 2):
        ax1.plot(n, rgb[j], rgb_colors[int(j/2)])
    
    for j in range(1, 6, 2):
        ax2.plot(n, rgb[j], rgb_colors[int(j/2)])

    ax1.set_title('Previo al filtrado')
    ax2.set_title('Post filtrado')
    
    ax1.set_ylabel('Valor (8 bits)')
    ax1.set_xlabel('n')
    ax2.set_xlabel('n')

    ax1.set_ylim([0, 260])
    ax2.set_ylim([0, 260])

    ax1.set_yticks(list(range(0, 280, 20)))
    ax2.set_yticks(list(range(0, 280, 20)))
    ax1.grid()
    ax2.grid()
    plt.show(block=True)

try:
    print("Waiting for data...")
    read_sensor_data()
        
except KeyboardInterrupt:
    print("Program terminated.")
finally:
    ser.close()
