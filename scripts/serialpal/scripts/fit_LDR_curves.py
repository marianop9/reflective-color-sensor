import pandas as pd
import numpy as np
import json

# constants
Vref = 3.3
Vdd = 5

# retrieve samples for each channel
channels = ["R", "G", "B"]

results = {}

for chan in channels:
    df = pd.read_json(f"measurements_{chan}_2.json").set_index("index")
    df = df.drop("data", axis="columns")
    df = df.rename(columns={"mean": "adc"})

    df["Vo"] = df["adc"] * Vref / 4095 
    df["G"] = (Vref - df["Vo"]) / (df["Rf"]*(Vdd-Vref))

    # apply least squares
    # b = Ax
    b = np.log(df.loc[df["intensity"] > 0, "G"])
    
    c = df.loc[df["intensity"] > 0, "intensity"]
    A = np.column_stack((np.ones(len(c)), np.log(c)))

    x = np.linalg.lstsq(A, b)[0]

    # adjust results
    Ka = np.exp(x[0])
    gamma = x[1]

    results[chan] = dict(
        Ka=Ka,
        gamma=gamma
    )

with open("LDR_params.json", "w") as file:
    json.dump(results, file)

