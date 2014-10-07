import cnoise
import ctypes
import struct
import ntype

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

a_t = context.types['array'](5,context.types['int'])

a=a_t.new()

print a.value

a.value=[1,2,3,4,5]

print a.value

a.value=[5,4,3,2,1]

print a.value
