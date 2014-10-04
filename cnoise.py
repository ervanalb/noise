from ctypes import *
import os

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p

class NODE_T(Structure):
	pass

PULL_FN_PT = POINTER(CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT)))

NODE_T._fields_ = [
	('input_node',POINTER(NODE_T)),
	('input_pull',PULL_FN_PT),
	('state',STATE_PT),
	]

NODE_PT = POINTER(NODE_T)

class NoiseContext(object):
	def __init__(self,global_vars):
		self.libs={}
		self.global_vars=global_vars
		self.blocks={}
		self.types={}

	def load(self,py_file):
		vars_dict={'context':self}
		execfile(py_file,vars_dict)

	def load_so(self,name,soname):
		if name not in self.libs:
			self.libs[name]=cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),soname))
			self.set_global_vars(self.libs[name])

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
        self.node.input_pull = self.input_pull_fns[0]
        # Allocate space for upstream nodes
        self.input_nodes = (NODE_PT * self.num_inputs)()
        self.node.input_node = self.input_nodes[0]

        self.setup_state()

    def setup_state(self):
        self.state_alloc(BLOCK_INFO_PT(), byref(self.node, NODE_T.state.offset))

    def set_input(self, input_idx, block, output_idx):
        self.input_nodes[input_idx].contents = block.node_ptr
        print block.node_ptr, block.pull_fns
        self.input_pull_fns[input_idx].contents = block.pull_fns[output_idx]
        self.node.input_pull = self.input_pull_fns[0]


#CHUNKSIZE = 128
#FRAMERATE = 48000

#c_int.in_dll(clib_noise, "global_chunk_size").value = CHUNKSIZE
#c_int.in_dll(clib_noisee, "global_frame_rate").value = FRAMERATE

