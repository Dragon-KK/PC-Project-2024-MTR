import sys
from pathlib import Path
import subprocess
import json

from scipy.interpolate import interp1d
import numpy as np
import matplotlib.pyplot as plt

# parallelX_Y.exe: X -> QUEUE_CHUNK_SIZE, Y -> MAX_LIVE_CHUNKS

STORE_DATA = False
PLOT_DATA = True

EXECUTABLES = [
    "parallel10000_10000.exe",
    "parallel.exe",
    "parallel10000_1000.exe",
    "parallel1000_10000.exe",
    "parallel100_100.exe",
    "sequential.exe",
]

CUTOFF_P = 0.1

NTEST_LIST = [
    (10, 1000, 100),
    (15, 1000, 100),
    (20, 1000, 100),
    (25, 1000, 100),
    (30, 1000, 100),
    (50, 1000, 100),
    (60, 1000, 100),
    (70, 1000, 100),
    (80, 1000, 100),
    (90, 1000, 100),
    (100, 100, 100),
    (125, 80, 100),
    (150, 50, 100),
    (175, 10, 100),
    (200, 5, 10),
    (300, 5, 10),
    (400, 5, 10),
    (500, 5, 10),
    (600, 5, 10),
    (750, 5, 5),
    (900, 5, 5),
    (1000, 5, 5),
    (1500, 2, 5),
    (2000, 1, 5),
    (2500, 1, 5),
]


def test(path, N, operations, NUM_ITER):
    
    times = []
    CUTOFF = int(NUM_ITER * CUTOFF_P)
    for i in range(NUM_ITER):
        xs = subprocess.run((path, str(N), str(operations)), text=True, capture_output=True, creationflags=subprocess.HIGH_PRIORITY_CLASS)
        times.append(int(xs.stdout.split(": ")[1].split("ms")[0]))

    decent = sorted(times)[CUTOFF:NUM_ITER-CUTOFF]
    
    return {
        'iterc': NUM_ITER,
        'ops': operations,
        'avg': (sum(decent) / (NUM_ITER - 2*CUTOFF)) / operations
    }



def main(path):
    if (not Path(path).is_file()):
        print("ERROR! Expected valid executable path as argument", file=sys.stderr)
        return 1
    
    results = {}
    print("TESTING", path, "!")

    for N in NTEST_LIST:
        results[N[0]] = test(path, *N)
        print(f"TESTED {N}!", results[N[0]])

    # input("Pres ENTER to dump")
    with open(path + ".json", 'w') as f:
        
        json.dump(results, f)
    # input("Pres ENTER to finish")

    return 0
    
if __name__ == '__main__':
    if STORE_DATA:
        for path in EXECUTABLES:
            main(path)

    if PLOT_DATA:
        avgs = {}
        for path in EXECUTABLES:
            with open(path + ".json") as f:
                xs = json.load(f)
                avgs[path] = ([i for i in xs], [xs[i]['avg'] for i in xs])

        for path in EXECUTABLES:
            x = np.array(avgs[path][0])
            n = np.arange(x.shape[0]) 
            y = np.array(avgs[path][1])


            x_spline = interp1d(n, x,kind='cubic')

            n_ = np.linspace(n.min(), n.max(), 500)
            y_spline = interp1d(n, y,kind='cubic')

            x_ = x_spline(n_)
            y_ = y_spline(n_)



            plt.plot(x_, y_, label=path)
        plt.xlabel("N")
        plt.ylabel("Average time taken per multiplication")
        plt.title("Time taken by different implementations")
        plt.legend(loc="upper left")
        plt.show()