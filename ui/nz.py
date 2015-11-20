from ctypes import *

nz = cdll.LoadLibrary("libnoise.so")
nz.nz_error_rc_str.restype = c_char_p

SUCCESS = 0

CONTEXT_POINTER = c_void_p
GRAPH_POINTER = c_void_p
BLOCK_HANDLE = c_void_p
TYPECLASS_POINTER = c_void_p
TYPE_POINTER = c_void_p
PULL_FN_P = c_void_p

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
            text = "{0} on {1}:{2}".format(self.code_str, self.filename, self.linenum, self.extra)
        super(NoiseError, self).__init__(text, *args, **kwargs)

def handle_nzrc(rc):
    if rc != SUCCESS:
        filename = str(c_char_p.in_dll(nz, 'nz_error_file').value, encoding = 'latin-1')
        linenum = int(c_int.in_dll(nz, 'nz_error_line').value)
        if c_char_p.in_dll(nz, 'nz_error_string').value == None:
            raise NoiseError(rc, filename, linenum)
        else:
            extra = str(c_char_p.in_dll(nz, 'nz_error_string').value, encoding = 'latin-1')
            nz.nz_error_string_free()
            raise NoiseError(rc, filename, linenum, extra)

class Context(object):
    STRING_ARRAY = POINTER(c_char_p)

    def __init__(self):
        self.context_created = False
        self.cp = CONTEXT_POINTER()
        handle_nzrc(nz.nz_context_create(byref(self.cp)))
        self.context_created = True

    def __del__(self):
        if self.context_created:
            nz.nz_context_destroy(self.cp)

    @property
    def blocks(self):
        result = self.STRING_ARRAY()
        handle_nzrc(nz.nz_block_list(self.cp, byref(result)))
        blocks = []
        i = 0
        while True:
            block = result[i]
            if block == None:
                break
            blocks.append(str(block, encoding = 'latin-1'))
            i += 1
        nz.nz_block_list_free(result)
        return blocks

    @property
    def types(self):
        result = self.STRING_ARRAY()
        handle_nzrc(nz.nz_type_list(self.cp, byref(result)))
        types = []
        i = 0
        while True:
            ntype = result[i]
            if ntype == None:
                break
            types.append(str(ntype, encoding = 'latin-1'))
            i += 1
        nz.nz_type_list_free(result)
        return types

class Graph(object):
    def __init__(self, context):
        self.graph_created = False
        self.context = context
        self.gp = GRAPH_POINTER()
        handle_nzrc(nz.nz_graph_create(self.context.cp, byref(self.gp)))
        self.graph_created = True

    def __del__(self):
        if self.graph_created:
            nz.nz_graph_destroy(self.gp)

    def add_block(self, name, constructor):
        handle_nzrc(nz.nz_graph_add_block(self.gp, bytes(name, encoding = 'latin-1'), bytes(constructor, encoding = 'latin-1')))

    def block_info(self, name):
        block_info_p = POINTER(BLOCK_INFO)()
        handle_nzrc(nz.nz_graph_block_info(self.gp, bytes(name, encoding = 'latin-1'), byref(block_info_p)))
        return BlockInfo(block_info_p)

    def block_handle(self, name):
        block_handle = BLOCK_HANDLE()
        handle_nzrc(nz.nz_graph_block_handle(self.gp, bytes(name, encoding = 'latin-1'), byref(block_handle)))
        return block_handle

    def del_block(self, name):
        if self.block_added:
            handle_nzrc(nz.nz_graph_del_block(self.graph.gp, bytes(self.name, encoding = 'latin-1')))

    def connect(self, output_block, output_port, input_block, input_port):
        handle_nzrc(nz.nz_graph_connect(
            self.gp,
            bytes(output_block, encoding = 'latin-1'),
            bytes(output_port, encoding = 'latin-1'),
            bytes(input_block, encoding = 'latin-1'),
            bytes(input_port, encoding = 'latin-1')
        ))

    def disconnect(self, output_block, output_port, input_block, input_port):
        handle_nzrc(nz.nz_graph_disconnect(
            self.gp,
            bytes(output_block, encoding = 'latin-1'),
            bytes(output_port, encoding = 'latin-1'),
            bytes(input_block, encoding = 'latin-1'),
            bytes(input_port, encoding = 'latin-1')
        ))

class BlockInfo(object):
    def __init__(self, struct_pointer):
        inputs = []
        for i in range(struct_pointer.contents.block_n_inputs):
            inputs.append(struct_pointer.contents.block_input_port_array[i].block_port_name)
        self.inputs = inputs
        outputs = []
        for i in range(struct_pointer.contents.block_n_outputs):
            outputs.append(struct_pointer.contents.block_output_port_array[i].block_port_name)
        self.outputs = outputs

class _PortAudio:
    def __enter__(self):
        handle_nzrc(nz.pa_init())

    def __exit__(self, type, value, tb):
        nz.pa_term()

    def start(self, blockhandle):
        handle_nzrc(nz.pa_start(blockhandle))

pa = _PortAudio()

if __name__ == '__main__':
    c = Context()
    g = Graph(c)
    with pa:
        g.add_block("freq1", "constant(real,440)")
        g.add_block("sound1", "wave(saw)")
        g.add_block("pa1", "pa")
        g.connect("freq1", "out", "sound1", "freq")
        g.connect("sound1", "out", "pa1", "in")
        pa.start(g.block_handle("pa1"))
