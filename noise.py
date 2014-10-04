from ctypes import *
import os
import pyaudio

clib=cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),'noise.so'))
print clib.wave_pull

CHUNKSIZE = 128
FRAMERATE = 48000

c_int.in_dll(clib, "global_chunk_size").value = CHUNKSIZE
c_int.in_dll(clib, "global_frame_rate").value = FRAMERATE

p = pyaudio.PyAudio()

stream = p.open(format=pyaudio.paFloat32,
	channels=1,
	rate=FRAMERATE,
	output=True)

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p

class NODE_T(Structure):
	pass

PULL_FN_PT = CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT))

NODE_T._fields_ = [
	('input_node',POINTER(NODE_T)),
	('input_pull',PULL_FN_PT),
	('state',STATE_PT),
	]


n=NODE_T()

ui_pulls=(PULL_FN_PT*1)()
ui_pulls[0].value = clib.constant_frequency

clib.wave_state_alloc(BLOCK_INFO_PT(),byref(n,NODE_T.state.offset))


output = POINTER(c_double)()

print ui_pulls
print byref(ui_pulls)

while True:
	result=clib.wave_pull(byref(n),byref(output))
	data=struct.pack('f'*CHUNKSIZE,*out)
	stream.write(data)

stream.stop_stream()
stream.close()

p.terminate()

