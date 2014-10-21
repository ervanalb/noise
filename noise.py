import cnoise
import ctypes
import ntype
import pyaudio
import struct

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000

context.load('blocks.py')

if __name__ == "__main__":
    heap=[]

    def instrument(notes,tb,tbout,wave_shape):
        # Build melody
        global heap
        melody_array = context.types['array'](len(notes),n_double).new(notes)
        melody_notes = context.blocks["ConstantBlock"](melody_array)
        melody_seq = context.blocks["SequencerBlock"](melody_array) # could also call this with a type
        melody_seq.set_input(0,tb,tbout)
        melody_seq.set_input(1,melody_notes,0)
        melody_freqs = context.blocks["NoteToFreqBlock"]()
        melody_freqs.set_input(0,melody_seq,0)
        melody_wave_shape = context.blocks["ConstantBlock"](n_int.new(wave_shape))
        melody_voice = context.blocks["WaveBlock"]()
        melody_voice.set_input(0,melody_freqs,0)
        melody_voice.set_input(1,melody_wave_shape,0)
        heap.append(melody_notes)
        heap.append(melody_seq)
        heap.append(melody_freqs)
        heap.append(melody_wave_shape)
        return melody_voice

    unison =         [65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77, None, None, None]
    unison_harmony = [61, 61, None, 61, 63, 63, 63, None, 63, 58, 58, 58, 65, 65, None, None, 65, 65, None, 65, 63, 63, 63, 63, 63, 68, None, 68, 61, None, None, None]

    n_double=context.types['double']

    n_int=context.types['int']
    n_wave=context.types['wave']

    # Build a timebase
    dt = context.blocks["ConstantBlock"](n_double.new(0.01))
    timebase = context.blocks["AccumulatorBlock"]()
    timebase.set_input(0,dt,0)

    timebase_splitter=context.blocks["TeeBlock"](n_double)
    timebase_splitter.set_input(0,timebase,0)

    def down_octave(n):
        if n is None:
            return None
        return n-12
    mel=instrument(unison,timebase_splitter,0,1)
    cm=instrument(map(down_octave,unison_harmony),timebase_splitter,1,2)

    mixer=context.blocks["MixerBlock"](2)
    mel_vol = context.blocks["ConstantBlock"](n_double.new(0.7))
    cm_vol = context.blocks["ConstantBlock"](n_double.new(0.5))
    mixer.set_input(0,mel,0)
    mixer.set_input(1,mel_vol,0)
    mixer.set_input(2,cm,0)
    mixer.set_input(3,cm_vol,0)

    ui=context.blocks["UIBlock"]()
    ui.set_input(0,mixer,0)


    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=context.frame_rate,
        frames_per_buffer=context.chunk_size,
        output=True)
 
    while True:
        try:
            #cb.cvalue.value += 10
            result=ui.pull()
            chunk=ui.output[:context.chunk_size]
            data=struct.pack('f'*context.chunk_size,*chunk)
            #print ui.output[:context.chunk_size]
            stream.write(data)

        except KeyboardInterrupt:
            break
    stream.stop_stream()
    stream.close()
