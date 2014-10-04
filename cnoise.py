from ctypes import *
import os

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p

class NODE_T(Structure):
	pass

PULL_FN_PT = CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT))

NODE_T._fields_ = [
	('input_node',POINTER(POINTER(NODE_T))),
	('input_pull',POINTER(PULL_FN_PT)),
	('state',STATE_PT),
	]

NODE_PT = POINTER(NODE_T)

class SEQUENCE_T(Structure):
    _fields_ = [
        ('length', c_int),
        ('array', POINTER(POINTER(c_double))),
    ]

class NoiseContext(object):
	def __init__(self):
		self.libs={}
		self.blocks={}
		self.types={}

	def load(self,py_file):
		vars_dict={'context':self}
		execfile(py_file,vars_dict)

	def load_so(self,name,soname):
		l=cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),soname))
		self.libs[name]=l
		return l

	def register_type(self,typename,typefn):
		self.types[typename]=typefn

	def register_block(self,blockname,blockfn):
		self.blocks[blockname]=blockfn

	def set_global_vars(self,lib):
		for (t,k,v) in self.global_vars:
			t.in_dll(lib,k).value=v

	def __getitem__(self,index):
		return self.libs[index]

class TypeFactory(object):
	def __init__(self,alloc,free,copy,typeinfo,string):
		self.alloc=alloc
		self.free=free
		self.copy=copy
		self.typeinfo=typeinfo
		self.string=string

	def alloc(self):
		self.alloc_fn

	def __eq__(self, other):
		return self.alloc==other.alloc and self.free==other.free and self.copy==other.copy and self.typeinfo==other.typeinfo

	def __str__(self):
		return self.string

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
        self.node = NODE_T()
        self.node_ptr = pointer(self.node)

        # Allocate space for upstream pull fns
        self.input_pull_fns = (PULL_FN_PT * self.num_inputs)()
	
        self.node.input_pull = cast(self.input_pull_fns, POINTER(PULL_FN_PT))
        # Allocate space for upstream nodes
        self.input_nodes = (NODE_PT * self.num_inputs)()
        self.node.input_node = cast(self.input_nodes, POINTER(NODE_PT))

        self.setup_state()

    def setup_state(self):
        self.state_alloc(BLOCK_INFO_PT(), byref(self.node, NODE_T.state.offset))

    def set_input(self, input_idx, block, output_idx):
        self.node.input_node[input_idx] = block.node_ptr
        self.node.input_pull[input_idx] = PULL_FN_PT(block.pull_fns[output_idx])


#CHUNKSIZE = 128
#FRAMERATE = 48000

#c_int.in_dll(clib_noise, "global_chunk_size").value = CHUNKSIZE
#c_int.in_dll(clib_noisee, "global_frame_rate").value = FRAMERATE

