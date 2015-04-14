#!/usr/bin/env python
import sys, time
import argparse

LOGFILE = sys.stderr
IN_STREAM   = sys.stdin
OUT_STREAM  = sys.stdout


parser = argparse.ArgumentParser()

parser.add_argument("-f","--outfile", help="path to optional output file")
parser.add_argument("-v", "--verbose", action="store_true", help="increase output verbosity")

args = parser.parse_args()


    
try:
    if args.outfile:
        OUT_STREAM = open(args.outfile,'w')

    for line in IN_STREAM:
        if args.verbose:
            OUT_STREAM.write("PUSHING: ")
        OUT_STREAM.write(line)
finally:
    LOGFILE.close()
    IN_STREAM.close()
    OUT_STREAM.close()
