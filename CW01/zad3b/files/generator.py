import sys
import random
import string

def random_line(line_length):
    alphabet = string.ascii_lowercase
    return "".join([random.choice(alphabet) for i in range(line_length)])

def random_pair_of_files(line_number, line_length, difference):
    file1 = [random_line(line_length) for i in range(line_number)]
    file2 = file1.copy()
    for i in random.sample(range(line_number), int(difference*line_number)):
        file2[i] = random_line(line_length)
    return ("\n".join(file1)+"\n", "\n".join(file2)+"\n")

pairs_number = int(sys.argv[1])
line_number = int(sys.argv[2])
line_length = int(sys.argv[3])
difference = float(sys.argv[4])

for i in range(pairs_number):
    random_pair = random_pair_of_files(line_number, line_length, difference)

    f = open(f'./files/{i}a.txt', "w")
    f.write(random_pair[0])
    f.close()

    f = open(f'./files/{i}b.txt', "w")
    f.write(random_pair[1])
    f.close()
