from ctypes import *

nz = cdll.LoadLibrary("libnoise.so")
nz.nz_error_rc_str.restype = c_char_p

SUCCESS = 0

CONTEXT_POINTER = c_void_p
GRAPH_POINTER = c_void_p
BLOCK_HANDLE = c_void_p

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
        block_handle = BLOCK_HANDLE()
        handle_nzrc(nz.nz_graph_add_block(self.gp, bytes(name, encoding = 'latin-1'), bytes(constructor, encoding = 'latin-1'), byref(block_handle)))

if __name__ == '__main__':
    c = Context()
    g = Graph(c)
    g.add_block("sound1", "wave(saw)")
