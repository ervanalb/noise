import cnoise
import ctypes
import struct
import ntype

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

c=context.blocks['ConstantBlock']()

n=context.types['int'].new()
n.value=3

c.pointer=n.o


