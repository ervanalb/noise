from ctypes import *
import os
#import grassroots as gr

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p
ERROR_T = c_int

SUCCESS=0

class NODE_T(Structure):
	pass

PULL_FN_PT = CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT))

NODE_T._fields_ = [
	('input_node',POINTER(POINTER(NODE_T))),
	('input_pull',POINTER(PULL_FN_PT)),
	('state',STATE_PT),
	]

NODE_PT = POINTER(NODE_T)

TYPE_INFO_PT = c_void_p
OUTPUT_ALLOC_FN_PT = CFUNCTYPE(ERROR_T,TYPE_INFO_PT, POINTER(OUTPUT_PT))
OUTPUT_FREE_FN_PT = CFUNCTYPE(ERROR_T,TYPE_INFO_PT, OUTPUT_PT)
OUTPUT_COPY_FN_PT = CFUNCTYPE(ERROR_T,TYPE_INFO_PT, OUTPUT_PT, OUTPUT_PT)

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

class MetaNoiseObject(type):
	def __str__(self):
		return self.string

	def __eq__(self,other):
		return self.string == other.string # LOL

class NoiseObject(object):
	__metaclass__ = MetaNoiseObject

	@classmethod
	def populate_object_info(cls,object_info):
		object_info.alloc_fn = OUTPUT_ALLOC_FN_PT(cls.alloc_fn)
		object_info.free_fn = OUTPUT_FREE_FN_PT(cls.free_fn)
		object_info.copy_fn = OUTPUT_COPY_FN_PT(cls.copy_fn)
		object_info.type_info = cast(pointer(cls.type_info),TYPE_INFO_PT)

	@classmethod
	def alloc(cls):
		out=OUTPUT_PT();
		e=cls.alloc_fn(byref(cls.type_info),byref(out))
		if e != SUCCESS:
			raise Exception("noise error")
		return out

	@classmethod
	def free(cls,ptr):
		cls.free_fn(byref(cls.type_info),ptr)

	def __init__(self):
		self.o=self.alloc()

	def __del__(self):
		self.free(self.o)

class TypeFactory(object):
	def __init__(self,alloc_fn,free_fn,copy_fn,typeinfo,string):
		self.alloc_fn=alloc_fn
		self.free_fn=free_fn
		self.copy_fn=copy_fn
		self.typeinfo=typeinfo
		self.string=string

	# This is actually really tricky and subtle, ignore it for now
	#def __eq__(self, other):
	#	return self.alloc_fn==other.alloc_fn and self.free_fn==other.free_fn and self.copy_fn==other.copy_fn and self.typeinfo==other.typeinfo
	# Unfortunate hack
	def __eq__(self, other):
		return self.string == other.string

	def __str__(self):
		return self.string

	def __call__(self,*args,**kwargs):
		return NoiseObject(*args,**kwargs)

	def populate_object_info(self,object_info):
		object_info.alloc_fn = OUTPUT_ALLOC_FN_PT(self.alloc_fn)
		object_info.free_fn = OUTPUT_FREE_FN_PT(self.free_fn)
		object_info.copy_fn = OUTPUT_COPY_FN_PT(self.copy_fn)
		object_info.type_info = cast(pointer(self.typeinfo),TYPE_INFO_PT)

	def alloc(self):
		out=OUTPUT_PT();
		e=self.alloc_fn(byref(self.typeinfo),byref(out))
		if e != SUCCESS:
			raise Exception("noise error")
		return out

	def free(self,ptr):
		self.free_fn(byref(self.typeinfo),ptr)

class Block(object):
    #input_blocks = gr.Field([])
    #block_type = gr.Field("Block")

    def __init__(self, *args, **kwargs):
        self.state_alloc = None
        self.state_free = None
        self.pull_fns = None # []
        self.num_inputs = 1
        self.num_outputs = 1
        self.setup()

    def setup(self):
        self.block_type = type(self).__name__
        # Allocate space for node
        self.node = NODE_T()
        self.node_ptr = pointer(self.node)

        # Allocate space for upstream pull fns
        self.input_pull_fns = (PULL_FN_PT * self.num_inputs)()
	
        self.node.input_pull = cast(self.input_pull_fns, POINTER(PULL_FN_PT))
        # Allocate space for upstream nodes
        self.input_nodes = (NODE_PT * self.num_inputs)()
        self.node.input_node = cast(self.input_nodes, POINTER(NODE_PT))

        self.input_blocks = [(None, None)] * self.num_inputs

        self.setup_state()

    def setup_state(self):
        self.state_alloc(BLOCK_INFO_PT(), byref(self.node, NODE_T.state.offset))

    def set_input(self, input_idx, block, output_idx):
        self.node.input_node[input_idx] = block.node_ptr
        self.node.input_pull[input_idx] = PULL_FN_PT(block.pull_fns[output_idx])
        self.input_blocks[input_idx] = (id(block), output_idx)

class OBJECT_STATE_T(Structure):
	_fields_=[
		('type_info',TYPE_INFO_PT),
		('copy_fn',OUTPUT_COPY_FN_PT),
		('object',OUTPUT_PT)
	]

class OBJECT_INFO_T(Structure):
	_fields_=[
		('type_info',TYPE_INFO_PT),
		('alloc_fn',OUTPUT_ALLOC_FN_PT),
		('copy_fn',OUTPUT_COPY_FN_PT),
		('free_fn',OUTPUT_FREE_FN_PT)
	]
