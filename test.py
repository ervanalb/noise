import cnoise
import ctypes
import struct
import ntype

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

w_t = context.types['wave'](5).new([1,2,3,4,5])

print w_t.value

w_t.value = [2,3,4,5,6]

print w_t.value
