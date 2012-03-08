#!/usr/bin/python
import os
import glob
import re
from numpy import *
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import pylab as pli
import matplotlib as mpl


result = {
	'EDM , TRAP': zeros((32,16), dtype=int), 
	'FAIL_SILENT': zeros((32,16), dtype=int), 
	'FAIL_SILENT_VIOLATION': zeros((32,16), dtype=int) , 
	'TIMEOUT': zeros((32,16), dtype=int) }

registertype = 'X'

def eval_fault(bitnumber, regtype,  regnumber, faulttype):
	regn = int(regnumber)
	bitn = int(bitnumber)
	result[faulttype][bitn,regn] +=1

def eval_file(filename):
	# parse filename: bitnumber and register type
	pattern = re.compile('.-(\S{1})-bit(\d{1,2})')
	exp = pattern.search(filename).groups()
	bitnumber = exp[1]
	regtype = exp[0]
	#  parse file: Fault type (FAIL_SILENT) and register number d(14)
	pattern = re.compile('(.*)\s+-\s+\S{1}(.{1,2})') 
	with open(filename, 'r') as f:
		for line in f:
			if line.startswith('='): 
				break
			exp = pattern.search(line).groups()
			eval_fault(bitnumber, regtype, exp[1].rstrip(), exp[0].rstrip())
	return regtype.rstrip()



def print_results(ftype):
	print ftype
	for i in range(16):
		print '\t' + registertype +  str(i) ,
	j = 0
	print ''
	for row in result[ftype]:
		print 'bit' + str(j) + '\t',
		for i in row:
			print str(i) + '\t',
		print ''
		j+=1
	print ''
 	return


def print_plot(what, title, subplot, regtp):
	X = result[what]
	ax = fig.add_subplot(subplot)
	cax = ax.imshow(X, cmap=cm.gist_yarg, interpolation='nearest')
	ax.set_title(title, verticalalignment='baseline')
	ax.invert_yaxis()
	ax.set_xticks(arange(16))
	ax.set_yticks(arange(32))
	ax.set_xlabel(regtp)	
	ax.set_ylabel('Bit#')
	cbar = fig.colorbar(cax, format='%d', shrink=0.5 , ticks=linspace(0,53,25) );
	
	for label in ax.get_xticklabels() + ax.get_yticklabels() + cbar.ax.get_yticklabels():
		label.set_fontsize(10)


#### Set desired path of FI results here###
path = 'exp3'
#### ^^^^^^^ Set desired path of FI results here###

## Traverse all *.dat files in path:
for infile in glob.glob( os.path.join(path, '*.dat')):
	registertype = eval_file(infile)

#for faulttype in result:
#	print_results(faulttype)

## Set verbose register name
if registertype == 'd':
	regtp = 'Data Registers'
elif registertype == 'a':
	regtp = 'Address Registers'

# plot figure.
fig = plt.figure()
#plt.suptitle('Distribution of Fault Injection Results ' +'(' + regtp + ')', fontsize=12 )
print_plot('FAIL_SILENT_VIOLATION', 'Fail-Silent Violations',121 , regtp)
print_plot('EDM , TRAP', 'Traps', 122, regtp)
#print_plot('FAIL_SILENT', 'Fail Silent', 221, regtp)
#print_plot('TIMEOUT', 'Timeout', 224, regtp)
plt.show()


