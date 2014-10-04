import cnoise
import ctypes
import ntype
import pyaudio
import struct


CHUNKSIZE=128
FRAMERATE=48000

global_vars=[(ctypes.c_int,'global_chunk_size',CHUNKSIZE),(ctypes.c_int,'global_frame_rate',FRAMERATE)]


context=cnoise.NoiseContext(global_vars)

context.load('blocks.py')

#n_double=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)),'double')
#n_int=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_int)),'int')
#n_chunk=ntype.TypeFactory(lib['noise'].simple_alloc,lib['noise'].simple_free,lib['noise'].simple_copy,ctypes.c_int(ctypes.sizeof(ctypes.c_double)*CHUNKSIZE),'chunk')

if __name__ == "__main__":
    p = pyaudio.PyAudio()

    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=FRAMERATE,
        output=True)
    wb = context.blocks["WaveBlock"]()
    ui = context.blocks["UIBlock"]()
    wb.set_input(0, ui, 0)
    ui.set_input(0, wb, 0)

    while True:
        try:
            result = ui.pull()
            data=struct.pack('f'*CHUNKSIZE,*(ui.output[:CHUNKSIZE]))
            stream.write(data)
        except KeyboardInterrupt:
            break
 
    stream.stop_stream()
    stream.close()
