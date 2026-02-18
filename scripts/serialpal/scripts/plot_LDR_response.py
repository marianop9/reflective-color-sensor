import numpy as np
import matplotlib.pyplot as plt

""" Graficala respuesta
"""

Vref = 3.3
Vdd = 5
Rf = [1, 10, 100]
Rldr = np.logspace(-1, 3, 100)

fig, ax1 = plt.subplots()

ax1.set_xlabel(r"R $[k\Omega]$")
ax1.set_ylabel("Vo")
ax1.set_ylim((0, 4))

for rf in Rf:
    Vo = Vref + (Vref - Vdd) * rf / Rldr

    ax1.semilogx(Rldr, Vo, label=f"Rf={rf}")

# ax2 -> eje-y en unidades de ADC
ax2 = ax1.twinx()
ax2.set_ylim((0, 4095/3.3*4))
ax2.set_ylabel("ADC")
ax2.set_yticks(np.arange(0, 4500, 500))

# ax3 -> eje-x en unidades de conductancia (G)
ax3 = ax1.twiny()
ax1_xlim = ax1.get_xlim()
print(ax1_xlim)
ax3.set_xlim((1/ax1_xlim[0], 1/ax1_xlim[1]))
ax3.set_xlabel(r"G $[1/k\Omega]$")
ax3.set_xscale("log")

ax1.legend()
ax2.grid()
ax1.grid(which="both", axis="x")
plt.show()
