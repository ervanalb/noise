from ctypes import *

class TypeFactory(object):
	def __init__(self,alloc,free,copy,typeinfo,string):
		self.alloc=alloc
		self.free=free
		self.copy=copy
		self.typeinfo=typeinfo
		self.string=string

	def __eq__(self, other):
		return self.alloc==other.alloc and self.free==other.free and self.copy==other.copy and self.typeinfo==other.typeinfo

	def __str__(self):
		return self.string

