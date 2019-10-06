#!/usr/bin/env python

import sys

mapped = {}
def read_input(file):
    for line in file:
        yield line.strip().split(',')

def main(separator='\t'):
    data = read_input(sys.stdin)

    for li in data:
        print("%d%s%f" %(int (li[0]), separator, float(li[1])))

if __name__ == "__main__":
    main()
