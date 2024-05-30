import sys
from pathlib import Path
import subprocess

CUTOFF_P = 0.2

NTEST_LIST = [
    (10, 100, 1000),
    (15, 100, 1000),
    (20, 100, 1000),
    (25, 100, 1000),
    (30, 100, 1000),
    (50, 100, 1000),
    (60, 100, 1000),
    (70, 100, 1000),
    (80, 100, 1000),
    (90, 100, 1000),
    (100, 100, 1000),
    (125, 80, 1000),
    (150, 80, 100),
    (175, 80, 100),
    (200, 50, 100),
    (300, 50, 100),
    (400, 50, 100),
    (500, 20, 100),
    (600, 20, 100),
    (750, 20,  50),
    (900, 20, 50),
    (1000, 10, 50),
    (1500, 5, 10),
    (2000, 5, 10),
    (2500, 1, 10),
]


def test(N, operations, NUM_ITER):
    
    times = []
    CUTOFF = int(NUM_ITER * CUTOFF_P)
    for i in range(NUM_ITER):
        xs = subprocess.run((sys.argv[1], str(N), str(operations)), text=True, capture_output=True)
        times.append(int(xs.stdout.split(": ")[1].split("ms")[0]))

    decent = sorted(times)[CUTOFF:NUM_ITER-CUTOFF]
    
    return {
        'iterc': NUM_ITER,
        'ops': operations,
        'avg': (sum(decent) / (NUM_ITER - 2*CUTOFF)) / operations
    }



def main():
    if (not Path(sys.argv[1] if len(sys.argv) > 1 else "").is_file()):
        print("ERROR! Expected valid executable path as argument", file=sys.stderr)
        return 1
    
    results = {}

    for N in NTEST_LIST:
        results[N[0]] = test(*N)
        print(f"TESTED {N}!", results[N[0]])

    input("Pres ENTER to dump")
    with open(sys.argv[1] + ".json", 'w') as f:
        import json
        json.dump(results, f)
    input("Pres ENTER to finish")

    return 0
    
if __name__ == '__main__':
    exit(main())

