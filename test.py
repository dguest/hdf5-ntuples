#!/usr/bin/env python3

from h5py import File
import argparse

def _get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    args = parser.parse_args()
    return args

def run():
    args = _get_args()
    with File(args.input_file, 'r') as h5:
        vals = h5['entries']
        print(vals[0])
        # for num, entry in enumerate(mid_level[4]):
            # print(num, entry)

if __name__ == '__main__':
    run()
