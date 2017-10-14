from parse import *
import sys
import matplotlib.pyplot as plt


t2i = {}


def init_fib():
    a, b = 0, 1
    for i in range(32):
        a, b = b, a+b
        t2i[b] = i+1


class Stat:
    def __init__(self, meta, achieve):
        self.meta = meta
        self.achieve = achieve

    def info(self):
        print(self.meta)
        print(self.achieve)


def to_chunks(lines):
    chunks = []
    current_chunk = []
    for line in lines:
        if line.startswith(tuple(str(i) for i in range(10))) and current_chunk:
            chunks.append(current_chunk[:])
            current_chunk = []
        if line.strip():
            current_chunk.append(line)
    chunks.append(current_chunk)
    return chunks


def parse_file(filename):
    try:
        f = open(filename)
    except IOError:
        print("Could not read file: " + filename)
        sys.exit()

    stats = []
    with f:
        chunks = to_chunks(f.readlines()[2:])
        for chunk in chunks:
            p = parse("{:d}\tavg = {:d}, max = {:d}, ops = {:d}", chunk[0])
            meta = dict(zip(('iteration', 'avg', 'max', 'ops'), p.fixed))
            achieve = {}
            for line in chunk[1:]:
                winrate = parse("\t{:d}\t{:f}%\t({}%)", line)
                if not winrate:
                    winrate = parse("\t{:d}\t{:d}%\t({}%)", line)
                achieve[t2i[winrate.fixed[0]]] = winrate.fixed[1]
            s = Stat(meta, achieve)
            stats.append(s)
    return stats


def plot(stats):
    avgs = []
    winrates = []
    for stat in stats:
        avgs.append(stat.meta['avg'])
        winrates.append(stat.achieve.get(17, 0))
    plt.subplot(211)
    plt.title("Win Rate (%)")
    plt.plot([i+1 for i in range(len(avgs))], winrates)
    plt.subplot(212)
    plt.title("Average Score")
    plt.plot([i+1 for i in range(len(avgs))], avgs)
    title = "Fibonacci-2584 game trained {:,d} iterations".format(stats[-1].meta['iteration'])
    plt.gcf().canvas.set_window_title(title)
    plt.show()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Bad argv number")
        sys.exit()
    init_fib()
    stats = parse_file(sys.argv[1])
    plot(stats)
