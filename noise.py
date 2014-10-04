import cnoise
import ctypes
import ntype
import pyaudio
import struct


context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

#n_double=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)),'double')
#n_int=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_int)),'int')
#n_chunk=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)*context.chunk_size),'chunk')

if __name__ == "__main__":
    p = pyaudio.PyAudio()

    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=context.frame_rate,
        output=True)

    wb = context.blocks["WaveBlock"]()
    wb2 = context.blocks["WaveBlock"]()
    cb440 = context.blocks["ConstantBlock"](440.)
    cb200 = context.blocks["ConstantBlock"](200.)
    cb1 = context.blocks["ConstantBlock"](0.01)
    cba = context.blocks["ConstantBlock"](0.9)
    ab = context.blocks["AccumulatorBlock"]()
    mult = context.blocks["MultiplyBlock"]()
    add = context.blocks["PlusBlock"]()
    lpf = context.blocks["LPFBlock"]()
    fgen = context.blocks["FunctionGeneratorBlock"]()
    ui = context.blocks["UIBlock"]()

    ab.set_input(0, cb1, 0)
    fgen.set_input(0, ab, 0)
    mult.set_input(0, fgen, 0)
    mult.set_input(1, cb200, 0)
    add.set_input(0, mult, 0)
    add.set_input(1, cb440, 0)
    wb.set_input(0, add, 0)
    lpf.set_input(0, wb, 0)
    lpf.set_input(1, cba, 0)
    #wb2.set_input(0, wb, 0)
    ui.set_input(0, wb, 0)

    while True:
        try:
            #cb.cvalue.value += 10
            result = ui.pull()
            data=struct.pack('f'*context.chunk_size,*(ui.output[:context.chunk_size]))
            stream.write(data)
        except KeyboardInterrupt:
            break
 
    stream.stop_stream()
    stream.close()
