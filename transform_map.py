#!/usr/bin/python3

from sys import argv as argv

with open(argv[1], 'r') as file:
	lst = [[int(i) for i in l.split(',')]for l in file.read().split('\n') if l != '']

sft = int(input('shift -> '))
scl = float(input('scale -> '))
nlst = [[int(l[i] * scl + .5) + sft for i in range(len(l))] for l in lst]

with open(argv[1], 'w') as file:
	file.write('\n'.join([str(lst[i][0]) + ',' + str(nlst[i][1]) + ',' + str(nlst[i][2]) for i in range(len(lst))]) + '\n')
