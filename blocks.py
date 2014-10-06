import struct
import cnoise as c

clib_noise = context.load_so('noise','noise.so')

c.c_int.in_dll(clib_noise, "global_chunk_size").value = context.chunk_size
c.c_int.in_dll(clib_noise, "global_frame_rate").value = context.frame_rate

class n_double(c.NoiseObject):
	alloc_fn = clib_noise.simple_alloc
	free_fn = clib_noise.simple_free
	copy_fn = clib_noise.simple_copy
	type_info = c.c_int(c.sizeof(c.c_double))
	string = 'double'

	@property
	def value(self):
		return c.cast(self.o,c.POINTER(c.c_double)).contents.value

	@value.setter
	def value(self,val):
		c.cast(self.o,c.POINTER(c.c_double)).contents.value=val

	def __init__(self,val):
		c.NoiseObject.__init__(self)
		self.o.contents.value=val

context.register_type('double',n_double)

class n_int(c.NoiseObject):
	alloc_fn = clib_noise.simple_alloc
	free_fn = clib_noise.simple_free
	copy_fn = clib_noise.simple_copy
	type_info = c.c_int(c.sizeof(c.c_int))
	string = 'int'

	@property
	def value(self):
		return c.cast(self.o,c.POINTER(c.c_int)).contents.value

	@value.setter
	def value(self,val):
		c.cast(self.o,c.POINTER(c.c_int)).contents.value=val

	def __init__(self,val):
		c.NoiseObject.__init__(self)
		self.value=val

context.register_type('int',n_int)

class n_chunk(c.NoiseObject):
	alloc_fn = clib_noise.simple_alloc
	free_fn = clib_noise.simple_free
	copy_fn = clib_noise.simple_copy
	type_info = c.c_int(c.sizeof(c.c_double)*context.chunk_size)
	string = 'chunk'

	def __init__(self):
		c.NoiseObject.__init__(self)

context.register_type('chunk',n_chunk)

class ARRAY_INFO_T(c.Structure):
	_fields_=[
		('length',c.c_int),
		('element',c.OBJECT_INFO_T)
	]

def array_factory(size,element_type):
	array_info=ARRAY_INFO_T()
	array_info.length=size
	element_type.populate_object_info(array_info.element)
	array_str = "{1}[{0}]".format(size,element_type.string)

	class n_array(c.NoiseObject):
		alloc_fn = clib_noise.array_alloc
		free_fn = clib_noise.array_free
		copy_fn = clib_noise.array_copy
		type_info = array_info
		string = array_str

		def __init__(self,lst):
			c.NoiseObject.__init__(self)
			for item in lst:
				if item is not None:
					element_type(item) # something like this

	return n_array

context.register_type('array',array_factory)

class WaveBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.wave_state_alloc
        self.state_free = clib_noise.wave_state_free
        self.pull_fns = [clib_noise.wave_pull]
        self.num_inputs = 2
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
    def __init__(self, value=None, ctype=None, cvalue=None):
        if cvalue is not None:
            self.cvalue = cvalue
        else:
            if ctype is None:
                if type(value) == int:
                    ctype = c.c_int32
                elif type(value) == float:
                    ctype = c.c_double
                else:
                    raise TypeError(value)
            self.cvalue = ctype(value)  
        self.block_info = c.cast(c.pointer(self.cvalue), c.BLOCK_INFO_PT)

        self.state_alloc = clib_noise.constant_state_alloc
        self.state_free = clib_noise.constant_state_free
        self.pull_fns = [clib_noise.constant_pull]
        self.num_inputs = 0
        self.num_outputs = 1
        self.setup()

    def setup_state(self):
        self.state_alloc(self.block_info, c.byref(self.node, c.NODE_T.state.offset))

context.register_block('ConstantBlock',ConstantBlock);

class PlusBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.maths_state_alloc
        self.state_free = clib_noise.maths_state_free
        self.pull_fns = [clib_noise.plus_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('PlusBlock', PlusBlock);

class MinusBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.maths_state_alloc
        self.state_free = clib_noise.maths_state_free
        self.pull_fns = [clib_noise.minus_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('MinusBlock', MinusBlock);

class MultiplyBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.maths_state_alloc
        self.state_free = clib_noise.maths_state_free
        self.pull_fns = [clib_noise.multiply_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('MultiplyBlock', MultiplyBlock);

class DivideBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.maths_state_alloc
        self.state_free = clib_noise.maths_state_free
        self.pull_fns = [clib_noise.divide_pull]
        self.num_inputs = 2
        self.num_outputs = 1
        self.setup()

context.register_block('DivideBlock', DivideBlock);

class NoteToFreqBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.maths_state_alloc
        self.state_free = clib_noise.maths_state_free
        self.pull_fns = [clib_noise.note_to_freq_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

context.register_block('NoteToFreqBlock', NoteToFreqBlock);

class FunctionGeneratorBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.function_gen_state_alloc
        self.state_free = clib_noise.function_gen_state_free
        self.pull_fns = [clib_noise.function_gen_pull]
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

context.register_block('FunctionGeneratorBlock', FunctionGeneratorBlock);

class SequencerBlock(c.Block):
    def __init__(self, *args, **kwargs):
        self.state_alloc = clib_noise.sequencer_state_alloc
        self.state_free = clib_noise.sequencer_state_free
        self.pull_fns = [clib_noise.sequencer_pull]
        self.num_inputs = 2 # time, seq
        self.num_outputs = 1
        self.setup()

context.register_block('SequencerBlock', SequencerBlock);

class UIBlock(c.Block):
    def __init__(self):
        # Super backwards block
        self.num_inputs = 1
        self.num_outputs = 1

        self.pull_fns = [clib_noise.constant_frequency]
        self.input_pull_fns = [None]
        self.input_nodes = [None]
        
        self.node = c.NODE_T()

        self.output = c.POINTER(c.c_double)()

    def set_input(self, input_idx, block, output_idx):
        print block.node
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](c.byref(self.input_nodes[0]), c.byref(self.output))

context.register_block('UIBlock', UIBlock);
