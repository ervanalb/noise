from ctypes import *

nz = cdll.LoadLibrary("libnoise.so")

SUCCESS = 0

CONTEXT_POINTER = c_void_p

class NoiseError(Exception):
    def __init__(self, code, filename, linenum, extra = None, *args, **kwargs):
        super(NoiseError, self).__init__(*args, **kwargs)
        self.code = code
        self.filename = filename
        self.linenum = linenum
        self.extra = extra

    def __str__(self):
        return "{0}, {1}".format(self.code, self.extra)

def handle_nzrc(rc):
    if rc != SUCCESS:
        raise NoiseError(rc, nz.nz_error_file, nz.nz_error_line)

class Context(object):
    STRING_ARRAY = POINTER(c_char_p)

    def __init__(self):
        self.context_created = False
        self.context = CONTEXT_POINTER()
        handle_nzrc(nz.nz_context_create(byref(self.context)))
        self.context_created = True

    def __del__(self):
        if self.context_created:
            nz.nz_context_destroy(self.context)

    @property
    def blocks(self):
        result = self.STRING_ARRAY()
        handle_nzrc(nz.nz_block_list(self.context, byref(result)))
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
        handle_nzrc(nz.nz_type_list(self.context, byref(result)))
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

if __name__ == '__main__':
    c = Context()
    print(c.blocks)
    print(c.types)
