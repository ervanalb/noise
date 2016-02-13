from nz import *

class PaBlock(Block):
    def start(self):
        nzlib.pa_start(byref(self.block))

    def stop(self):
        nzlib.pa_stop(byref(self.block))

class _PortAudio:
    def __enter__(self):
        handle_nzrc(nzlib.pa_init())

    def __exit__(self, type, value, tb):
        nzlib.pa_term()

PortAudio = _PortAudio()

def init():
    global nzhooks

    nzhooks = (
        (nzlib.nz_pa_blockclass, PaBlock),
    )
