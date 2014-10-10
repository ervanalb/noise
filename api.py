import cnoise
import ntype
import threading

from flask import Flask, request, jsonify, render_template

context=cnoise.NoiseContext()
context.chunk_size = 128
context.frame_rate = 48000
context.load('blocks.py')

app = Flask(__name__, static_folder="spider")

next_block_id = 0
block_inventory = {}

next_connection_id = 0
connection_inventory = {}

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
            "class": blk.__class__.__name__ #XXX
        } for bid, blk in block_inventory.items()
    ]
    conn_inv = [
        {
            "id": cid,
            "conn": conn
        } for cid, conn in connection_inventory.items()
    ]
    return jsonify(blocks=block_types, types=type_types, block_instances=block_inv, connection_instances=conn_inv)

@app.route("/new/block", methods=["POST"])
def new_block():
    global next_block_id
    #data = request.get_json(force=True)
    data = request.form
    block_class = data["class"]
    args = data.get("args", [])
    kwargs = data.get("args", {})

    block = context.blocks[block_class](*args, **kwargs)
    block_inventory[next_block_id] = block
    next_block_id += 1
    return jsonify(id=next_block_id - 1, bclass=block_class)

@app.route("/new/connection", methods=["POST"])
def new_connection():
    global next_connection_id
    #data = request.get_json(force=True)
    data = request.form

    source_block_id = int(data["source_block_id"])
    target_block_id = int(data["target_block_id"])
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

    connection_inventory[next_connection_id] = connection
    next_connection_id += 1

    return jsonify(id=next_connection_id - 1)

@app.route("/delete/connection", methods=["POST"])
def delete_connection():
    #TODO
    data = request.form
    del connection_inventory[int(data["id"])]

    return jsonify(old_id=next_connection_id)
    

if __name__ == "__main__":
    app.run(debug=True)
