import cnoise
import ctypes
import json
import ntype
import pyaudio
import struct
import sys

context=cnoise.NoiseContext()
context.chunk_size = 1024
context.frame_rate = 48000

context.load('blocks.py')

def play_json_netlist(json_filename):
    with open(json_filename) as f:
        data = json.load(f)

    blocks = {}

    for block_name, block_data in data["blocks"].items():
        params = block_data["params"]
        if params is None: params = {}
        blocks[block_name] = context.blocks[block_data["class"]](**params)

    for block_name, block_data in data["blocks"].items():
        block = blocks[block_name]
        for inp in block_data["inputs"]:
            block.set_input(inp["input_port"], blocks[inp["output_block"]], inp["output_port"])

    return blocks

if __name__ == "__main__":

    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=context.frame_rate,
        frames_per_buffer=context.chunk_size,
        output=True)
    blocks = play_json_netlist("netlist/output.json")
 
    output_block = blocks["AUDIO"]
    
    """
    n_double=context.types['double']
    n_int=context.types['int']

    c440 = context.blocks["ConstantBlock"](n_double.new((440.0)))
    c1 = context.blocks["ConstantBlock"](n_int.new(1))
    wv = context.blocks["WaveBlock"]()
    ui=context.blocks["UIBlock"]()
    wv.set_input(0, c440, 0)
    wv.set_input(1, c1, 0)
    ui.set_input(0, wv, 0)

    output_block = ui
    """

    while True:
        try:
            #cb.cvalue.value += 10
            result=output_block.pull()
            chunk=output_block.output[:context.chunk_size]
            data=struct.pack('f'*context.chunk_size,*chunk)
            #print ui.output[:context.chunk_size]
            stream.write(data)

        except KeyboardInterrupt:
            break
    stream.stop_stream()
    stream.close()
