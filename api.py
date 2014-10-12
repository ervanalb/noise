import cnoise
import ntype
import threading

from flask import Flask, request, jsonify, render_template
from flask.ext.restful import reqparse, abort, Api, Resource, marshal_with, fields

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000
context.load('blocks.py')

app = Flask(__name__, static_folder="spider")
api = Api(app)

next_block_id = 0
block_inventory = {}

next_connection_id = 0
connection_inventory = {}


block_fields = {
    'x': fields.Float,
    'y': fields.Float,
    'block_id': fields.Integer,
    'args': fields.Raw,
    'kwargs': fields.Raw,
    'block_class': fields.String,
    'name': fields.String,
}

def parse_arg(arg):
    if isinstance(arg, dict):
        if "__type__" in arg:
            typename = arg["__type__"]
            args = arg.get("args", [])
            kwargs = arg.get("kwargs", {})
            return context.types[typename].new(*args, **kwargs)
    return arg


@app.route("/status")
def list_available_blocks():
    block_types = [ 
        {
            "class": name,
            "num_inputs": blk.num_inputs,
            "num_outputs": blk.num_outputs
        } for name, blk in context.blocks.items()
    ]

    type_types = [
        {
            "class": name,
            "string": str(typ),
        } for name, typ in context.types.items()
    ]
    block_inv = [
        {
            "id": bid,
            "class": blk.__class__.__name__, #XXX
            "x": blk.x,
            "y": blk.y
        } for bid, blk in block_inventory.items()
    ]
    conn_inv = [
        {
            "id": cid,
            "conn": conn
        } for cid, conn in connection_inventory.items()
    ]
    #return jsonify(blocks=block_types, types=type_types, block_instances=block_inv, connection_instances=conn_inv)
    return jsonify(blocks=block_types, types=type_types)


class Block(Resource):
    def assert_exists(self, block_id):
        if block_id not in block_inventory:
            abort(404, "Block {} does not exist".format(block_id))

    @marshal_with(block_fields)
    def get(self, block_id):
        self.assert_exists(block_id)
        return block_inventory[block_id]

    def delete(self, block_id):
        self.assert_exists(block_id)
        del block_inventory[block_id]
        return '', 204

    @marshal_with(block_fields)
    def put(self, block_id):
        self.assert_exists(block_id)
        block = block_inventory[block_id]
        data = request.get_json()
        block.x = float(data.get('x', block.x))
        block.y = float(data.get('y', block.y))
        return block

class BlockList(Resource):
    @marshal_with(block_fields)
    def get(self):
        return block_inventory.values()

    @marshal_with(block_fields)
    def post(self):
        global next_block_id
        data = request.get_json()
        block_class = data["block_class"]
        args = map(parse_arg, data.get("args", []))
        kwargs = data.get("kwargs", {})

        x = float(data.get('x', 100))
        y = float(data.get('y', 100))

        block = context.blocks[block_class](*args, **kwargs)
        block.block_class = block_class
        block.x = x
        block.y = y
        block.name = data.get('name', block_class)
        block.args = data.get("args", [])
        block.kwargs = data.get("kwargs", {})
        block.block_id = next_block_id 

        block_inventory[next_block_id] = block
        next_block_id += 1

        return block

class Connection(Resource):
    def assert_exists(self, connection_id):
        if connection_id not in connection_inventory:
            abort(404, "Connection {} does not exist".format(connection_id))

    def get(self, connection_id):
        self.assert_exists(connection_id)
        return connection_inventory[connection_id]

    def delete(self, connection_id):
        self.assert_exists(connection_id)
        del connection_inventory[connection_id]
        #TODO: unconnect blocks
        return '', 204

    def put(self, connection_id):
        self.assert_exists(connection_id)
        connection_inventory[connection_id]
        data = request.get_json()

        #TODO: I don't think this is useful
        return connection

class ConnectionList(Resource):
    def get(self):
        return connection_inventory.values()

    def post(self):
        global next_connection_id
        data = request.get_json(force=True)

        source_block_id = int(data["source_id"])
        target_block_id = int(data["target_id"])
        source_idx = int(data["source_idx"])
        target_idx = int(data["target_idx"])

        source_block = block_inventory[source_block_id]
        target_block = block_inventory[target_block_id]

        connection = {
            "source_id": source_block_id,
            "target_id": target_block_id,
            "source_idx": source_idx,
            "target_idx": target_idx
        }

        target_block.set_input(target_idx, source_block, source_idx)

        connection["connection_id"] = next_connection_id
        connection_inventory[next_connection_id] = connection
        next_connection_id += 1

        return connection

api.add_resource(BlockList, '/blocks')
api.add_resource(Block, '/blocks/<int:block_id>')
api.add_resource(ConnectionList, '/connections')
api.add_resource(Connection, '/connections/<int:connection_id>')
        

def main():
    import pyaudio
    import struct
    import threading
    import time

    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paFloat32,
        channels=1,
        rate=context.frame_rate,
        frames_per_buffer=context.chunk_size,
        output=True)

    print stream.get_output_latency()

    thread = threading.Thread(target=app.run)
    thread.start()

    ui_block = context.blocks["UIBlock"]()
    ui_block.block_class = "UIBlock"
    ui_block.x = 400
    ui_block.y = 200
    ui_block.name = "Audio"
    ui_block.args = []
    ui_block.kwargs = {}
    ui_block.block_id = 0 
    block_inventory[0] = ui_block
    global next_block_id
    next_block_id += 1

    while True:
        try:
            #cb.cvalue.value += 10
            if len(block_inventory) < 5:
                print 'waiting'
                time.sleep(1)
                continue
            result = ui_block.pull()
            data=struct.pack('f'*context.chunk_size,*(ui_block.output[:context.chunk_size]))
            stream.write(data)

        except KeyboardInterrupt:
            break
 
    stream.stop_stream()
    stream.close()
    thread.join(1.0)



if __name__ == "__main__":
    main()
