import ctypes
import os

from ctypes import byref, pointer

def _load_so(soname):
    return ctypes.cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),soname))

libnoise = _load_so("../libnoise.so")

class NzType(ctypes.Structure):
    pass
NzTypeP = ctypes.POINTER(NzType)

class NzObj(ctypes.Structure):
    _fields_ = [("type", NzTypeP)]
NzObjP = ctypes.POINTER(NzObj)

class NzPort(ctypes.Structure):
    pass
NzPortP = ctypes.POINTER(NzPort)

class NzInport(ctypes.Structure):
    _fields = [
        ("type", NzTypeP),
        ("name", ctypes.c_char_p),
        ("port", NzPortP),
    ]
NzInportP = ctypes.POINTER(NzInport)

class NzNode(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char_p),
        ("_term", ctypes.c_void_p),
        ("_state", ctypes.c_void_p),
        ("n_inputs", ctypes.c_size_t),
        ("inputs", NzInportP),
        ("n_outputs", ctypes.c_size_t),
        ("outputs", NzPortP),
        ("_flags", ctypes.c_int)
    ]
NzNodeP = ctypes.POINTER(NzNode)

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

_FUNCTIONS = {
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
    "nz_node_term": (None, [NzNodeP], None),
    "nz_node_str": (ctypes.c_char_p, [NzNodeP], check_null),
    "nz_port_pull": (NzObjP, [NzPortP], check_null),
    "nz_node_pull": (NzObjP, [NzNodeP, ctypes.c_size_t], check_null),

    # note.h
    "nz_note_init": (None, [NzNoteP, ctypes.c_double, ctypes.c_double], None),

    #
    #"soundcard_get": (NzNodeP, [], check_null),
    #"soundcard_run": (None, [], None),
}

_BLOCKS = {
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

_TYPES = [
    "double",
    "long",
    "string",
    "chunk",
    "sample",
    "note_vector",
    "object_vector"
]

_exports = globals()

for func_name, (return_type, arg_types, err_check) in _FUNCTIONS.items():
    _exports[func_name] = getattr(libnoise, func_name)
    _exports[func_name].restype = return_type
    _exports[func_name].argtypes = arg_types
    if err_check is not None:
        _exports[func_name].errcheck = err_check

blocks = {}

for block_name, args in _BLOCKS.items():
    blocks[block_name] = getattr(libnoise, "nz_{}_init".format(block_name))
    blocks[block_name].argtypes = [NzNodeP] + args
    blocks[block_name].restype = ctypes.c_int
    blocks[block_name].errcheck = check_rc

types = {}

for type_name in _TYPES:
    types[type_name] = NzTypeP(getattr(libnoise, "nz_{}_type".format(type_name)))
