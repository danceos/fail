#!/usr/bin/env python

# -------------------------------------------------------------------------
# Import modules
import csv, sys
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.patches as mpatches
import matplotlib.pylab as pylab

ENTRY_COUNT_PER_LINE = 6
RESULT_RECT_HEIGHT   = 1.0/8

# -------------------------------------------------------------------------
# Basic drawing class
class Rectangle:
    def __init__(self, x, y, w, h, color):
                              #           w
        self.xpos    = x      # |-------------------|
        self.ypos    = y      # |                   | h
        self.width   = w      # |                   |
        self.height  = h      # (x,y)---------------|
        self.color   = color  # fill color
        self.alpha   = None   # transparency level [0,1]
    def draw(self):
        """ Draw the rectangle. """
        if self.color == (1,1,1):
            return
        tmp = self.xpos, self.ypos,
        p = mpatches.Rectangle(tmp, self.width, self.height, color=self.color, \
                               alpha=self.alpha)
        plt.gca().add_patch(p)

# -------------------------------------------------------------------------
# Check provided arguments:
if len(sys.argv) <= 1:
    print "ERROR: Not enough arguments provided! See -h for more infos."
    exit(1)
if sys.argv[1] == '-h':
    print "Displays experiment results for the weather-monitor-experiment."
    print "    CALL-SYNTAX: fsp.plot.py DATA_FILE [USER_TAG_FILE]"
    print "DATA_FILE is a CSV-file, storing the tab-separated values"
    print "retrieved by the experiment run. USER_TAG_FILE is an optional"
    print "CSV-file which can be used to add user-specific marks to the"
    print "plot." # TODO: be more precise here
    exit(0)

print "Opening and processing \"" + sys.argv[1] + "\"..."
file = open(sys.argv[1], "r")
dialect = csv.Sniffer().sniff(file.read(1024))
file.seek(0)
reader = csv.reader(file, dialect)
reader.next() # Move down a line to skip the header

if len(sys.argv) >= 3:
    print "Opening and processing \"" + sys.argv[2] + "\"..."
    symbolfile = open(sys.argv[2], "r")
    dialect = csv.Sniffer().sniff(symbolfile.read(1024))
    symbolfile.seek(0)
    symbolreader = csv.reader(symbolfile, dialect)
    symbolreader.next() # Move down a line to skip the header
    have_symbols = True
else:
    have_symbols = False

fig = plt.figure()

xmin = 99999999
xmax = 0
ymin = 0xffffffff
ymax = 0

line_counter = 1
for row in reader:
    line_counter += 1
    # Check if there are at least ENTRY_COUNT_PER_LINE entries per line:
    if len(row) != ENTRY_COUNT_PER_LINE:
        print "ERROR: Line " + str(line_counter) + " is invalid (" +\
              str(ENTRY_COUNT_PER_LINE) + " entries expected)"
        sys.exit(1)

    # Some constants to access the row-entries much easier:
    IDX_INSTR1 = 0; IDX_INSTR2 = 1; IDX_DATA_ADDRESS = 2;
    IDX_BITNR = 3; IDX_BITWIDTH = 4; IDX_COLOR = 5;

    # Update xmin/xmax/ymin/ymax
    x1 = int(row[IDX_INSTR1])
    x2 = int(row[IDX_INSTR2]) + 1 # inclusive
    width = x2 - x1
    if xmin > x1:
        xmin = x1
    if xmax < x2:
        xmax = x2

    y1 = float(row[IDX_DATA_ADDRESS]) + float(row[IDX_BITNR]) / 8.0
    height = float(row[IDX_BITWIDTH]) / 8.0
    y2 = y1 + height

    if ymin > y1:
        ymin = y1
    if ymax < y2:
        ymax = y2

    Rectangle(x1, y1, width, height, row[IDX_COLOR]).draw()
    if line_counter == 50000: # debug stuff
        pass
        #break

# round down to nearest 1000
#ymin = int(ymin / 1000) * 1000

file.close()
plt.xlim(xmin, xmax)
plt.ylim(ymin, ymax)

plt.ylabel('Data Memory (RAM)')
plt.xlabel('Time (Cycles)')

# show symbols
if have_symbols:
    # Adding 2nd y-axis for object names
    ax1 = fig.add_subplot(111)
    ax2 = ax1.twinx()
    ax2.set_ylabel('Symbols')

    # necessary for proper symbol placement
    plt.ylim(ymin, ymax)

    ticks = []
    symbols = []

    IDX_SYMBOL_ADDRESS = 0; IDX_SYMBOL_SIZE = 1; IDX_SYMBOL_NAME = 2;

    for row in symbolreader:
        # TODO: somehow plot size as well
        address = int(row[IDX_SYMBOL_ADDRESS])
        size = int(row[IDX_SYMBOL_SIZE])
        if (address >= ymin) and (address <= ymax):
            ticks.append(address)
            symbols.append(row[IDX_SYMBOL_NAME])
            #print "symbol: " + str(address) + " " + str(size) + " " + row[IDX_SYMBOL_NAME]
        elif (address < ymin) and (address + size >= ymin):
            ticks.append(ymin)
            symbols.append("(" + row[IDX_SYMBOL_NAME] + ")")
            #print "partial symbol: " + str(address) + " " + str(size) + " " + row[IDX_SYMBOL_NAME]
        else:
            #print "skipped symbol: " + str(address) + " " + str(size) + " " + row[IDX_SYMBOL_NAME]
            pass

    # list of interesting addresses
    ax2.yaxis.set_ticks(ticks)
    ax2.yaxis.grid(b=True, color='gray')

    # create minor ticks centered between major tick marks
    centered_ticks = []
    major_locator = ax2.yaxis.get_major_locator()
    major_locs = major_locator()
    for i in range(1,len(major_locs)):
        y_last,y = major_locs[i-1],major_locs[i]
        centered_ticks.append(0.5*(y+y_last))
    ax2.yaxis.set_ticks(centered_ticks, minor=True)

    # list of corresponding symbol names
    ax2.set_yticklabels(symbols, minor=False, rotation=0, rotation_mode='anchor')

plt.show()
#pylab.savefig('baseline.pdf', bbox_inches='tight')
