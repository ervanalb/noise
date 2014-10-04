from ctypes import *
import os

print "LOADING CTYPES!"

STATE_PT = c_void_p
OUTPUT_PT = c_void_p
BLOCK_INFO_PT = c_void_p

class NODE_T(Structure):
	pass

PULL_FN_PT = POINTER(CFUNCTYPE(c_int, POINTER(NODE_T), POINTER(OUTPUT_PT)))

NODE_T._fields_ = [
	('input_node',POINTER(NODE_T)),
	('input_pull',PULL_FN_PT),
	('state',STATE_PT),
	]

NODE_PT = POINTER(NODE_T)

class NoiseLib(object):
	def __init__(self,global_vars):
		self.libs={}
		self.global_vars=global_vars

	def load(self,name,filename):
		self.libs[name]=cdll.LoadLibrary(os.path.join(os.path.abspath(os.path.dirname(__file__)),filename))
		self.set_global_vars(self.libs[name])

	def set_global_vars(self,lib):
		for (t,k,v) in self.global_vars:
			t.in_dll(lib,k).value=v

	def __getitem__(self,index):
		return self.libs[index]

#CHUNKSIZE = 128
#FRAMERATE = 48000

#c_int.in_dll(clib_noise, "global_chunk_size").value = CHUNKSIZE
#c_int.in_dll(clib_noisee, "global_frame_rate").value = FRAMERATE

