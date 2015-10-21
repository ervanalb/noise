import wrapper

def camel_case(words):
    return "".join([w.capitalize() for w in words.split("_")])


class NType(object):
    def __init__(self):
        self._as_parameter_ = self._type_p

# Metaprogramming to generate a class for each type
for type_name, type_p in wrapper.types.items():
    type_class_name = camel_case(type_name) + "Type"
    globals()[type_class_name] = type(type_class_name, (NType,), {
        "type_name": type_name,
        "_type_p": type_p
    })


class Block(object):
    def __init__(self, *args):
        assert len(args) + 1 == len(self._block_fn.argtypes)
        self._node = wrapper.NzNode()
        self._block_fn(wrapper.byref(self._node), *args)
        self._node_str = wrapper.nz_node_str(wrapper.byref(self._node))
        self._as_parameter_ = wrapper.byref(self._node)

    def __del__(self):
        # TODO: free self._node_str
        pass

    def __str__(self):
        return self._node_str

    def pull(self, port_idx):
        result = wrapper.nz_node_pull(self, port_idx)
        return result

    @property
    def inputs(self):
        n_inputs = self._node.n_inputs
        _inputs_array = (wrapper.NzInport * n_inputs)(self._node.inputs)
        return _inputs_array

# Metaprogramming to generate a class for each block
for block_name, block_fn in wrapper.blocks.items():
    block_class_name = camel_case(block_name) + "Block"
    globals()[block_class_name] = type(block_class_name, (Block,), {
        "block_name": block_name,
        "_block_fn": block_fn
    })


