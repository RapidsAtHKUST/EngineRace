if __name__ == '__main__':
    with open('input_data/write_sample0.txt') as ifs:
        lines = ifs.readlines()
        tuples = [line.split() for line in lines]
        for t in tuples:
            print t[-5]
        for t in tuples:
            print t[7]
