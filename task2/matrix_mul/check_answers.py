"""
Usage:
    check.py <log_file_name>
"""

from sys import argv
from pathlib import Path
from numpy import matrix

def main():
    log_file = Path(argv[1] if len(argv) > 1 else "matrix_mul.log")
    if not log_file.is_file():
        print("ERROR! Please provide valid log file to check")
        return 1

    text = log_file.read_text().splitlines()
    text += ['']
    i = 0

    def read_matrix():
        nonlocal i, text
        line = text[i]
        i+=1
        rows, cols = (int(j) for j in line.split("<")[1].split(">")[0].split(","))
        
        matrix = [[int(j) for j in row.split()] for row in text[i:i + rows]]
        i += rows
        return matrix

    while (text[i].startswith("Operation")):
        i += 1
        a = read_matrix()
        b = read_matrix()
        p = read_matrix()
        if not (matrix(a) * matrix(b) == p).all():
            print(f"ERROR! Multiplication before line {i} did not match")
            return 1
    print("SUCCESS!")
    return 0

if __name__ == '__main__':
    exit(main())