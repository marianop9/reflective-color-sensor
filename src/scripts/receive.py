import serial
import matplotlib.pyplot as plt
import numpy as np

# Initialize serial with no timeout to wait indefinitely for data
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=None)

def read_sensor_data():
    i = 0
    rgb = []
    n = np.arange(50)
    fig, ax = plt.subplots()
    while i < 3:
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

            if i == 3:
                print('Received: ', ser.readline().decode('utf-8').strip())

        except ValueError:
            print("Received malformed data")
    
    print(f'got {len(rgb)} arrays of size ', [len(x) for x in rgb])
    rgb_colors = ['r', 'g', 'b']
    for j in range(3):
        ax.plot(n, [x+j for x in rgb[j]], rgb_colors[j])
    
    plt.show(block=True)

try:
    print("Waiting for data...")
    read_sensor_data()
        
except KeyboardInterrupt:
    print("Program terminated.")
finally:
    ser.close()
