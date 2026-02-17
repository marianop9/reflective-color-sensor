import json

from .serialpal import Response


class Measurement:
    def __init__(self, chan, index, intensity, data, mean, Rf):
        self.channel = chan
        self.index = index
        self.intensity = intensity
        self.data = data
        self.mean = mean
        self.Rf = Rf

    @classmethod
    def from_response(cls, chan: str, index: int, intensity: int, resp: Response, mean: int, Rf: int):
        return cls(chan, index, intensity, resp.payload, mean, Rf)

    @classmethod
    def from_dict(cls, obj):
        return cls(obj["channel"], obj["index"], obj["intensity"], obj["data"], obj["mean"], obj["Rf"])

    def __str__(self):
        return f"{self.channel},{self.index},{self.Rf},{self.data[:3]} ({self.mean})"


def _measurement_encoder(obj):
    if isinstance(obj, Measurement):
        return {"channel": obj.channel,
                "index": obj.index,
                "intensity": obj.intensity,
                "mean": obj.mean,
                "data": obj.data,
                "Rf": obj.Rf}


def export_measurements_json(measurements: list[Measurement], chan: str, version = 0):
    fname = f"measurements_{chan}_{version if version > 0 else ""}.json"
    
    with open(fname, "w") as file:
        json.dump(measurements, file, default=_measurement_encoder)


def parse_measurements_json(path: str) -> list[Measurement]:
    dicts: list[dict]
    with open("measurements.json", "r") as file:
        dicts = json.load(file)

    measurements = []
    for d in dicts:
        measurements.append(Measurement.from_dict(d))

    return measurements
