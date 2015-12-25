from ctypes import *

nz = cdll.LoadLibrary("libnoise.so")
nz.nz_error_rc_str.restype = c_char_p

SUCCESS = 0

NZRC = c_int
CONTEXT_POINTER = c_void_p
BLOCK_POINTER = c_void_p
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
                ('block_create', CFUNCTYPE(NZRC, CONTEXT_POINTER, c_char_p, POINTER(BLOCK_POINTER), POINTER(BLOCK_INFO))),
                ('block_destroy', CFUNCTYPE(None, BLOCK_POINTER, POINTER(BLOCK_INFO)))]

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
        filename = str(c_char_p.in_dll(nz, 'nz_error_file').value, encoding = 'latin-1')
        linenum = int(c_int.in_dll(nz, 'nz_error_line').value)
        if c_char_p.in_dll(nz, 'nz_error_string').value == None:
            raise NoiseError(rc, filename, linenum)
        else:
            err_str = c_char_p.in_dll(nz, 'nz_error_string')
            extra = str(err_str.value, encoding = 'latin-1')
            nz.nz_free_str(err_str)
            raise NoiseError(rc, filename, linenum, extra)

class Context(object):
    STRING_ARRAY = POINTER(c_char_p)

    def __init__(self):
        self.context_created = False
        self.cp = CONTEXT_POINTER()
        handle_nzrc(nz.nz_context_create(byref(self.cp)))
        self.libs = []
        self.context_created = True

    def __del__(self):
        if self.context_created:
            for lib in self.libs:
                self.unload_lib(lib)
            nz.nz_context_destroy(self.cp)

    def load_lib(self, lib_filename):
        lib_handle = LIB_HANDLE()
        handle_nzrc(nz.nz_context_load_lib(self.cp, bytes(lib_filename, encoding = 'latin-1'), byref(lib_handle)))
        self.libs.append(lib_handle)
        return lib_handle

    def unload_lib(self, lib):
        assert lib in self.libs
        nz.nz_context_unload_lib(self.cp, lib)
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
        blockstate = BLOCK_POINTER()
        blockinfo = BLOCK_INFO()
        handle_nzrc(nz.nz_context_create_block(self.cp, byref(blockclass), byref(blockstate), byref(blockinfo), bytes(string, encoding = 'latin-1')))
        return Block(blockclass, blockstate, blockinfo)

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

    def __del__(self):
        self.typeclass.contents.type_destroy(self.state)

class Block(object):
    def __init__(self, blockclass, state, info):
        self.blockclass = blockclass
        self.state = state
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

        self.output_connections = []
        self.input_connections = []

    def __str__(self):
        return str(self.blockclass.contents.block_id, encoding = 'latin-1')

    def __repr__(self):
        return "{}({})".format(type(self).__name__, self.__str__())

    def __del__(self):
        self.blockclass.contents.block_destroy(self.state, self.info)

def connect(upstream_block, out_name, downstream_block, in_name):
    out_index = dict(zip(upstream_block.outputs[0], range(len(upstream_block.outputs))))[out_name]
    in_index = dict(zip(downstream_block.inputs[0], range(len(downstream_block.inputs))))[in_name]

    out_type = upstream_block.outputs[out_index][1]
    in_type = downstream_block.inputs[in_index][1]
    assert out_type == in_type

    downstream_block.state.block_upstream_pull_fn_p_array[in_index] = upstream_block.info.block_pull_fns[out_index]
    downstream_block.state.block_upstream_block_array[in_index] = upstream_block.block
    downstream_block.state.block_upstream_output_index_array[in_index] = out_index

class _PortAudio:
    def __enter__(self):
        handle_nzrc(nz.pa_init())

    def __exit__(self, type, value, tb):
        nz.pa_term()

    def start(self, blockhandle):
        handle_nzrc(nz.pa_start(blockhandle))

    def stop(self, blockhandle):
        handle_nzrc(nz.pa_stop(blockhandle))

pa = _PortAudio()

if __name__ == '__main__':
    c = Context()
    c.load_lib("libstd.so")
    b = c.create_block("constant(int,3)")
    print(repr(b))
    print(b.outputs)
    del b
    del c
