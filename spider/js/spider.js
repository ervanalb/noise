var BASE_URL = "";
var block_types = {};
var type_types = {};

// Blocks
var Views = {};

var Block = Backbone.Model.extend({
    idAttribute: "block_id",
    defaults: {
        /*
        inputs: [
            {name: "A", type: "int"},
        ],
        outputs: [
            {name: "X", type: "int"},
        ],
        */
        "block_class": "WaveBlock",
        name: "Block",
        x: 150,
        y: 150,
        args: [],
        kwargs: {},
        view: "BlockView"
    },
    initialize: function(options){
        var model = this;
        var bclass = block_types[this.get('block_class')];
        console.log('nm', this.get('name'));
        //this.set("name", this.get("block_class"));
        if(!bclass){
            console.log(this);
            throw "Unknown class: " + this.get('block_class');
        }
        if(!this.has('inputs')){
            var names = this.get('input_names') || bclass.input_names;
            var num = this.get('num_inputs') || bclass.num_inputs;
            this.set('inputs', _.map(_.range(num), function(i){
                return {name: names[i] || "Input", type: "int"};
            }));
            this.set('input_names', names);
            this.set('num_inputs', num);
        }
        if(!this.has('outputs')){
            var names = this.get('output_names') || bclass.output_names;
            var num = this.get('num_outputs') || bclass.num_outputs;
            this.set('outputs', _.map(_.range(num), function(i){
                return {name: names[i] || "Output", type: "int"};
            }));
            this.set('output_names', names);
            this.set('num_outputs', num);
        }
    },
    toJSON: function(options){
        return this.pick('block_class', 'x', 'y', 'name', 'args', 'kwargs', 'data');
    }
});


var Blocks = Backbone.Collection.extend({
    model: Block,
    url: BASE_URL + "/blocks",
});
var blocks = new Blocks();

var Connection = Backbone.Model.extend({
    idAttribute: "connection_id",
    defaults: {
    },
    initialize: function(data, options){
        var options = options || {};
        if(!options.fromPlumb){
            source = blocks.get(this.get('source_id'));
            target = blocks.get(this.get('target_id'));
            console.log(source, target, this.attributes);
            source_endpoint = source.get('output_endpoints')[this.get('source_idx')];
            target_endpoint = target.get('input_endpoints')[this.get('target_idx')];

            var jsConn = jsPlumb.connect({source: source_endpoint,target: target_endpoint}, {parameters: {id: this.id }});
            this.set('jsConn', jsConn);
        }else{
            this.save();
        }
    },
    toJSON: function(options){
        return this.pick('source_id', 'source_idx',
                         'target_id', 'target_idx',
                         'connection_id');
    }
});

var Connections = Backbone.Collection.extend({
    model: Connection,
    url: BASE_URL + "/connections"
});
var connections = new Connections();

var BlockView = Backbone.View.extend({
    tagName: "div",
    className: "block-base",
    initialize: function(options){
        this.$el.appendTo($("div.block-container"));
        this.listenTo(this.model, 'change', this.render);
        this.$el.attr('id', this.model.id);
        //this.$el.draggable({handle: "div.block-header"});
        this.setupPlumb();
        this.render();
    },
    stopDrag: function(ev){
        console.log('drag', this, ev);
        var pos = this.$el.offset();
        this.model.set('x', pos.left);
        this.model.set('y', pos.top);
        this.model.save();
    },
    setupPlumb: function(){
        var view = this;
        jsPlumb.draggable(this.$el, {
            //handle: "div.block-header",
            stop: _.bind(this.stopDrag, this)
        });
        var inSize = 0.8 / (this.model.get('inputs').length );
        var outSize = 0.8 / (this.model.get('outputs').length );
        this.model.set('input_endpoints', _.map(this.model.get('inputs'), function(inp, i){
            return jsPlumb.addEndpoint(view.$el, 
                {anchor: [0, inSize * (i + 0.5) + 0.1, -1, 0]},
                {
                    isSource: false, 
                    isTarget: true,
                    parameters: {
                        target_idx: i,
                        target_param: inp,
                        target_block: view.model
                    }
                });
        }));
        this.model.set('output_endpoints', _.map(this.model.get('outputs'), function(outp, i){
            return jsPlumb.addEndpoint(view.$el, 
                {anchor: [1, outSize * (i + 0.5) + 0.1, 1, 0]},
                {
                    isSource: true, 
                    isTarget: false,
                    parameters: {
                        source_idx: i,
                        source_param: outp,
                        source_block: view.model
                    }
                });
        }));

    },
    events: {
        "click .block-close": "destroy",
    },
    destory: function(ev){
        this.model.destroy();
        this.remove();
    },
    render: function(){
        var view = this;
        this.$divBox = $("<div>");
        this.$divHeader = $("<div>").appendTo(this.$divBox).addClass("block-header");
        this.$txtName = $("<input type='text'>").appendTo(this.$divHeader).val(this.model.get('name'));

        this.$inputUl = $("<ul>").appendTo(this.$divBox).addClass("block-inputs");
        this.$outputUl = $("<ul>").appendTo(this.$divBox).addClass("block-outputs");

        this.$inpLis = _.map(this.model.get('inputs'), function(inp){
            var $inpLi =  $("<li>").appendTo(view.$inputUl).text(inp.name);
            return $inpLi;
        });
        this.$outpLis = _.map(this.model.get('outputs'), function(outp){
            var $outpLi =  $("<li>").appendTo(view.$outputUl).text(outp.name);
            return $outpLi;
        });

        this.$txtData = $("<textarea>").appendTo(this.$divBox).val(JSON.stringify(this.model.get('data')));

        this.$divFooter = $("<div>").appendTo(this.$divBox).addClass("block-footer").text("Ready");
        this.$aClose = $("<a>").appendTo(this.$divFooter).addClass("block-close ").html("&times;");

        this.$el.empty()
        this.$el.offset({left: this.model.get('x'), top: this.model.get('y')});
        this.$el.append(this.$divBox);
        this.renderPipes();
        jsPlumb.repaint(this.$el);
        this.delegateEvents();

        this.$txtName.change(function(ev){
            view.model.set('name', $(this).val());
            view.model.save();
            console.log(view.model.get('name'));
        });

        this.$txtData.change(function(ev){
            view.model.set('data', JSON.parse($(this).val()));
            view.model.save();
        });
        return this;
    },
    renderPipes: function(){
        var view = this;
    }
});
Views.BlockView = BlockView;

var BlocksView = Backbone.View.extend({
    initialize: function(options){
        this.listenTo(this.collection, 'add', this.addOne);
        this.listenTo(this.collection, 'reset', this.addAll);
        //this.listenTo(this.collection, 'all', this.render);
    },
    addOne: function(block) {
        console.log(block.attributes.view);
        var view = new (Views[block.get('view')])({model: block});
        console.log(view);
        this.$el.append(view.render().el);
    },
    addAll: function(block){
        this.collection.each(this.addOne, this);
    }
});

var blocksView = new BlocksView({el: "div.block-container", collection: blocks});

var ConnectionsView = Backbone.View.extend({
    initialize: function(options){
        this.listenTo(connections, 'add', this.addOne);
        this.listenTo(connections, 'remove', this.rmOne);
        this.listenTo(connections, 'reset', this.addAll);
    },
    addOne: function(conn) {
        //XXX
    },
    rmOne: function(conn) {
        console.log('rm', conn);
        jsPlumb.detach(this.model.get('jsConn'));
    },
    addAll: function(block){
        this.collection.each(this.addOne, this);
    }
});

var connectionsView = new ConnectionsView();


// Canvas
jsPlumb.setContainer($("body"));
jsPlumb.ready(function() {

}); 

jsPlumb.bind("connection", function(info, ev){
    // Called even after connnectionMoved
    console.log("conn", info);
    var params = info.connection.getParameters();
    if(_.isUndefined(params.id)){
        var conn = connections.add({
            "source_id": params.source_block.id,
            "target_id": params.target_block.id,
            "source_idx": params.source_idx,
            "target_idx": params.target_idx,
            "jsConn": info.connection
        }, {fromPlumb: true});
    }
});

jsPlumb.bind("connectionMoved", function(info, ev){
    console.log("conn moved", info);
});

jsPlumb.bind("connectionDetached", function(info, ev){
    // Called even if the connection didn't exist
    console.log("conn detached", info);
    var params = info.connection.getParameters();
    console.log("detached", params);
    if(!_.isUndefined(params.id)){
        console.log('deleting', params.id);
        var connection = connections.get(params.id);
        connection.destroy();
    }
});

// Create new blocks
BLOCK_FNS = {
    "Constant<double>": {block_class: "ConstantBlock", args: [{"__type__": "double", args: [0]}], view: BlockView},
    "Constant<int>": {block_class: "ConstantBlock", args: [{"__type__": "int", args: [0]}], view: BlockView},
    "Accumulator": {block_class: "AccumulatorBlock", args: [], view: BlockView},
    "FunctionGenerator": {block_class: "FunctionGeneratorBlock", args: [], view: BlockView},
    "Sequencer": {block_class: "SequencerBlock", args: [], view: BlockView},
    //"Convolve<10>": {block_class: "ConvolveBlock", args: [10]}]},
    "Plus": {block_class: "PlusBlock", args: [], view: BlockView},
    "Multiply": {block_class: "MultiplyBlock", args: [], view: BlockView},
    "Wave": {block_class: "WaveBlock", args: [], view: BlockView},
    "Tee<double>": {block_class: "TeeBlock", args: [{"__type__": "double", args: [0]}], view: BlockView},
    "Wye<double>": {block_class: "WyeBlock", args: [{"__type__": "double", args: [0]}], view: BlockView},
    "Mixer<4>": {block_class: "MixerBlock", args: [4], view: BlockView, options: {num_inputs: 8}},
}
var setupBlockBtns = function(){
    var createBlock = function(bc, name, bargs){
        var args = bargs.args;
        var view = bargs.view;
        var opt = bargs.options || {};
        b = blocks.add(_.extend(opt, {block_class: bc, name: name, args: args, viewclass: view}));
        b.save();
    }
    _.map(BLOCK_FNS, function(bargs, bname){
        if(_.has(block_types, bargs.block_class)){
            var button = $("<button>").text(bname).click(function(){
                createBlock(bargs.block_class, bname, bargs);
            });
            $(".new-blocks").append(button);
        }else{
            console.warn("Server did not report block type '%s'", bname);
        }
    });
}

// Sync
var sync = function(){
    return $.get(BASE_URL + "/status", function(response){
        block_types = _.indexBy(response.blocks, 'class');
        type_types = _.indexBy(response.types, 'class');

        setupBlockBtns();

        //blocks.set(response.block_instances);
        //connections.set(response.connection_instances);
        blocks.fetch();
        connections.fetch();
        
    });
}
