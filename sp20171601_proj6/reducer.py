#!/usr/bin/env python

from itertools import groupby
from operator import itemgetter
import sys

def read_mapper_output(file, separator='\t'):
    for line in file:
        yield line.strip().split(separator, 1)

def main(separator = '\t'):
    data = read_mapper_output(sys.stdin, separator=separator)

    for key, group in groupby(data, itemgetter[0]):
        try:
            maxNum = 0
            for num_group, val in group:
                if float(val) > maxNum:
                    maxNum = float(val)
            print("%s%s%s" %(num_group, separator, maxNum))
        except ValueError:
            pass

if __name__ == "__main__":
    main()
