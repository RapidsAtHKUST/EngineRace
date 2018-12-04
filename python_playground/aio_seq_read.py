import matplotlib.pyplot as plt

with open('raw/aio-1024buckets.txt') as ifs:
    lines = filter(lambda line: 'In bucket' in line, ifs.readlines())
    print len(lines)
    lines = [float(line.split()[-2]) for line in lines]
    print lines

    print 'avg:', sum(lines) / len(lines)
    print 'min:', min(lines), 'max', max(lines)
