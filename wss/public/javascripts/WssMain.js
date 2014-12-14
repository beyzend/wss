// void ComputeBlockXY(float uvX, out float2 blockCoords)
// {
// float idx = uvX * 256.0 - 1;
// blockCoords.y = floor(idx / 16.0);
// blockCoords.x = (idx - blockCoords.y * 16.0);
// }

var SCREEN_WIDTH = 640;
var SCREEN_HEIGHT = 480;

require(["/javascripts/map.js", "/javascripts/region.js"], function(Map, Region) {

    $.data["map"] = Map;
    $.data["region"] = Region;


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
        
        
        
        socket.on('map', function(data) {
            // pretend to load map right now. We need to manually load data.map.
            console.log(data.map);
            var mapGroup = $("#mapTiles");
            var map = $.data["map"];
            map.addTilemapToGroup(mapGroup, "testmap", data.map);
            
            // pre-allocate entities
            var entityGroup = $("#entities");
            var regionData = {};
            // Initialize player to initial position (in world-space).
            var playerPosition = [data.map.width * data.map.tilewidth / 2,
                 data.map.height * data.map.tileheight / 2];
            regionData[-1] = playerPosition;
            addSprite(entityGroup, -1, playerPosition, playerAnimation);
            
            //Hack: we want to pool sprites & entities so we pre-allocate them in someway before 
            //we receive messages from server. Now just assume server ids are from 0 to 99 and just preallocate
            //then now.
            for (i = 0; i < 100; ++i) {
                var position = [0, 0];
                regionData[i] = position;
                addSprite(entityGroup, i, position, playerAnimation);
            }
            
            var region = $.data['region'];
            region.init(regionData);
            
            // Setup 
            $(document).keydown(function(e) {

                var region = $.data["region"];
                
                switch ( e.keyCode ) {
                case 65:
                    // this is left
                    region.updatePositionRelative(-1, [-1, 0]);    
                    break;
                case 87:
                    // this is up
                    region.updatePositionRelative(-1, [0, -1]);
                    break;
                case 68:
                    // this is right
                    region.updatePositionRelative(-1, [1, 0]);
                    break;
                case 83:
                    // this is down
                    region.updatePositionRelative(-1, [0, 1]);
                    break;
                }
            });

        });

        socket.on('regionData', function(data) {
            var region = $.data["region"];
            // synethetic play with data.
            $.each(data.positions, region.updateEntityPositions.bind(region));
            
            // update sprites
            var sprites = $("#entities").children("div");
            region.updateSprites([SCREEN_WIDTH/2.0, SCREEN_HEIGHT/2.0]);     
       
        });

        console.log("regionModule.entityState");
    });

});
