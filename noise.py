import cnoise
import ctypes
import ntype
import pyaudio
import struct
#import nanokontrol
import threading
import scipy.signal

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

if __name__ == "__main__":


    p = pyaudio.PyAudio()
    try:
        nk = nanokontrol.NanoKontrol2()
    except:
        nk = None
    #nkm = nanokontrol.Map

    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=context.frame_rate,
        frames_per_buffer=context.chunk_size,
        output=True)

    print stream.get_output_latency()

    unison = [65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77, None, None, None]
    n=101
    lpf = scipy.signal.firwin(101,.05)
    hpf = -lpf
    hpf[n/2]+=1

    echo = [.7]+[0]*500+[.3]

    f=lpf

    n_double=context.types['double']
    n_int=context.types['int']
    n_wave=context.types['wave']

    song=context.types['array'](len(unison),n_double).new(unison)

    wb = context.blocks["WaveBlock"]()
    wb2 = context.blocks["WaveBlock"]()
    wb3 = context.blocks["WaveBlock"]()
    cb440 = context.blocks["ConstantBlock"](n_double.new(440))
    cb200 = context.blocks["ConstantBlock"](n_double.new(20))
    cb1 = context.blocks["ConstantBlock"](n_double.new(0.05))
    cba = context.blocks["ConstantBlock"](n_double.new(0.10))
    cbt = context.blocks["ConstantBlock"](n_double.new(0.012))
    csaw = context.blocks["ConstantBlock"](n_int.new(1))
    cbsong = context.blocks["ConstantBlock"](song)
    ab = context.blocks["AccumulatorBlock"]()
    atime = context.blocks["AccumulatorBlock"]()
    mult = context.blocks["MultiplyBlock"]()
    add = context.blocks["PlusBlock"]()
    add2 = context.blocks["PlusBlock"]()
    lpf = context.blocks["LPFBlock"]()
    lpfnote = context.blocks["LPFBlock"]()
    fgen = context.blocks["FunctionGeneratorBlock"]()
    seq = context.blocks["SequencerBlock"](song) # could also call this with a type
    nfb = context.blocks["NoteToFreqBlock"]()
    filt = context.blocks["ConstantBlock"](n_wave(len(f)).new(f))
    fb = context.blocks["ConvolveBlock"](len(f))
    ui = context.blocks["UIBlock"]()

    atime.set_input(0, cbt, 0)
    seq.set_input(0, atime, 0)
    seq.set_input(1, cbsong, 0)
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

    fb.set_input(0, wb2, 0)
    fb.set_input(1, filt, 0)
    ui.set_input(0, fb, 0)

    while True:
        try:
            #cb.cvalue.value += 10
            result = ui.pull()
            data=struct.pack('f'*context.chunk_size,*(ui.output[:context.chunk_size]))
            #print ui.output[:context.chunk_size]
            stream.write(data)
            if nk is not None:
                nk.process_input()
                cbt.cvalue.value = nk.state[nkm.SLIDERS[0]] / 30.
                cba.cvalue.value = nk.state[nkm.SLIDERS[1]] / 5.

        except KeyboardInterrupt:
            break
 
    stream.stop_stream()
    stream.close()
    thread.join(1.0)
