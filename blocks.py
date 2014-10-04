import struct
import cnoise as c

context.load_so('noise','noise.so')
clib_noise = context.libs['noise']

n_double=c.TypeFactory(clib_noise.simple_alloc,clib_noise.simple_free,clib_noise.simple_copy,c.c_int(c.sizeof(c.c_double)),'double')
context.register_type('double',n_double)

n_int=c.TypeFactory(clib_noise.simple_alloc,clib_noise.simple_free,clib_noise.simple_copy,c.c_int(c.sizeof(c.c_int)),'int')
context.register_type('int',n_int)

#n_chunk=c.TypeFactory(clib_noise.simple_alloc,clib_noise.simple_free,clib_noise.simple_copy,c.c_int(c.sizeof(c.c_double)*context.global_vars[0]),'chunk')
#context.register_type('chunk',n_chunk)


class WaveBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.wave_state_alloc
        self.state_free = clib_noise.wave_state_free
        self.pull_fns = [clib_noise.wave_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

context.register_block('WaveBlock',WaveBlock);

class LPFBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.lpf_state_alloc
        self.state_free = clib_noise.lpf_state_free
        self.pull_fns = [clib_noise.lpf_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('LPFBlock',LPFBlock);

class AccumulatorBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.accumulator_state_alloc
        self.state_free = clib_noise.accumulator_state_free
        self.pull_fns = [clib_noise.accumulator_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

context.register_block('AccumulatorBlock',AccumulatorBlock);

class UnionBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.union_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

context.register_block('UnionBlock',UnionBlock);

class TeeBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.union_pull, clib_noise.tee_pull_aux]
        self.num_inputs = 1
        self.num_outputs = 2
        self.setup()

context.register_block('TeeBlock',TeeBlock);

class WyeBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.union_state_alloc
        self.state_free = clib_noise.union_state_free
        self.pull_fns = [clib_noise.wye_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('WyeBlock',WyeBlock);

class ConstantBlock(c.Block):
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

context.register_block('ConstantBlock',ConstantBlock);

class UIBlock(c.Block):
    def __init__(self):
        # Super backwards block
        self.num_inputs = 1
        self.num_outputs = 1

        self.pull_fns = [clib_noise.constant_frequency]
        self.input_pull_fns = [None]
        self.input_nodes = [None]
        
        self.node_ptr = c.NODE_PT()

        self.output = c.POINTER(c.c_double)()

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](c.byref(self.input_nodes[0]), c.byref(self.output))

context.register_block('UIBlock', UIBlock);