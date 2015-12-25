from ctypes import *
import os
import importlib

nz = cdll.LoadLibrary("libnoise.so")

nz.nz_error_rc_str.restype = c_char_p

SUCCESS = 0

NZRC = c_int
CONTEXT_POINTER = c_void_p
BLOCK_STATE_POINTER = c_void_p
TYPE_POINTER = c_void_p
OBJ_POINTER = c_void_p
PULL_FN_P = c_void_p
LIB_HANDLE = c_void_p

class TYPECLASS(Structure):
    _fields_ = [('type_id', c_char_p),
                ('type_create', CFUNCTYPE(NZRC, CONTEXT_POINTER, POINTER(TYPE_POINTER), c_char_p)),
                ('type_destroy', CFUNCTYPE(None, TYPE_POINTER)),
                ('type_is_equal', CFUNCTYPE(c_int, TYPE_POINTER, TYPE_POINTER)),
                ('type_str', CFUNCTYPE(NZRC, TYPE_POINTER, POINTER(POINTER(c_char)))),
                ('type_create_obj', CFUNCTYPE(NZRC, TYPE_POINTER, POINTER(OBJ_POINTER))),
                ('type_init_obj', CFUNCTYPE(NZRC, TYPE_POINTER, OBJ_POINTER, c_char_p)),
                ('type_copy_obj', CFUNCTYPE(NZRC, TYPE_POINTER, OBJ_POINTER, OBJ_POINTER)),
                ('type_destroy_obj', CFUNCTYPE(NZRC, TYPE_POINTER, OBJ_POINTER)),
                ('type_str_obj', CFUNCTYPE(NZRC, TYPE_POINTER, OBJ_POINTER, POINTER(POINTER(c_char))))]

TYPECLASS_POINTER = POINTER(TYPECLASS)

class PORT_INFO(Structure):
    _fields_ = [('block_port_name', c_char_p),
                ('block_port_typeclass_p', TYPECLASS_POINTER),
                ('block_port_type_p', TYPE_POINTER)]

class BLOCK_INFO(Structure):
    _fields_ = [('block_n_inputs', c_size_t),
                ('block_input_port_array', POINTER(PORT_INFO)),
                ('block_n_outputs', c_size_t),
                ('block_output_port_array', POINTER(PORT_INFO)),
                ('block_pull_fns', POINTER(PULL_FN_P))]

class BLOCKCLASS(Structure):
    _fields_ = [('block_id', c_char_p),
                ('block_create', CFUNCTYPE(NZRC, CONTEXT_POINTER, c_char_p, POINTER(BLOCK_STATE_POINTER), POINTER(BLOCK_INFO))),
                ('block_destroy', CFUNCTYPE(None, BLOCK_STATE_POINTER, POINTER(BLOCK_INFO)))]

class BLOCK(Structure):
    pass

BLOCK._fields_ = [('block_state_p', BLOCK_STATE_POINTER),
                  ('block_upstream_pull_fn_p_array', POINTER(PULL_FN_P)),
                  ('block_upstream_block_array', POINTER(BLOCK)),
                  ('block_upstream_output_index_array', POINTER(c_size_t))]

BLOCKCLASS_POINTER = POINTER(BLOCKCLASS)

class NoiseError(Exception):
    def __init__(self, code, filename, linenum, extra = None, *args, **kwargs):
        self.code = code
        self.code_str = str(nz.nz_error_rc_str(code), encoding = 'latin-1')
        self.filename = filename
        self.linenum = linenum
        if extra is not None:
            self.extra = extra
            text = "{0} \"{3}\" on {1}:{2}".format(self.code_str, self.filename, self.linenum, self.extra)
        else:
            text = "{0} on {1}:{2}".format(self.code_str, self.filename, self.linenum)
        super(NoiseError, self).__init__(text, *args, **kwargs)

def handle_nzrc(rc):
    if rc != SUCCESS:
        filename = str(c_char_p.in_dll(nz, 'nz_error_file'), encoding = 'latin-1')
        linenum = int(c_int.in_dll(nz, 'nz_error_line').value)
        if c_char_p.in_dll(nz, 'nz_error_string') == None:
            raise NoiseError(rc, filename, linenum)
        else:
            err_str = c_char_p.in_dll(nz, 'nz_error_string')
            extra = str(err_str, encoding = 'latin-1')
            nz.nz_free_str(err_str)
            raise NoiseError(rc, filename, linenum, extra)

class Library(object):
    def __init__(self, handle, pyhandle):
        self.handle = handle
        self.pyhandle = pyhandle

    def __getattr__(self, attr):
        if self.pyhandle is not None:
            return getattr(self.pyhandle, attr)

class Context(object):
    STRING_ARRAY = POINTER(c_char_p)

    def __init__(self):
        pass

    def __enter__(self):
        self.cp = CONTEXT_POINTER()
        handle_nzrc(nz.nz_context_create(byref(self.cp)))
        self.context_created = True
        self.hooks = {}
        self.libs = []
        self.blocklist = []
        return self

    def __exit__(self, type, value, tb):
        while self.blocklist:
            self.destroy_block(self.blocklist[-1])
        while self.libs:
            self.unload_lib(self.libs[-1])
        nz.nz_context_destroy(self.cp)

    def add_hooks(self, hook_list):
        for (k, v) in hook_list:
            hk = nzhash(k)
            assert hk not in self.hooks
            self.hooks[hk] = v

    def del_hooks(self, hook_list):
        for (k, v) in hook_list:
            hk = nzhash(k)
            del self.hooks[hk]

    def load_lib(self, lib_filename, py_filename = None):
        handle = LIB_HANDLE()
        handle_nzrc(nz.nz_context_load_lib(self.cp, bytes(lib_filename, encoding = 'latin-1'), byref(handle)))
        pyhandle = None
        if py_filename:
            if lib_filename.startswith("/"):
                abs_lib = lib_filename
            else:
                abs_lib = os.path.join(os.getcwd(), lib_filename)
            lib = cdll.LoadLibrary(abs_lib)

            pyhandle = importlib.import_module(py_filename)
            pyhandle.nzlib = lib
            pyhandle.context = self
            if hasattr(pyhandle, 'init'):
                pyhandle.init()
            if hasattr(pyhandle, 'nzhooks'):
                self.add_hooks(pyhandle.nzhooks)

        lib = Library(handle, pyhandle)
        self.libs.append(lib)
        return lib

    def unload_lib(self, lib):
        assert lib in self.libs
        if lib.pyhandle is not None:
            if hasattr(lib.pyhandle, 'nzhooks'):
                self.del_hooks(lib.pyhandle.nzhooks)
            if hasattr(lib.pyhandle, 'deinit'):
                lib.pyhandle.deinit()
        nz.nz_context_unload_lib(self.cp, lib.handle)
        self.libs.remove(lib)

    @property
    def blocks(self):
        result = self.STRING_ARRAY()
        handle_nzrc(nz.nz_context_list_blocks(self.cp, byref(result)))
        blocks = []
        i = 0
        while True:
            block = result[i]
            if block == None:
                break
            blocks.append(str(block, encoding = 'latin-1'))
            i += 1
        nz.nz_context_free_block_list(result)
        return blocks

    @property
    def types(self):
        result = self.STRING_ARRAY()
        handle_nzrc(nz.nz_context_list_types(self.cp, byref(result)))
        types = []
        i = 0
        while True:
            ntype = result[i]
            if ntype == None:
                break
            types.append(str(ntype, encoding = 'latin-1'))
            i += 1
        nz.nz_context_free_type_list(result)
        return types

    def create_block(self, string):
        blockclass = BLOCKCLASS_POINTER()
        block = BLOCK()
        blockinfo = BLOCK_INFO()
        handle_nzrc(nz.nz_context_create_block(self.cp, byref(blockclass), byref(block), byref(blockinfo), bytes(string, encoding = 'latin-1')))
        if nzhash(blockclass) in self.hooks:
            B = self.hooks[nzhash(blockclass)]
        else:
            B = Block
        block = B(blockclass, block, blockinfo)
        self.blocklist.append(block)
        return block

    def destroy_block(self, block):
        assert block in self.blocklist
        nz.nz_context_destroy_block(block.blockclass, byref(block.block), byref(block.info))
        self.blocklist.remove(block)

class Type(object):
    def __init__(self, typeclass, state):
        self.typeclass = typeclass
        self.state = state

    def __str__(self):
        result = POINTER(c_char)()
        handle_nzrc(self.typeclass.contents.type_str(self.state, byref(result)))
        str_result = string_at(result)
        nz.nz_free_str(result)
        return str(str_result, encoding = 'latin-1')

    def __repr__(self):
        return "{}({})".format(type(self).__name__, self.__str__())

    def __eq__(self, other):
        return nz.nz_types_are_equal(self.typeclass, self.state, other.typeclass, other.state)

    def __neq__(self, other):
        return not self.__eq__(other)

class Block(object):
    def __init__(self, blockclass, block, info):
        self.blockclass = blockclass
        self.block = block
        self.info = info

        inputs = []
        for i in range(info.block_n_inputs):
            port = info.block_input_port_array[i]
            inputs.append((str(port.block_port_name, encoding = 'latin-1'),
                           Type(port.block_port_typeclass_p, port.block_port_type_p)))
        self.inputs = inputs
        outputs = []
        for i in range(info.block_n_outputs):
            port = info.block_output_port_array[i]
            outputs.append((str(port.block_port_name, encoding = 'latin-1'),
                            Type(port.block_port_typeclass_p, port.block_port_type_p)))
        self.outputs = outputs

        self.input_connections = [None] * len(inputs)
        self.output_connections = [None] * len(outputs)

    def __str__(self):
        return str(self.blockclass.contents.block_id, encoding = 'latin-1')

    def __repr__(self):
        return "{}({})".format(type(self).__name__, self.__str__())

def connect(upstream_block, out_name, downstream_block, in_name):
    out_index = dict(zip(upstream_block.outputs[0], range(len(upstream_block.outputs))))[out_name]
    in_index = dict(zip(downstream_block.inputs[0], range(len(downstream_block.inputs))))[in_name]

    assert upstream_block.output_connections[out_index] is None
    assert downstream_block.input_connections[in_index] is None

    out_type = upstream_block.outputs[out_index][1]
    in_type = downstream_block.inputs[in_index][1]
    assert out_type == in_type

    nz.nz_block_set_upstream(byref(downstream_block.block),
                             in_index,
                             byref(upstream_block.block),
                             byref(upstream_block.info),
                             out_index)

    upstream_block.output_connections[out_index] = (downstream_block, in_index)
    downstream_block.input_connections[in_index] = (upstream_block, out_index)

def disconnect(upstream_block, out_name, downstream_block, in_name):
    out_index = dict(zip(upstream_block.outputs[0], range(len(upstream_block.outputs))))[out_name]
    in_index = dict(zip(downstream_block.inputs[0], range(len(downstream_block.inputs))))[in_name]
    assert upstream_block.output_connections[out_index] == (downstream_block, in_index)
    assert downstream_block.input_connections[in_index] == (upstream_block, out_index)

    nz.nz_block_clear_upstream(byref(downstream_block.block), in_index)

    upstream_block.output_connections[out_index] = None
    downstream_block.input_connections[in_index] = None

def nzhash(funcptr):
    return cast(funcptr, c_void_p).value

if __name__ == '__main__':
    import time

    with Context() as c:
        l = c.load_lib("../nzlib/nzstd.so", "nzlib.std")

        with l.PortAudio:
            b1 = c.create_block("constant(real,440)")
            b2 = c.create_block("wave(saw)")
            b3 = c.create_block("pa")
            connect(b1, "out", b2, "freq")
            connect(b2, "out", b3, "in")
            b3.start()
            time.sleep(0.5)
            b3.stop()
