var BASE_URL = "http://localhost:5000";

// Blocks
var Connection = Backbone.Model.extend({
    defaults: {
        id: null
    }
});

var Connections = Backbone.Collection.extend({
    model: Connection
});

var Block = Backbone.Model.extend({
    defaults: {
        inputs: [
            {name: "A", type: "int"},
        ],
        outputs: [
            {name: "X", type: "int"},
        ],
        name: "Block",
        sid: null
    },
    create: function(){
        var block = this; 
        $.post(BASE_URL + "/new/block", {
            "class": "WaveBlock"
        }, function(response){
            console.log("Created block:", response);
            block.set('sid', parseInt(response.id));
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
        _.each(this.model.get('inputs'), function(inp, i){
            jsPlumb.addEndpoint(view.$el, 
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
        });
        _.each(this.model.get('outputs'), function(outp, i){
            jsPlumb.addEndpoint(view.$el, 
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
        });

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

var connections = new Connections();
var blocks = new Connections();

// Canvas
jsPlumb.setContainer($("body"));
jsPlumb.ready(function() {

}); 

jsPlumb.bind("connection", function(info, ev){
    // Called even after connnectionMoved
    console.log("conn", info);
    var params = info.connection.getParameters();
    $.post(BASE_URL + "/new/connection", {
        "source_block_id": params.source_block.get('sid'),
        "target_block_id": params.target_block.get('sid'),
        "source_idx": params.source_idx,
        "target_idx": params.target_idx
    }, function(response){
        info.connection.setParameter('id', parseInt(response.id));
    });
});

jsPlumb.bind("connectionMoved", function(info, ev){
    console.log("conn moved", info);
});

jsPlumb.bind("connectionDetached", function(info, ev){
    // Called even if the connection didn't exist
    console.log("conn detached", info);
    var params = info.connection.getParameters();
    if(params.id){
        $.post(BASE_URL + "/delete/connection", {
            "id": params.id
        }, function(response){
            console.log(response);
        });
    }
});

var block_types = {};
var type_types = {};

// Sync
var setupSync = function(){
    $.get(BASE_URL + "/", function(response){
        block_types = response.blocks;
        type_types = response.types;
    });
}
