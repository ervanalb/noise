import struct
import cnoise as c

clib_noise = context.load_so('noise','noise.so')

c.c_int.in_dll(clib_noise, "global_chunk_size").value = context.chunk_size
c.c_int.in_dll(clib_noise, "global_frame_rate").value = context.frame_rate


class n_double(c.NoiseObject):
    alloc_fn = c.OUTPUT_ALLOC_FN_PT(("simple_alloc",clib_noise))
    free_fn = c.OUTPUT_FREE_FN_PT(("simple_free", clib_noise))
    copy_fn = c.OUTPUT_COPY_FN_PT(("simple_copy", clib_noise))
    type_info = c.c_int(c.sizeof(c.c_double))
    string = 'double'

    @property
    def value(self):
        return c.cast(self.o,c.POINTER(c.c_double)).contents.value

    @value.setter
    def value(self,val):
        c.cast(self.o,c.POINTER(c.c_double)).contents.value=val

context.register_type('double',n_double)

class n_int(c.NoiseObject):
    alloc_fn = c.OUTPUT_ALLOC_FN_PT(("simple_alloc", clib_noise))
    free_fn = c.OUTPUT_FREE_FN_PT(("simple_free", clib_noise))
    copy_fn = c.OUTPUT_COPY_FN_PT(("simple_copy", clib_noise))
    type_info = c.c_int(c.sizeof(c.c_int))
    string = 'int'

    @property
    def value(self):
        return c.cast(self.o,c.POINTER(c.c_int)).contents.value

    @value.setter
    def value(self,val):
        c.cast(self.o,c.POINTER(c.c_int)).contents.value=val

context.register_type('int',n_int)

class n_chunk(c.NoiseObject):
    alloc_fn = c.OUTPUT_ALLOC_FN_PT(("simple_alloc", clib_noise))
    free_fn = c.OUTPUT_FREE_FN_PT(("simple_free", clib_noise))
    copy_fn = c.OUTPUT_COPY_FN_PT(("simple_copy", clib_noise))
    type_info = c.c_int(c.sizeof(c.c_double)*context.chunk_size)
    string = 'chunk'

    @property
    def value(self):
        arr=c.cast(self.o,c.POINTER(c.c_double*context.chunk_size)).contents
        return [e for e in arr]

    @value.setter
    def value(self,val):
        array=c.cast(self.o,c.POINTER(c.c_double*context.chunk_size)).contents
        for i in range(context.chunk_size):
            array[i]=val[i]


context.register_type('chunk',n_chunk)

def wave_factory(length):
    class n_wave(c.NoiseObject):
        alloc_fn = c.OUTPUT_ALLOC_FN_PT(("simple_alloc", clib_noise))
        free_fn = c.OUTPUT_FREE_FN_PT(("simple_free", clib_noise))
        copy_fn = c.OUTPUT_COPY_FN_PT(("simple_copy", clib_noise))
        type_info = c.c_int(c.sizeof(c.c_double)*length)
        string='wave'

        @property
        def value(self):
            arr=c.cast(self.o,c.POINTER(c.c_double*length)).contents
            return [e for e in arr]

        @value.setter
        def value(self,val):
            array=c.cast(self.o,c.POINTER(c.c_double*length)).contents
            for i in range(length):
                array[i]=val[i]

    return n_wave

context.register_type('wave',wave_factory)

class ARRAY_INFO_T(c.Structure):
    _fields_=[
        ('length',c.c_int),
        ('element',c.OBJECT_INFO_T)
    ]

OUTPUT_ARRAY_T = c.OUTPUT_PT

def array_factory(size,element_type):
    array_info=ARRAY_INFO_T()
    array_info.length=size
    element_type.populate_object_info(array_info.element)
    array_str = "{1}[{0}]".format(size,element_type.string)

    class n_array(c.NoiseObject):
        alloc_fn = c.OUTPUT_ALLOC_FN_PT(("array_alloc", clib_noise))
        free_fn = c.OUTPUT_FREE_FN_PT(("array_free", clib_noise))
        copy_fn = c.OUTPUT_COPY_FN_PT(("array_copy", clib_noise))

        type_info = array_info
        string = array_str

        @property
        def value(self):
            arr=c.cast(self.o,c.POINTER(OUTPUT_ARRAY_T*size)).contents
            return [element_type.deref(arr[i]) for i in range(size)]

        @value.setter
        def value(self,val):
            array=c.cast(self.o,c.POINTER(OUTPUT_ARRAY_T*size)).contents
            for i in range(size):
                if array[i] is not None:
                    element_type.free(array[i])
                if val[i] is not None:
                    array[i]=element_type.alloc()
                    element_type(array[i]).value=val[i]
                else:
                    array[i]=c.OUTPUT_PT()

    return n_array

context.register_type('array',array_factory)

class WaveBlock(c.Block):
    state_alloc = clib_noise.wave_state_alloc
    state_free = clib_noise.wave_state_free
    pull_fns = [c.PULL_FN_PT(("wave_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["freq", "type"]
    output_names = ["wave"]

context.register_block('WaveBlock',WaveBlock)

class LPFBlock(c.Block):
    state_alloc = clib_noise.lpf_state_alloc
    state_free = clib_noise.lpf_state_free
    pull_fns = [c.PULL_FN_PT(("lpf_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["wave", "alpha"]
    output_names = ["wave"]

context.register_block('LPFBlock',LPFBlock)

class AccumulatorBlock(c.Block):
    state_alloc = clib_noise.accumulator_state_alloc
    state_free = clib_noise.accumulator_state_free
    pull_fns = [c.PULL_FN_PT(("accumulator_pull", clib_noise))]
    num_inputs = 1
    num_outputs = 1
    input_names = ["x"]
    output_names = ["Sum[x]"]

context.register_block('AccumulatorBlock',AccumulatorBlock)

class UnionBlock(c.Block):
    state_alloc = clib_noise.union_state_alloc
    state_free = clib_noise.union_state_free
    pull_fns = [c.PULL_FN_PT(("union_pull", clib_noise))]
    num_inputs = 1
    num_outputs = 1

    def __init__(self, datatype):
        info = c.OBJECT_INFO_T()
        datatype.populate_object_info(info)
        self.block_info = c.cast(c.pointer(info),c.BLOCK_INFO_PT)
        c.Block.__init__(self)

context.register_block('UnionBlock',UnionBlock)

class TeeBlock(c.Block):
    state_alloc = clib_noise.union_state_alloc
    state_free = clib_noise.union_state_free
    pull_fns = [c.PULL_FN_PT(("union_pull", clib_noise)), c.PULL_FN_PT(("tee_pull_aux", clib_noise))]
    num_inputs = 1
    num_outputs = 2
    input_names = ["in"]
    output_names = ["left", "right"]

    def __init__(self, datatype):
        info = c.OBJECT_INFO_T()
        datatype.populate_object_info(info)
        self.block_info = c.cast(c.pointer(info),c.BLOCK_INFO_PT)
        c.Block.__init__(self)

context.register_block('TeeBlock',TeeBlock)

class WyeBlock(c.Block):
    state_alloc = clib_noise.union_state_alloc
    state_free = clib_noise.union_state_free
    pull_fns = [c.PULL_FN_PT(("wye_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["left", "right"]
    output_names = ["out"]

    def __init__(self, datatype):
        info = c.OBJECT_INFO_T()
        datatype.populate_object_info(info)
        self.block_info = c.cast(c.pointer(info),c.BLOCK_INFO_PT)
        c.Block.__init__(self)

context.register_block('WyeBlock',WyeBlock)

class ConstantBlock(c.Block):
    state_alloc = clib_noise.constant_state_alloc
    state_free = clib_noise.constant_state_free
    pull_fns = [c.PULL_FN_PT(("constant_pull", clib_noise))]
    num_outputs = 1 
    input_names = []
    output_names = ["Const"]


    def __init__(self,noise_obj=None):
        c.Block.__init__(self)
        if noise_obj is not None:
            self.noise_obj = noise_obj
            self.pointer=noise_obj.o

    @property
    def pointer(self):
        return c.cast(self.node.state,c.OUTPUT_PT)

    @pointer.setter
    def pointer(self,ptr):
        self.node.state=c.cast(ptr,c.BLOCK_INFO_PT)

    @property
    def data(self):
        return self.noise_obj.value

    @data.setter
    def data(self, val):
        self.noise_obj.value = val

context.register_block('ConstantBlock',ConstantBlock)

class PlusBlock(c.Block):
    state_alloc = clib_noise.maths_state_alloc
    state_free = clib_noise.maths_state_free
    pull_fns = [c.PULL_FN_PT(("plus_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["double", "double"]
    output_names = ["double"]

context.register_block('PlusBlock', PlusBlock)

class MinusBlock(c.Block):
    state_alloc = clib_noise.maths_state_alloc
    state_free = clib_noise.maths_state_free
    pull_fns = [c.PULL_FN_PT(("minus_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["+ double", "- double"]
    output_names = ["double"]

context.register_block('MinusBlock', MinusBlock)

class MultiplyBlock(c.Block):
    state_alloc = clib_noise.maths_state_alloc
    state_free = clib_noise.maths_state_free
    pull_fns = [c.PULL_FN_PT(("multiply_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["double", "double"]
    output_names = ["double"]

context.register_block('MultiplyBlock', MultiplyBlock)

class DivideBlock(c.Block):
    state_alloc = clib_noise.maths_state_alloc
    state_free = clib_noise.maths_state_free
    pull_fns = [c.PULL_FN_PT(("divide_pull", clib_noise))]
    num_inputs = 2
    num_outputs = 1
    input_names = ["N double", "D double"]
    output_names = ["double"]

context.register_block('DivideBlock', DivideBlock)

class NoteToFreqBlock(c.Block):
    state_alloc = clib_noise.maths_state_alloc
    state_free = clib_noise.maths_state_free
    pull_fns = [c.PULL_FN_PT(("note_to_freq_pull", clib_noise))]
    num_inputs = 1
    num_outputs = 1
    input_names = ["MIDI Note (int)"]
    output_names = ["Freq"]

context.register_block('NoteToFreqBlock', NoteToFreqBlock)

class FunctionGeneratorBlock(c.Block):
    state_alloc = clib_noise.function_gen_state_alloc
    state_free = clib_noise.function_gen_state_free
    pull_fns = [c.PULL_FN_PT(("function_gen_pull", clib_noise))]
    num_inputs = 1
    num_outputs = 1
    input_names = ["Frequency"]
    output_names = ["Chunks"]

context.register_block('FunctionGeneratorBlock', FunctionGeneratorBlock)

class SEQUENCER_INFO_T(c.Structure):
    _fields_=[('array_info',c.POINTER(ARRAY_INFO_T))]

class SequencerBlock(c.Block):
    state_alloc = clib_noise.sequencer_state_alloc
    state_free = clib_noise.sequencer_state_free
    pull_fns = [c.PULL_FN_PT(("sequencer_pull", clib_noise))]
    num_inputs = 2 # time, seq
    num_outputs = 1
    input_names = ["Time", "Sequence"]
    output_names = ["Seq Item"]

    def __init__(self,array_type):
        seq_info = SEQUENCER_INFO_T()
        seq_info.array_info = c.cast(c.pointer(array_type.type_info),c.POINTER(ARRAY_INFO_T))
        self.block_info = c.cast(c.pointer(seq_info),c.BLOCK_INFO_PT)
        c.Block.__init__(self)

context.register_block('SequencerBlock', SequencerBlock)

class ConvolveBlock(c.Block):
    state_alloc = clib_noise.convolve_state_alloc
    state_free = clib_noise.convolve_state_free
    pull_fns = [c.PULL_FN_PT(("convolve_pull", clib_noise))]
    num_inputs = 2 # chunks, wave
    num_outputs = 1 # chunks
    input_names = ["Chunks In", "Wave In"]
    output_names = ["Chunks Out"]

    def __init__(self,length):
        self.block_info = c.cast(c.pointer(c.c_int(length)),c.BLOCK_INFO_PT)
        c.Block.__init__(self)

context.register_block('ConvolveBlock', ConvolveBlock)

class UIBlock(c.Block):
    num_inputs = 1
    num_outputs = 0
    input_names = ["Audio In"]
    output_names = []
    def __init__(self):
        # Super backwards block

        self.pull_fns = [clib_noise.constant_frequency]
        self.input_pull_fns = [None]
        self.input_nodes = [None]
        
        self.node = c.NODE_T()

        self.output = c.POINTER(c.c_double)()

    def alloc(self):
        pass
    def free(self):
        pass

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx] = block.node
        self.input_pull_fns[input_idx] = block.pull_fns[output_idx]

    def pull(self):
        return self.input_pull_fns[0](self.input_nodes[0], c.cast(c.byref(self.output),c.POINTER(c.OUTPUT_PT)))

context.register_block('UIBlock', UIBlock)
