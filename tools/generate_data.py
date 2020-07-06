import itertools

alphabet = "abcdefghijklmnopqrstuvwxyz"
f = open("test.txt", "w")
for i in range(1, 6):
    for s in itertools.permutations(alphabet, i):
        f.write(''.join(s) + "\n")