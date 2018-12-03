if __name__ == '__main__':
    def replace_line(line):
        return line.replace('INFO', '').replace('ts: ', ''). \
            replace('engine_race.cc:502:', '').replace('engine_race.cc:523:', '').replace('Bucket 0', '')


    with open('raw/log12-03-12:54.txt') as ifs:
        lines = filter(lambda line: 'Bucket 0 ' in line, ifs.readlines())
    with open('raw/bucket0stat.txt', 'w') as ofs:
        ofs.writelines(map(replace_line, lines))
