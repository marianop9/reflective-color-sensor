import serial
import time

PORT = '/dev/tty.usbmodem101'# Cambiar según el puerto donde esté conectada la Pico
BAUD = 115200
ser = serial.Serial(PORT, BAUD, timeout=1)
print(f"Conectado a {PORT} a {BAUD} baudios.")
ser.reset_input_buffer()
time.sleep(1)

def cmd_to_pico(cmd):
    ser.reset_input_buffer()
    command = "&" + cmd + "*"
    print(f"Enviando comando: {command}")
    ser.write(command.encode('ascii'))

def listen_pico():
    cmd_to_pico("hola,") #    "PING", "MEM", "TOGGLE_LED", "ADC", "SET_LED"

    if not ser.read_until(b'&'):
        return None
    
    header = ser.read_until(b',').strip(b',')
    header_t = header.decode('ascii').strip(',')
    length = ser.read_until(b',').decode('ascii').strip(',')
    long = int(length)

    if header_t == "DATA":
        payload = ser.read(long)
        ser.read(1) # Leer el carácter de finalización '*'
        print(f"{header.decode('ascii').strip(',')} |bytes: {long} | Datos: {list(payload)}") # &<tipo>,<payload_bytes>,<payload>*
    else:
        ser.read(long + 1) # Leer el payload y el carácter de finalización '*'
        print(f"Respuesta no esperada: {header.decode('ascii').strip(',')}")
    
    
if __name__ == "__main__":
    listen_pico()