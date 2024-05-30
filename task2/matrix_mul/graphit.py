import sys
from pathlib import Path
import subprocess

EXECUTABLES = [
    "parallel.exe",
    "parallel10000_1000.exe",
    "parallel1000_10000.exe",
    "parallel10000_10000.exe",
    "parallel100_100.exe",
    "sequential.exe",
]

CUTOFF_P = 0.2

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
    (100, 1000, 100),
    (125, 800, 100),
    (150, 500, 100),
    (175, 100, 100),
    (200, 50, 100),
    (300, 50, 100),
    (400, 50, 100),
    (500, 20, 100),
    (600, 15, 100),
    (750, 10,  50),
    (900, 10, 50),
    (1000, 10, 50),
    (1500, 5, 10),
    (2000, 5, 10),
    (2500, 1, 10),
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
        import json
        json.dump(results, f)
    # input("Pres ENTER to finish")

    return 0
    
if __name__ == '__main__':
    for path in EXECUTABLES:
        main(path)

