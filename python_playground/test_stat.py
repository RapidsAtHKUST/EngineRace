with open('raw/stat.txt') as ifs:
    lines = ifs.readlines()
    lines = [float(line.split()[-9]) for line in lines]
    print sum(lines)