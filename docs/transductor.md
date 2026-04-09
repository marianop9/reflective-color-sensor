# transductor

Para un LDR:
$$R \approx k E^{-\gamma} = \frac{k}{E^\gamma}$$
donde,
- $E$ -> [[light units#Illuminance|illuminance]]
- $0.6 \le \gamma \le 0.9$ 

## desventajas

- es una aproximación en el mejor de los casos
- $\gamma$ varia mucho entre LDRs
    - requiere calibración
- la iluminancia depende de cuanta luz incide sobre el LDR, que a su vez depende de las propiedades reflectivas de la superficie que se mide
- no responde igual a todas las longitudes de onda del espectro visible

## objetivo

1. Desarrollar un sensor económico que pueda dar una buena aproximación del color de una superficie.
2. Que el mismo pueda ser calibrado contra una paleta de colores especifica y pueda determinar con precisión el color de un objeto cuyo color es uno de los calibrados.

## circuito de acondicionamiento

Amplificador operacional inversor con tensión de referencia.

![[circuito_acondicionador.png]]

Transforma la conductancia $G$ del LDR en una tensión $V_o$:

- $V_s = VCC = 5 \text{ V}$
- $V_+ = 3.3 \text{ V}$


$$
V_o = V_{+} + (V_+ - V_s) \frac{R_f}{R_{LDR}} = V_{+} + (V_+ - V_s) R_f G
$$

- $R_{LDR} \ll R_f \,:\quad V_o \to -\infty \,(0\,\text V)$  
- $R_{LDR} = R_f \,:\quad V_o =V_{+} + (V_+ - V_s) = 3.3 + (5-3.3) = 1.6\,\text V$
- $R_{LDR} \gg R_f \,:\quad V_o \to 3.3\,\text V$

Utiliza múltiples resistencias $R_f$ mediante un mux analógico para evitar la saturación debido al gran rango de resistencia del LDR.

![[acondicionamiento.png]]

La tensión de salida es:
$$
V_o = 3.3 + (3.3 - 5) R_f G
$$

$$
V_o = b + a R_f G
$$

$V_o$ es función de $G$, $V_o(G)$.
## lectura ADC

Suponiendo ADC 12 bits, a 3.3 V:
$$
D = \frac{4095}{3.3\text V} V_o(G)
$$
$$
D = \frac{4095}{3.3\text V} (a+bR_f G)
$$

La resistencia $R_f$ se elige mediante el mux, por lo que es conocida y se puede dividir para quitarla la medición. Se obtiene un valor *"normalizado"* $D^*$:

$$
D^* = D/R_f = \frac{4095}{3.3\text V} (\frac{a}{R_f}+bG)
$$

En cada medición hay que quitar el offset propio del LDR, ya que el $V_o$ medido en ausencia de iluminación (con los LEDs apagados) no necesariamente es 0. El valor *normalizado y compensado* $D'$ es:
$$
D' = D^* - D^*_{\text{dark}}
$$

$D'$ también es  función de $G$: $D'(V_o(G))$.

## interpretación

Cada medición $D'$ se transforma la conductancia $G$ equivalente.

De la ec. del LDR:
$$
G \approx {\alpha}{E^\gamma}
$$

Nos interesa $E$. La iluminancia, o la cantidad de luz que incide sobre la superficie del LDR. Esto nos dará una idea del color de la superficie.

No se conoce ni $\alpha$ ni $\gamma$, son las variables que deseamos calcular.

> En realidad, la luz que incide sobre el LDR depende también de las propiedades de la superficie que se mide. Una superficie mas brillante reflejará mas luz que una superficie opaca. Es por esto que se ajustó el ángulo al que se colocan los LEDs para minimizar la reflexión especular. En este caso se busca medir solo la *reflexión difusa*.


## parametrización del LDR

Se tomarán mediciones con el sensor sobre una superficie blanca arbitraria. Se tomaran medidas para cada canal $[R, G, B]$ por separado, a distintos niveles de intensidad $I$ (ej. $(10,20,\dots,100\%)$).

Para un WS2812, la intensidad de cada canal se codifica en un valor de 8 bits: $I \in [0-255]$

Dado que es imposible conocer la iluminancia $E$ exacta producida por los LEDs en cada nivel de intensidad $I$, lo mejor que podemos hacer es estimar la cantidad de luz que incide sobre su superficie modulando la *intensidad* $I$ (el brillo) de los LEDs. 

Al reducir la intensidad de los LEDs, disminuye la cantidad de fotones emitidos, por lo tanto disminuye la cantidad de luz que recibe el LDR, por lo tanto:

$$
E \approx I
$$

Recolectando las mediciones tenemos un mapa de conductancia-intensidad: $G \leftrightarrow I$

Dado que la lectura del ADC $D'$ es una función lineal de G, podemos expresar:

$$
D' = \frac{4095}{3.3\text V} (\frac{a}{R_f}+b \alpha I^\gamma)
$$

$$
D' = A + BI^\gamma
$$
$A$ es conocida, por lo tanto
$$
D' - A = BI^\gamma
$$
- variables: $B, \gamma$

Aplicando mínimos cuadrados podemos obtener las variables desconocidas:

$$
\ln(D'-A) = \ln(B)+\gamma \ln(I)
$$

$$
\begin{bmatrix}
1 & \ln(I_1) \\
1 & \ln(I_2) \\
\vdots &  \vdots \\
1 & \ln(I_N) & \\
\end{bmatrix}
\begin{bmatrix}
\ln B \\
\gamma 
\end{bmatrix}
\approx 
\begin{bmatrix}
\ln(D'_1-A) \\
\ln(D'_2-A) \\
\vdots \\
\ln(D'_N-A) \\
\end{bmatrix}
$$

Aplicando este proceso para cada canal, se obtienen las variables 
$$
(B_x, \gamma_x) \qquad,\, x \in [R, G, B]
$$

A esta altura, deberíamos tener una buena aproximación de la respuesta LDR para distintas cantidades de luz incidente sobre su superficie.

> Una vez conocidos todos los valores para cada canal, se puede generar una LUT entre los valores del ADC y la intensidad resultante para disminuir el procesamiento requerido en cada medición.

