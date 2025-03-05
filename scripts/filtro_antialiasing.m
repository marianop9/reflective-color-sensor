# Filtro antialising activo - PB Butterworth
fmuestreo = 500; # Hz

# fp < fmuestreo/2
fp = 200;

# se fija el orden en 2 ya que se dispone de un solo op-amp
n = 2;

# filtro pasa-bajos
# metodo de Savant - fijo R1 = R2 = R
R = 100e3; # ajustar para obtener valores de C convenientes

# valores normalizados de capacitancia
Cn = [1.414 .7071];

# desnormalizo
C = Cn / (2*pi*fp * R)


