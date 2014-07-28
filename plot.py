import sys
import matplotlib.pyplot as plt
import numpy
#import matplotlib
from collections import OrderedDict

def settings():
    pass

def plot(dat, title):
    fig, ax = plt.subplots()

    f = open(dat, 'r')
    first = True
    labels = []
    lines = f.readlines()
    data = OrderedDict()
    scale_by = None
    cur_name = None
    it = iter(xrange(len(lines)))
    for i in it:
        if first:
            cur_name = lines[i]
            labels.append(lines[i])
            first = False
        elif lines[i] == '\n':
            for d in data.values():
                if not d.has_key(cur_name):
                    d[cur_name] = 0
            first = True
            scale_by = None
        else:
            print lines[i], lines[i+1]
            if not scale_by: scale_by = float(lines[i+1])
            if not data.has_key(lines[i]):
                data[lines[i]] = OrderedDict()
                # initialize previously missed values to 0
                for l in labels:
                    data[lines[i]][l] = 0
            data[lines[i]][cur_name] = float(lines[i+1]) / scale_by
            next(it)

    print data
    
    cur_width = 0
    colors = ['r', 'g', 'b']
    n_datapoints = len(data.values()[0].values())
    inds = numpy.arange(n_datapoints)
    n_types = len(data.values())
    width = .7 / n_types
    bars = []
    for (d, c) in zip(data.values(), colors):
        pts = d.values()
        bars.append(ax.bar(inds + cur_width, pts, width, color=c))
        cur_width += width

    ax.set_xticks(inds + width * n_types / 2.0)
    ax.set_xticklabels(labels, size=10)
    ax.legend(map(lambda x: x[0], bars), data.keys(), loc='upper center', bbox_to_anchor=(.9, 1.1) )

    ax.set_title(title)

    return plt

try:
    title = sys.argv[3]
except:
    title = ''

plot(sys.argv[1], title).savefig(sys.argv[2])
