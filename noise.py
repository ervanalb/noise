import os
import pyaudio
import struct
import cnoise as c

"""
n=c.NODE_T()

ui_pulls=(c.PULL_FN_PT*1)()
ui_pulls[0].contents = c.clib_noise.constant_frequency

clib.wave_state_alloc(c.BLOCK_INFO_PT(), c.byref(n,c.NODE_T.state.offset))

n.input_pull = ui_pulls[0]
#n.input_node = 0
"""


class Block(object):
    def __init__(self, *args, **kwargs):
        self.state_alloc = c.clib_noise.wave_state_alloc
        self.state_free = c.clib_noise.wave_state_free
        self.pull_fns = [c.clib_noise.wave_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

    def setup(self):
        # Allocate space for node
        self.node = c.NODE_T()
        # Allocate space for upstream pull fns
        self.input_pull_fns = (c.PULL_FN_PT * self.num_inputs)()
        self.node.input_pull = self.input_pull_fns[0]
        # Allocate space for upstream nodes
        self.input_nodes = (c.NODE_PT * self.num_inputs)()
        self.node.input_node = self.input_nodes[0]

        self.state_alloc(c.BLOCK_INFO_PT(), c.byref(self.node, c.NODE_T.state.offset))

    def set_input(self, block, input_idx, output_idx):
        self.input_nodes[input_idx].contents = block.node
        print block.pull_fns
        self.input_pull_fns[input_idx].contents = block.pull_fns[output_idx]


class UIBlock(Block):
    def __init__(self):
        # Super backwards block
        self.num_inputs = 1
        self.num_outputs = 1

        self.pull_fns = [c.clib_noise.constant_frequency]
        self.input_pull_fns = [None]
        self.input_nodes = [None]
        
        self.node = c.NODE_PT()

        self.output = c.POINTER(c.c_double)()

    def set_input(self, block, input_idx, output_idx):
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](c.byref(self.input_nodes[0]), c.byref(self.output))
    
wb = Block()
ui = UIBlock()
wb.set_input(ui, 0, 0)
ui.set_input(wb, 0, 0)

while True:
	result = ui.pull()
	data=struct.pack('f'*CHUNKSIZE,*(ui.output[:CHUNKSIZE]))
	stream.write(data)

stream.stop_stream()
stream.close()

p.terminate()

