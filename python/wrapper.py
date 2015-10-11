import ctypes
import os

def load_so(soname):
    return ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),soname))

libnoise = load_so("../libnoise.so")

class NzType(ctypes.Structure):
    pass
NzTypeP = ctypes.POINTER(NzType)

class NzObj(ctypes.Structure):
    _fields_ = [("type", NzTypeP)]
NzObjP = ctypes.POINTER(NzObj)

class NzNode(ctypes.Structure):
    pass
NzNodeP = ctypes.POINTER(NzNode)

class NzPort(ctypes.Structure):
    pass
NzPortP = ctypes.POINTER(NzPort)

class NzNote(ctypes.Structure):
    pass
NzNoteP = ctypes.POINTER(NzNote)

def check_null(result, func, args):
    if result == None:
        raise Exception("NULL ptr returned from {}, errno = {}".format(func, ctypes.get_errno()))

def check_rc(result, func, args):
    if result != 0:
        raise Exception("Nonzero rc {} returned from {}, errno = {}".format(result, func, ctypes.get_errno()))

def check_negative(result, func, args):
    if result < 0:
        raise Exception("Negative rc {} returned from {}, errno = {}".format(result, func, ctypes.get_errno()))

FUNCTIONS = {
    # ntypes.h
    "nz_obj_create": (NzObjP, [NzTypeP], check_null),
    "nz_obj_destroy": (None, [ctypes.POINTER(NzObjP)], None),
    "nz_obj_copy": (NzObjP, [NzObjP, NzObjP], check_null),
    "nz_obj_str": (ctypes.c_char_p, [NzObjP], check_null),
    "nz_type_create_simple": (NzTypeP, [ctypes.c_size_t], check_null),
    "nz_type_create_vector": (NzTypeP, [ctypes.c_size_t], check_null),
    "nz_vector_set_size": (ctypes.c_size_t, [NzObjP, ctypes.c_size_t], None), #TODO: check
    "nz_vector_get_size": (ctypes.c_size_t, [NzObjP], None),
    "nz_vector_at": (ctypes.c_void_p, [NzObjP, ctypes.c_size_t], check_null),
    "nz_vector_push_back": (ctypes.c_int, [NzObjP, ctypes.c_void_p], check_rc),
    "nz_vector_erase": (None, [NzObjP, ctypes.c_int], None),
    "nz_vector_sizeofel": (ctypes.c_size_t, [NzObjP], None),

    # block.h
    "nz_node_connect": (ctypes.c_int, [NzNodeP, ctypes.c_size_t, NzNodeP, ctypes.c_size_t], check_rc),
    "nz_port_pull": (NzObjP, [NzPortP], check_null),

    # note.h
    "nz_note_init": (None, [NzNoteP, ctypes.c_double, ctypes.c_double], None),

    #
    #"soundcard_get": (NzNodeP, [], check_null),
    #"soundcard_run": (None, [], None),
}

BLOCKS = {
    "accumulator": [],
    "constant": [NzObjP],
    "debug": [ctypes.c_char_p, ctypes.c_bool],
    "math": [ctypes.c_int],
    "sequencer": [],
    "tee": [ctypes.c_size_t],
    "wye": [ctypes.c_size_t],

    "compressor": [],
    "fungen": [],
    "impulse": [],
    "lpf": [],
    "clpf": [],
    "mixer": [ctypes.c_size_t],
    "cmixer": [ctypes.c_size_t],
    "recorder": [],
    "sampler": [],
    "wave": [],
    "white": [],

    "midireader": [ctypes.c_char_p],
    "midiintegrator": [],

    "instrument_sine": [],
    "instrument_saw": [],
    "instrument_snare": [],
}

nz_double_type = NzTypeP(libnoise.nz_double_type)
nz_long_type = NzTypeP(libnoise.nz_long_type)
nz_string_type = NzTypeP(libnoise.nz_string_type)
nz_chunk_type = NzTypeP(libnoise.nz_chunk_type)
nz_sample_type = NzTypeP(libnoise.nz_sample_type)
nz_note_vector_type = NzTypeP(libnoise.nz_note_vector_type)
nz_object_vector_type = NzTypeP(libnoise.nz_object_vector_type)

exports = globals()

for func_name, (return_type, arg_types, err_check) in FUNCTIONS.items():
    exports[func_name] = getattr(libnoise, func_name)
    exports[func_name].restype = return_type
    exports[func_name].argtypes = arg_types
    if err_check is not None:
        exports[func_name].errcheck = err_check

blocks = {}

for block_name, args in BLOCKS.items():
    blocks[func_name] = getattr(libnoise, "nz_{}_init".format(block_name))
    blocks[func_name].argtypes = [NzNodeP] + args
    blocks[func_name].restype = ctypes.c_int
    blocks[func_name].errcheck = check_rc

