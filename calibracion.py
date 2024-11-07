import numpy as np


# resistencias
Rw = np.array([20.15e3, 38.5e3, 25.5e3])
Rk = np.array([180e3, 450e3, 243e3])

I = 3e-6

# convierto a tensiones conociendo la corriente
Vk = I * Rk
Vw = I * Rw

# valores de ADC
adc_k = Vk * (1 << 12) / 3.33
adc_w = Vw * (1 << 12) / 3.33

print(f'valores adc sup NEGRA: {adc_k}')
print(f'valores adc sup BLANCA: {adc_w}')