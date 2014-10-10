var BASE_URL = "http://localhost:5000";
var block_types = {};
var type_types = {};

// Blocks
var Connection = Backbone.Model.extend({
    idAttribute: "id",
    defaults: {
        id: null
    }
});

var Connections = Backbone.Collection.extend({
    model: Connection
});

var Block = Backbone.Model.extend({
    idAttribute: "id",
    defaults: {
        /*
        inputs: [
            {name: "A", type: "int"},
        ],
        outputs: [
            {name: "X", type: "int"},
        ],
        */
        name: "Block",
    },
    initialize: function(options){
        var bclass = block_types[this.get('class')];
        if(!bclass){
            console.log(this);
            throw "Unknown class: " + this.get('class');
        }
        if(!this.has('inputs')){
            this.set('inputs', _.map(_.range(bclass.num_inputs), function(i){
                return {name: "Input " + i, type: "int"};
            }));
        }
        if(!this.has('outputs')){
            this.set('outputs', _.map(_.range(bclass.num_outputs), function(i){
                return {name: "Output " + i, type: "int"};
            }));
        }
    },
    create: function(){
        var block = this; 
        $.post(BASE_URL + "/new/block", {
            "class": "WaveBlock"
        }, function(response){
            console.log("Created block:", response);
            block.set('id', parseInt(response.id));
        });
    },
});


var Blocks = Backbone.Collection.extend({
    model: Block,
});

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
    setupPlumb: function(){
        var view = this;
        jsPlumb.draggable(this.$el, {handle: "div.block-header"});
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
    render: function(){
        var view = this;
        this.$divBox = $("<div>");
        this.$divHeader = $("<div>").appendTo(this.$divBox).addClass("block-header").text(this.model.get('name'));

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

        this.$divFooter = $("<div>").appendTo(this.$divBox).addClass("block-footer").text("Ready");

        this.$el.empty().append(this.$divBox);
        this.renderPipes();
        return this;
    },
    renderPipes: function(){
        var view = this;
    }
});

var BlocksView = Backbone.View.extend({
    initialize: function(options){
        this.listenTo(this.collection, 'add', this.addOne);
        this.listenTo(this.collection, 'reset', this.addAll);
        //this.listenTo(this.collection, 'all', this.render);
    },
    addOne: function(block) {
        var view = new BlockView({model: block});
        console.log(view);
        this.$el.append(view.render().el);
    },
    addAll: function(block){
        this.collection.each(this.addOne, this);
    }
});

var connections = new Connections();
var blocks = new Blocks();
var blocksView = new BlocksView({el: "div.block-container", collection: blocks});

var ConnectionsView = Backbone.View.extend({
    initialize: function(options){
        this.listenTo(connections, 'add', this.addOne);
        this.listenTo(connections, 'remove', this.rmOne);
        this.listenTo(connections, 'reset', this.addAll);
    },
    addOne: function(conn) {
        info = conn.get('conn');
        console.log('add', info);
        source = blocks.get(info.source_id);
        target = blocks.get(info.target_id);
        //console.log(blocks, info.source_id, source);
        source_endpoint = source.get('output_endpoints')[info.source_idx];
        target_endpoint = target.get('input_endpoints')[info.target_idx];
        var jsConn = jsPlumb.connect({source: source_endpoint,target: target_endpoint}, {parameters: {id: conn.id }});
        conn.set('jsConn', jsConn);
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
        $.post(BASE_URL + "/new/connection", {
            "source_block_id": params.source_block.id,
            "target_block_id": params.target_block.id,
            "source_idx": params.source_idx,
            "target_idx": params.target_idx
        }, function(response){
            info.connection.setParameter('id', parseInt(response.id));
        });
    }
});

jsPlumb.bind("connectionMoved", function(info, ev){
    console.log("conn moved", info);
});

jsPlumb.bind("connectionDetached", function(info, ev){
    // Called even if the connection didn't exist
    console.log("conn detached", info);
    var params = info.connection.getParameters();
    if(!_.isUndefined(params.id)){
        console.log('deleting', params.id);
        $.post(BASE_URL + "/delete/connection", {
            "id": params.id
        }, function(response){
            console.log(response);
        });
    }
});

// Sync
var sync = function(){
    $.get(BASE_URL + "/status", function(response){
        block_types = _.indexBy(response.blocks, 'class');
        type_types = _.indexBy(response.types, 'class');
        blocks.set(response.block_instances);
        connections.set(response.connection_instances);
    });
}
