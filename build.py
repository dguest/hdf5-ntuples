#!/usr/bin/env python3

import numpy as np
import h5py
import copy

# arbitrary leaf type
leaf = np.dtype([('v1', 'f8'), ('v2', 'i')])
# we want to store arbitrary length array of these...
leafs = h5py.special_dtype(vlen=leaf)

# recursive calls to nest leafs in compound datatype
def nest(level=0, do_vl=False):

    # top level condition
    if level == 0:
        return np.array( [(3.4, 1), (3.2, 9)], dtype=leaf), leafs

    # if we're here, recursively nest the data
    subdat, subtype = nest(level - 1, do_vl)

    # make the subnodes variable length
    if do_vl:
        subdat = np.array([copy.deepcopy(subdat) for i in range(2)],
                          dtype=subtype)
        subtype = h5py.special_dtype(vlen=subtype)

    # define the nodes to be compound datatypes which include subnodes
    nodetype = np.dtype([('l', 'i'), ('x', subtype)])
    node = (level, subdat)
    return node, nodetype

f = h5py.File('pytest.h5','w')
l1d, l1t = nest(1, True) # <--- when this is `True` I get opaque datatypes
l1_ds = f.create_dataset('entries', (1,), maxshape=(None,), dtype=l1t)
l1_ds[0] = l1d
f.close()
