#!/usr/bin/env python3

import numpy as np
import h5py

track = np.dtype([('pt', 'f8'), ('eta', 'f8')])
tracks = h5py.special_dtype(vlen=track)
def get_tracks():
    return np.array( [(3.4, 1), (3.2, 9)], dtype=track)
f = h5py.File('pytest.h5','w')
ds = f.create_dataset('tracks', (2,), maxshape=(None,), dtype=tracks)
ds[0] = np.array( get_tracks() , dtype=track)

vertex = np.dtype([ ('n', 'i'), ('trk', tracks)])
vertices = h5py.special_dtype(vlen=vertex)
newds = f.create_dataset('testds', (100,), maxshape=(None,), dtype=vertices)
vx_entry = [(3, get_tracks()), (2, get_tracks() )]
# print(vx_entry)
newarr = np.array( vx_entry, dtype=vertex)
print(tracks)
# print(newarr.dtype)
newds[0] = newarr
f.close()
