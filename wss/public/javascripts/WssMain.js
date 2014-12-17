
var SCREEN_WIDTH = 640;
var SCREEN_HEIGHT = 480;

require(["/javascripts/map.js", "/javascripts/region.js"], function(Map, Region) {

    $.data["map"] = Map;
    $.data["region"] = Region;
    $.data["playerId"] = 0;

    playerAnimation = new $.gQ.Animation({
        imageURL : "images/art/ironhand.png",
        numberOfFrame : 1,
        delta : 18,
        distance : 0,
        type : $.gQ.ANIMATION_MULTI | $.gQ.ANIMATION_HORIZONTAL | $.gQ.ANIMATION_ONCE,
        offsetx : 1 * 18,
        offsety : 0 * 18
    });

    

    $("#gamescreen").playground({
        width : SCREEN_WIDTH,
        height : SCREEN_HEIGHT,
        refreshRate : 60,
        keyTracker : true
    }).addGroup("mapTiles", {
        width : SCREEN_WIDTH,
        height : SCREEN_HEIGHT
    })
    //.importTilemaps( "images/art/test.json" )
    .end()
    .addGroup("entities", {
        posx : 0, 
        posy : 0, 
        width : SCREEN_WIDTH,
        height : SCREEN_HEIGHT
    }).end();
    /*
    .addSprite("player", {
        animation : playerAnimation,
        width : 18,
        height : 18
    }).end();
    */
    

    $.playground().registerCallback(function() {
    }, 30);

    $.playground().startGame(function() {
        var socket = io.connect('http://localhost:3000');
        socket.on('news', function(data) {
            console.log(data);
            socket.emit('my other event', {
                my : 'data'
            });
        });
        
        function addSprite(group, id, position, animation) {
            group.addSprite(id, {
               animation: animation,
               width: 18,
               height: 18,
               posx: position[0],
               posy: position[1] 
            });
        }
        
        function getCameraPosition(playerId, region) {
            var playerPosition = region.getPosition(playerId);
            return [-playerPosition[0] + SCREEN_WIDTH/2.0, -playerPosition[1] + SCREEN_HEIGHT/2.0];
        }
        
        socket.on('map', function(data) {
            
            // pretend to load map right now. We need to manually load data.map.
            console.log(data.map);
            var mapGroup = $("#mapTiles");
            var map = $.data["map"];
            map.addTilemapToGroup(mapGroup, "testmap", data.map);
            
            // pre-allocate entities
            var entityGroup = $("#entities");
            var regionData = {};
            
            
            // synthetic add player event for now
            socket.emit('AddEntityEvent', {
               type: 1 
            });
            
            socket.on("EntityAddedEvent", function(data) {
                var region = $.data['region'];
                console.log("EntityAddedEvent data: " + data.id);
                
                // Initialize player to initial position (in world-space).
                $.data["playerId"] = data.id;
                addSprite(entityGroup, playerId, [0, 0], playerAnimation);
                
                //Hack: we want to pool sprites & entities so we pre-allocate them in someway before 
                //we receive messages from server. Now just assume server ids are from 0 to 99 and just preallocate
                //then now.
                for (i = 0; i < 200; ++i) {
                    var state = [0, 0, false];
                    regionData[i] = state;
                    addSprite(entityGroup, i, state, playerAnimation);
                }
            
                var region = $.data['region'];
                region.init(regionData);
                
                socket.on('regionData', function(data) {
                    var region = $.data["region"];
       
                    $.each(data.positions, region.updateEntityPositions.bind(region));
                    region.render(getCameraPosition($.data["playerId"], region), region);
                    
                    // render the map
                    var cameraPosition = getCameraPosition($.data["playerId"], region);
                    
                    var mapGroup = $("#mapTiles");
                    mapGroup.render(cameraPosition, mapGroup);
                    
                });
            
                
            });
            
            $.fn.render = function(cameraPosition, mapGroup) {
                mapGroup.x(cameraPosition[0]);
                mapGroup.y(cameraPosition[1]);
            };
            
            var down = {};
          
            // Setup 
            $(document).keydown(function(e) {

                var region = $.data["region"];
                
                if (down[e.keyCode])
                    return;
                
                switch ( e.keyCode ) {
                case 65:
                    // this is left
                    region.updatePositionRelative(-1, [-1, 0]);
                    down[e.keyCode] = true;    
                    break;
                case 87:
                    // this is up
                    region.updatePositionRelative(-1, [0, -1]);
                    down[e.keyCode] = true;
                    break;
                case 68:
                    // this is right
                    region.updatePositionRelative(-1, [1, 0]);
                    down[e.keyCode] = true;
                    break;
                case 83:
                    // this is down
                    region.updatePositionRelative(-1, [0, 1]);
                    down[e.keyCode] = true;
                    break;
                }
                var playerId = $.data["playerId"];s
                var playerPosition = region.getPosition(playerId);
                var mapGroup = $("#mapTiles");
                var cameraPosition = getCameraPosition(playerId, region);
                
                mapGroup.render(cameraPosition, mapGroup);
                region.render(cameraPosition, region);
                
            });
            
            $(document).keyup(function(e) {
               down[e.keyCode] = false; 
            });

        });
        console.log("regionModule.entityState");
    });

});
