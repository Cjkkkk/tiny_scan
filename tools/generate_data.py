import itertools
import sys

alphabet = "abcdefghijklmnopqrstuvwxyz"

if len(sys.argv) != 3:
    print("usage: python generate_data.py [length] [output]")
    exit(0)

f = open(sys.argv[2], "w")
for i in range(1, int(sys.argv[1])):
    for s in itertools.permutations(alphabet, i):
        f.write(''.join(s) + "\n")