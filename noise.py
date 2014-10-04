import os
import pyaudio
import struct
import cnoise as c

n=c.NODE_T()

ui_pulls=(c.PULL_FN_PT*1)()
ui_pulls[0].contents = c.clib_noise.constant_frequency

clib.wave_state_alloc(c.BLOCK_INFO_PT(), c.byref(n,NODE_T.state.offset))

n.input_pull = ui_pulls[0]
#n.input_node = 0

output = c.POINTER(c_double)()


class Block(object):
    def __init__(self, *args, **kwargs):
        self.state_alloc = None
        self.state_free = None
        self.pull_fns = []
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

    def setup(self):
        self.node = c.NODE_T()
        self.input_pull_fns = (c.PULL_FN_PT * self.num_inputs)()
        self.input_nodes = (c.NODE_PT * self.num_inputs)()
        self.state_alloc(

    def set_input(self, input_idx, block):
        s



while True:
	result=clib.wave_pull(byref(n),byref(output))
	data=struct.pack('f'*CHUNKSIZE,*(output[:CHUNKSIZE]))
	stream.write(data)

stream.stop_stream()
stream.close()

p.terminate()

