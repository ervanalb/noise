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
    unison = [65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77, None, None, None]

    seqa = (ctypes.POINTER(ctypes.c_double) * len(unison))()
    for i, u in enumerate(unison):
        if u is not None:
            seqa[i] = ctypes.pointer(ctypes.c_double(u))
        else:
            seqa[i] = ctypes.POINTER(ctypes.c_double)()

    #seqa[0] = ctypes.pointer(ctypes.c_double(70.))
    #seqa[1] = ctypes.pointer(ctypes.c_double(71.))
    ##seqa[2] = ctypes.pointer(ctypes.c_double(72.))
    #seqa[2] = ctypes.POINTER(ctypes.c_double)()
    #seqa[3] = ctypes.pointer(ctypes.c_double(74.))
    sequence = cnoise.SEQUENCE_T()
    sequence.length = len(unison)
    sequence.array = seqa

    wb = context.blocks["WaveBlock"]()
    wb2 = context.blocks["WaveBlock"]()
    wb3 = context.blocks["WaveBlock"]()
    cb440 = context.blocks["ConstantBlock"](440.)
    cb200 = context.blocks["ConstantBlock"](20.)
    cb1 = context.blocks["ConstantBlock"](0.05)
    cba = context.blocks["ConstantBlock"](0.10)
    cbt = context.blocks["ConstantBlock"](0.012)
    csaw = context.blocks["ConstantBlock"](1)
    cbseq = context.blocks["ConstantBlock"](cvalue=sequence)
    cbsong = context.blocks["ConstantBlock"](cvalue=sequence)
    ab = context.blocks["AccumulatorBlock"]()
    atime = context.blocks["AccumulatorBlock"]()
    mult = context.blocks["MultiplyBlock"]()
    add = context.blocks["PlusBlock"]()
    add2 = context.blocks["PlusBlock"]()
    lpf = context.blocks["LPFBlock"]()
    lpfnote = context.blocks["LPFBlock"]()
    fgen = context.blocks["FunctionGeneratorBlock"]()
    seq = context.blocks["SequencerBlock"]()
    nfb = context.blocks["NoteToFreqBlock"]()
    ui = context.blocks["UIBlock"]()

    atime.set_input(0, cbt, 0)
    seq.set_input(0, atime, 0)
    seq.set_input(1, cbsong, 0)
    #seq.set_input(1, cbseq, 0)
    nfb.set_input(0, seq, 0)
    lpfnote.set_input(0, nfb, 0)
    lpfnote.set_input(1, cba, 0)
    wb2.set_input(0, lpfnote, 0)
    wb2.set_input(1, csaw, 0)
    mult.set_input(0, wb2, 0)

    ab.set_input(0, cb1, 0)
    fgen.set_input(0, ab, 0)
    #mult.set_input(0, fgen, 0)
    mult.set_input(1, cb200, 0)
    add.set_input(0, mult, 0)
    add.set_input(1, cb440, 0)
    wb.set_input(0, add, 0)
    wb.set_input(1, csaw, 0)
    lpf.set_input(0, wb, 0)
    lpf.set_input(1, cba, 0)
    #wb2.set_input(0, wb, 0)
    wb3.set_input(0, cb440, 0)
    wb3.set_input(1, csaw, 0)
    ui.set_input(0, wb2, 0)

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
