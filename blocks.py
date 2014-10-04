import struct
import cnoise as c


class Block(object):
    def __init__(self, *args, **kwargs):
        self.state_alloc = None
        self.state_free = None
        self.pull_fns = None # []
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

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx].contents = block.node
        print block.pull_fns
        self.input_pull_fns[input_idx].contents = block.pull_fns[output_idx]
        self.node.input_pull = self.input_pull_fns[0]

class WaveBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = c.clib_noise.wave_state_alloc
        self.state_free = c.clib_noise.wave_state_free
        self.pull_fns = [c.clib_noise.wave_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

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

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](c.byref(self.input_nodes[0]), c.byref(self.output))
    
