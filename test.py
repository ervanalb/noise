import cnoise
import ctypes
import struct
import ntype

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

#n_double=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)),'double')
#n_int=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_int)),'int')
#n_chunk=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)*CHUNKSIZE),'chunk')

wb = context.blocks["WaveBlock"]()
ui = context.blocks["UIBlock"]()
wb.set_input(0, ui, 0)
ui.set_input(0, wb, 0)

while True:
    result = ui.pull()
    data=struct.pack('f'*context.chunk_size,*(ui.output[:context.chunk_size]))
    print ui.output[:context.chunk_size]
