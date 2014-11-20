import copy
import json
import ply.lex as lex
import ply.yacc as yacc

tokens = (
    "NAME",
    "INPUT", "OUTPUT",
    "JSONPARAMS",
    "SUBCKT", "ENDS", "INC",
    "FILENAME",
    "COMMENT",
    "NL",
)

t_NAME = r"[a-zA-Z_][a-zA-Z0-9_-]*"
t_INPUT = r"<<"
t_OUTPUT = r">>"
#t_JSONPARAMS = r".*"
t_SUBCKT = r"\.subckt"
t_ENDS = r"\.ends"
t_INC = r"\.inc"
t_FILENAME = r'"[-_.A-Za-z0-9]+"'

t_ignore = " \t"

def t_NL(t):
    r"\n+"
    t.lexer.lineno += t.value.count("\n")
    return t

def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)

def t_comment(t):
    r"\#.*"
    pass

def t_JSONPARAMS(t):
    r"{.*}"
    try:
        t.value = json.loads(t.value)
        return t
    except ValueError:
        raise SyntaxError

lex.lex()

subckts = {}
global_nets = {}
global_blocks = {}
global_wires = {}
current_subckt = None

def p_statement_include(p):
    "statement : INC FILENAME NL"
    #print "Include file: ", p[2]

def p_port(p):
    """port : NAME INPUT NAME
            | NAME OUTPUT NAME
            | INPUT NAME
            | OUTPUT NAME """
    if len(p) == 4:
        port_name, output, net_name = p[1], p[2] == ">>", p[3]
    else:
        port_name, output, net_name = None, p[1] == ">>", p[2]
    p[0] =  (port_name, net_name, output)

def p_ports(p):
    "port_list : port"
    p[0] = [p[1]]

def p_port_list(p):
    "port_list : port_list port"
    p[0] = p[1] + [p[2]]

def p_block(p):
    """block : NAME NAME port_list NL   
       block : NAME NAME port_list JSONPARAMS NL"""
    global current_subckt
    block_name = p[1]
    class_name = p[2]
    ports = p[3]
    if len(p) > 5:
        json_params = p[4]
    else:
        json_params = None
    #print "Block:", (block_name, class_name, ports, json_params)

    if current_subckt is None:
        blocks = global_blocks
        nets = global_nets
    else:
        blocks = subckts[current_subckt]["blocks"]
        nets = subckts[current_subckt]["nets"]

    if block_name in blocks:
        raise SyntaxError("Block name %s is already defined" % block_name)

    if class_name in subckts: 
        prefix = block_name + "."
        subckt = subckts[class_name]

        for name, block in subckt["blocks"].items():
            blocks[prefix + name] = copy.deepcopy(block)
        print 'nets', subckt["nets"]
        for name, net in subckt["nets"].items():
            inp, outp = net[0], net[1]
            if net[0] is not None:
                inp_name, inp_port = net[0]
                inp_name = prefix + inp_name
                inp = inp_name, inp_port
            if net[1] is not None:
                outp_name, outp_port = net[1]
                outp_name = prefix + outp_name
                outp = outp_name, outp_port
            nets[prefix + name] = inp, outp
    else:
        prefix = ""
        subckt = None

    #blocks[block_name] = {
    input_idx = 0
    output_idx = 0
    subckt_inputs = {}
    subckt_outputs = {}
    for i, (pname, pnet, poutput) in enumerate(ports):
        pnet = pnet
        net = nets.get(pnet, (None, None))
        if poutput:
            if pname is None:
                pname = output_idx
            if net[0] is not None:
                raise Exception("Net '%s' has multiple inputs" % pnet)
            if subckt is not None:
                snet_name = prefix + subckt["inputs"][pname]
                snet = nets.pop(snet_name)
                assert snet[1] is None
                net = (snet[0][0], snet[0][1]), net[1]
                print 141, snet
            else:
                net = (prefix + block_name, pname), net[1]
            print 144, pnet, net
            #subckt_outputs[output_idx] = pnet, pname
            #subckt_outputs[pname] = pnet, pname
            output_idx += 1
        else:
            if pname is None:
                pname = input_idx
            if net[1] is not None:
                raise Exception("Net '%s' has multiple outputs" % pnet)
            if subckt is not None:
                snet_name = prefix + subckt["outputs"][pname]
                snet = nets.pop(snet_name)
                assert snet[0] is None
                net = net[0], (snet[1][0], snet[1][1])
                print 158, snet
            else:
                net = net[0], (prefix + block_name, pname)
            print 160, pnet, net
            #subckt_inputs[input_idx] = (pnet, pname)
            #subckt_inputs[pname] = (pnet, pname)
            input_idx += 1
        nets[pnet] = net

    # print 'subc', subckt_outputs, subckt_inputs
    if subckt is None:
        blocks[block_name] = {
            "class": class_name,
            "inputs" : [],
            #"outputs": [],
            "params": json_params,
            #"is_subckt": subckt is not None
        }


    p[0] = (block_name, class_name, ports, json_params)

def p_error(t):
    print("Syntax error at '%s'" % t.value)

def p_statement_block(p):
    """statement : block"""
    p[0] = p[1]

def p_statement_newline(p):
    "statement : NL"

def p_statement_start_subckt(p):
    "statement : SUBCKT NAME port_list"
    global current_subckt
    if current_subckt is not None:
        raise Exception("No nested subckts!")
    if p[2] in subckts:
        raise Exception("Redeclaration of subckt '%s'" % p[2])
    current_subckt = p[2]
    ports = p[3]
    nets = {}
    inputs = {}
    outputs = {}
    input_idx = 0
    output_idx = 0
    for i, (pname, pnet, poutput) in enumerate(ports):
        #net = nets.get(pnet, (None, None))
        if not poutput: # Flopped from normal
            if pname is None:
                pname = output_idx
            outputs[pname] = pnet
            outputs[output_idx] = pnet
            output_idx += 1
        else:
            if pname is None:
                pname = input_idx
            inputs[pname] = pnet
            inputs[input_idx] = pnet
            input_idx += 1
        #nets[pnet] = net
    subckts[current_subckt] = {
        "blocks": {},
        "nets": nets,
        "inputs": inputs,
        "outputs": outputs,
    }
    print "SUBCKT:", subckts[current_subckt]

def p_statement_end_subckt(p):
    "statement : ENDS"
    global current_subckt
    if current_subckt is None:
        raise Exception(".ends without .subckt")
    current_subckt = None

def p_netlist_single(p):
    "netlist : statement"
    p[0] = [p[1]]

def p_netlist(p):
    "netlist : netlist statement"
    p[0] = p[1] + [p[2]]

yacc.yacc(start="netlist")

text = open("example_netlist.net").read()

yacc.parse(text)

#print nets
#print blocks

print json.dumps(global_nets, indent=4)

for net_name, (outp, inp) in global_nets.items():
    if inp is None:
        #raise Exception("Net '%s' does not have an input" % net_name)
        print("WARNING: Net '%s' does not have an input" % net_name)
        continue
    if outp is None:
        #raise Exception("Net '%s' does not have an output" % net_name)
        print("WARNING: Net '%s' does not have an output" % net_name)
        continue

    in_block, in_port = inp
    out_block, out_port = outp

    block = global_blocks[in_block]
    #if not block["is_subckt"]:
    block["inputs"].append({ #(in_port, out_block, out_port))
        "input_port": in_port,
        "output_block": out_block,
        "output_port": out_port
    })
        
outf = open("output.json", "w")
json.dump({"blocks": global_blocks, "nets": global_nets}, outf, indent=4)
