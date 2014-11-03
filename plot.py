import sys
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy
#import matplotlib
from collections import OrderedDict

tableau20 = None

COLORZ = True

def settings():
    # These are the "Tableau 20" colors as RGB.  
    global tableau20
    tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),  
                 (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),  
             (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),  
             (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),  
             (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]  
  
    # Scale the RGB values to the [0, 1] range, which is the format matplotlib accepts.  
    for i in range(len(tableau20)):  
        r, g, b = tableau20[i]  
        tableau20[i] = (r / 255., g / 255., b / 255.)


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
            if not scale_by: scale_by = 1#float(lines[i+1])
            if not data.has_key(lines[i]):
                data[lines[i]] = OrderedDict()
                # initialize previously missed values to 0
                for l in labels:
                    data[lines[i]][l] = 0
            data[lines[i]][cur_name] = float(lines[i+1]) / scale_by
            next(it)

    print data
    
    cur_width = 0
    if COLORZ:
        colors = tableau20
    else:
        colors = [(0,0,0), (.5,.5,.5), (.7,.7,.7)]
    n_datapoints = len(data.values()[0].values())
    inds = numpy.arange(n_datapoints)
    n_types = len(data.values())
    width = .7 / n_types
    bars = []
    for (d, c) in zip(data.values(), colors):
        pts = d.values()
        bars.append(ax.bar(inds + cur_width + .1, pts, width, color=c))
        cur_width += width

    ax.set_xticks(inds + width * n_types / 2.0)
    ax.set_xticklabels(labels, size=10)
    ax.set_ylabel('Thousands of transactions per second', rotation=90)
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, pos: str(int(x/1000))))
    ax.set_aspect(1/900000. * 2)
    plt.xlim()
    ax.legend(map(lambda x: x[0], bars), data.keys(), ncol=4, loc='upper left', prop={'size': 10})
#loc='lower center', bbox_to_anchor=(.9, .05) )

    ax.set_title(title)

    plt.tight_layout()

    return plt

try:
    title = sys.argv[3]
except:
    title = ''

try:
    COLORZ = sys.argv[4] != 'bw'
except:
    pass

settings()
plot(sys.argv[1], title).savefig(sys.argv[2])
