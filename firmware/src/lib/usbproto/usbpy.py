import serial
import time

PORT = '/dev/tty.usbmodem101'
BAUD = 115200

def listen_pico():
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Conectado a {PORT} a {BAUD} baudios.")
    ser.reset_input_buffer()
    time.sleep(1)

    print("Enviando comando: &DATA*")
    ser.write(b'&DATA*')
    
    if ser.read_until(b'&'):
        
        header = ser.read_until(b',').strip(b',')
        len = ser.read_until(b',')
        
        long = int(len.decode('ascii', errors='ignore').strip(','))
        payload = ser.read(long)
        ser.read(1)
        
        print(f"[{header.decode('ascii').strip(',')}] | Datos: {list(payload)}")
        
        
if __name__ == "__main__":
    listen_pico()