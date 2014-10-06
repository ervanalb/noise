import cnoise
import ctypes
import struct
import ntype

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

i_t=context.types['int']
a=context.types['array'](5,context.types['int'])

print i_t
i=i_t(5)
print i.value
i.value=6
print i.value
