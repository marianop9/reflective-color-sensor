import matplotlib.pyplot as plt


class Plotter():
    def __init__(self, title="Mediciones"):
        self.fig, self.ax = plt.subplots()
        self.ax.set_title(title)
        self.ax.set_xlabel("n")
        self.ax.set_ylabel("ADC")
        self.ax.grid(True)
        self.k = 0

    def add_batch(self, y, label=None):
        self.k += 1
        if label is None:
            label = f"meas {self.k}"

        self.ax.plot(y, marker=".", linestyle="-", label=label)

    def show(self):
        self.ax.legend()
        plt.show()
    # def __init__(self, title="Mediciones"):
    #     # plt.ion()
    #     self.create_figure()
    #     self.lines = []
    #     self.k = 0

    # def create_figure(self, title="Mediciones"):
    #     self.fig, self.ax = plt.subplots()
    #     self.ax.set_title(title)
    #     self.ax.set_xlabel("n")
    #     self.ax.set_ylabel("ADC")
    #     # self.ax.set_ylim(1000, 4095)
    #     self.ax.grid(True)

    # def add_batch(self, y, label=""):
    #     self.ensure_figure()

    #     x = range(len(y))
    #     self.k += 1
    #     if label == "":
    #         label = f"meas {self.k}"

    #     line, = self.ax.plot(x, y, marker=".", linestyle="-", label=label)
    #     self.lines.append(line)

    # def show(self):
    #     self.ax.legend()
    #     self.ax.relim()
    #     self.ax.autoscale_view()
    #     self.ensure_figure()
    #     self.fig.canvas.draw()
    #     self.fig.canvas.flush_events()
    #     self.fig.show()


    # def ensure_figure(self):
    #     if self.fig is None or not plt.fignum_exists(self.fig.number):
    #         self.create_figure()
    #         self.lines = []
    #         self.k = 0

    def add_test_data(self, dataset) -> tuple[tuple[int]]:
        # test ADC data
        # low end
        a = (119, 121, 117, 120, 119, 118, 119, 119, 120, 122, 122, 120, 121,
             121, 120, 120, 120, 117, 121, 120, 118, 119, 121, 117, 120, 120,
             119, 120, 120, 120, 119, 121, 120, 120, 122, 120, 122, 120, 121,
             121)
        # mid end
        b = (1797, 1793, 1789, 1798, 1792, 1801, 1794, 1791, 1792, 1793, 1789,
             1792, 1790, 1793, 1797, 1798, 1796, 1792, 1795, 1793, 1794, 1794,
             1792, 1794, 1794, 1795, 1793, 1793, 1792, 1792, 1787, 1799, 1794,
             1795, 1786, 1798, 1791, 1792, 1793, 1792)
        # hi end
        c = (3816, 3815, 3809, 3816, 3816, 3814, 3818, 3814, 3813, 3816, 3812,
             3814, 3816, 3811, 3811, 3816, 3815, 3814, 3816, 3814, 3812, 3816,
             3815, 3814, 3815, 3816, 3811, 3813, 3817, 3814, 3815, 3812, 3814,
             3817, 3814, 3813, 3813, 3812, 3814, 3818)

        if dataset == "a":
            self.add_batch(a)
            return (a,)
        elif dataset == "b":
            self.add_batch(b)
            return (b,)
        elif dataset == "c":
            self.add_batch(c)
            return (c,)
        else:
            self.add_batch(a)
            self.add_batch(b)
            self.add_batch(c)
            return (a, b, c)
