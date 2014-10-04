import struct
import cnoise as c

clib_noise = c.noise_lib

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

        self.state_alloc()

    def state_alloc(self):
        self.state_alloc(c.BLOCK_INFO_PT(), c.byref(self.node, c.NODE_T.state.offset))

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx].contents = block.node
        print block.pull_fns
        self.input_pull_fns[input_idx].contents = block.pull_fns[output_idx]
        self.node.input_pull = self.input_pull_fns[0]

class WaveBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.wave_state_alloc
        self.state_free = clib_noise.wave_state_free
        self.pull_fns = [clib_noise.wave_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

class LPFBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.lpf_state_alloc
        self.state_free = clib_noise.lpf_state_free
        self.pull_fns = [clib_noise.lpf_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

class AccumulatorBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.accumulator_state_alloc
        self.state_free = clib_noise.accumulator_state_free
        self.pull_fns = [clib_noise.accumulator_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

class UnionBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.union_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

class TeeBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.union_pull, clib_noise.tee_pull_aux]
        self.num_inputs = 1
        self.num_outputs = 2
        self.setup()

class WyeBlock(Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.wye_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

class ConstantBlock(Block):
    def __init__(self, value, ctype=None):
        if ctype is None:
            if type(value) == int:
                ctype = c.c_int32
            elif type(value) == float:
                ctype = c.c_double
            else:
                raise TypeError(value)
        self.cvalue = c.POINTER(ctype)(ctype(value))
        self.block_info = c.cast(self.cvalue, c.BLOCK_INFO_PT)

        self.state_alloc = clib_noise.constant_state_alloc
        self.state_free = clib_noise.constant_state_free
        self.pull_fns = [clib_noise.constant_pull]
        self.num_inputs = 0
        self.num_outputs = 1
        self.setup()

    def state_alloc(self):
        self.state_alloc(self.block_info, c.byref(self.node, c.NODE_T.state.offset))

class UIBlock(Block):
    def __init__(self):
        # Super backwards block
        self.num_inputs = 1
        self.num_outputs = 1

        self.pull_fns = [clib_noise.constant_frequency]
        self.input_pull_fns = [None]
        self.input_nodes = [None]
        
        self.node = c.NODE_PT()

        self.output = c.POINTER(c.c_double)()

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](c.byref(self.input_nodes[0]), c.byref(self.output))
    
