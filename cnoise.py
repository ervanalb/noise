from ctypes import *
from ctypes import _CFuncPtr
import os

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p
ERROR_T = c_int

SUCCESS=0

class NODE_T(Structure):
    pass

PULL_FN_PT = CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT))
STATE_ALLOC_FN_PT = CFUNCTYPE(ERROR_T,BLOCK_INFO_PT, POINTER(STATE_PT))
STATE_FREE_FN_PT = CFUNCTYPE(ERROR_T,BLOCK_INFO_PT, STATE_PT)

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
        self.types=[]

    def load(self,py_file):
        vars_dict={'context':self}
        execfile(py_file,vars_dict)

    def load_so(self,name,soname):
        l=cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),soname))
        self.libs[name]=l
        return l

    def resolve_type(self, type_or_name):
        if isinstance(type_or_name, str) or isinstance(type_or_name, unicode):
            return self.types[type_or_name]
        return type_or_name

    def register_type(self,typefn):
        self.types.append(typefn)

    def register_block(self,blockname,blockfn):
        self.blocks[blockname]=blockfn

    def set_global_vars(self,lib):
        for (t,k,v) in self.global_vars:
            t.in_dll(lib,k).value=v

    def get_type(self,string):
        for t in self.types:
            ti=t(string,self.get_type)
            if ti is not None:
                return ti
        raise TypeError(string)

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
        object_info.alloc_fn = cast(cls.alloc_fn,OUTPUT_ALLOC_FN_PT)
        object_info.free_fn = cast(cls.free_fn,OUTPUT_FREE_FN_PT)
        object_info.copy_fn = cast(cls.copy_fn,OUTPUT_COPY_FN_PT)
        object_info.type_info = cast(pointer(cls.type_info),TYPE_INFO_PT)

    @classmethod
    def alloc(cls):
        out=OUTPUT_PT()
        e=cls.alloc_fn(byref(cls.type_info),byref(out))
        if e != SUCCESS:
            raise Exception("noise error")
        return out

    # Override me for more complicated behaviour!
    @classmethod
    def fromstring(cls,string,lkup):
        if string == cls.string:
            return cls
        return None

    @classmethod
    def new(cls,val=None):
        instance=cls(cls.alloc())
        def del_method(self):
            cls.free(self.o)
        instance.__del__=del_method
        if val is not None:
            instance.value=val
        return instance

    @classmethod
    def free(cls,ptr):
        cls.free_fn(byref(cls.type_info),ptr)

    @classmethod
    def deref(cls,ptr):
        if ptr is None: return None
        return cls(ptr).value

    def __init__(self, pointer):
        self.o=pointer

class Block(object):
    num_inputs=0
    num_outputs=0
    input_names = []
    output_names = []
    block_info=BLOCK_INFO_PT()
    pull_fns=[]
    state_alloc=None
    state_free=None

    def __init__(self):
        self.block_type = type(self).__name__

        # Allocate space for node
        self.node = NODE_T()
        self.node_ptr = pointer(self.node)

        # Allocate space for upstream pull fns
        self.input_pull_fns = (PULL_FN_PT * self.num_inputs)() # TODO Populate this with null pointers
    
        self.node.input_pull = cast(self.input_pull_fns, POINTER(PULL_FN_PT))
        # Allocate space for upstream nodes
        self.input_nodes = (NODE_PT * self.num_inputs)()
        self.node.input_node = cast(self.input_nodes, POINTER(NODE_PT))

        self.input_blocks = [(None, None)] * self.num_inputs

        self.alloc()

    def alloc(self):
        if self.state_alloc:
            self.state_alloc(self.block_info, cast(byref(self.node, NODE_T.state.offset),POINTER(STATE_PT)))

    def free(self):
        if self.state_free:
            self.state_free(self.block_info, self.node.state)

    def __del__(self):
        self.free()

    def set_input(self, input_idx, block, output_idx):
        self.node.input_node[input_idx] = block.node_ptr
        self.node.input_pull[input_idx] = cast(block.pull_fns[output_idx],PULL_FN_PT)
        self.input_blocks[input_idx] = (id(block), output_idx)

    def __str__(self):
        return self.block_type

    def input_pull(self,n,otype=None):
        output_p = OUTPUT_PT()
        e=self.input_pull_fns[n](self.input_nodes[n], byref(output_p))
        if e:
            raise Exception("noise error {0}".format(e))
        if not otype:
            return output_p
        if not output_p:
            return None
        return cast(output_p,POINTER(otype)).contents

    def output_pull(self,n,otype=None):
        output_p = OUTPUT_PT()
        e=self.pull_fns[n](self.node_ptr, byref(output_p))
        if e:
            raise Exception("noise error {0}".format(e))
        if not otype:
            return output_p
        if not output_p:
            return None
        return cast(output_p,POINTER(otype)).contents

    # decorator
    @staticmethod
    def pull_fn(fn):
        def _pull_fn(self,node_p,output_pp):
            try:
                self.output_value=fn(self)
            except Exception:
                # TODO make this nicer
                print "exception!"
                raise
                return 1
            if self.output_value is None:
                output_pp[0]=OUTPUT_PT()
            else:
                output_pp[0]=cast(pointer(self.output_value),OUTPUT_PT)
            return 0
        return _pull_fn

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
