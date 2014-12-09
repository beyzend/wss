
// void ComputeBlockXY(float uvX, out float2 blockCoords)
// {
  // float idx = uvX * 256.0 - 1;
  // blockCoords.y = floor(idx / 16.0);
  // blockCoords.x = (idx - blockCoords.y * 16.0);
// }

function Entity( id , animation ) {
    this.id = id;
    this.animation = animation;
}

var AREA_WIDTH = 640;
var AREA_HEIGHT = 480;

function loadMap( jsonMap ) {
    
    var tileAnims = [];
    
    for ( y = 0; y < 16; ++y) {
        for ( x = 0; x < 16; ++x) {
            tileAnims.push(new $.gQ.Animation({
                imageURL : "images/art/ironhand.png",
                numberOfFrame : 1,
                delta : 18,
                distance : 0,
                type : $.gQ.ANIMATION_MULTI | $.gQ.ANIMATION_HORIZONTAL | $.gQ.ANIMATION_ONCE,
                offsetx : x * 18,
                offsety : y * 18
            }));
        }
    }

    var idx = 0;
    
    var tileDef = Array.apply(null, {
        length : 100
    }).map(function() {
        return Array.apply(null, {
            length : 100
        }).map(function() {
            return idx++ % 5;
        });
    });
    
    mapTiles.addTilemap("map", tileDef, tileAnims, {
         width : 18,
         height : 18,
         sizex : 100,
         sizey : 100
    }).end();
    
}

require([], function(){
    
    var playerAnimation = new $.gQ.Animation({
        imageURL : "images/art/ironhand.png",
        numberOfFrame : 1,
        delta : 18,
        distance : 0,
        type : $.gQ.ANIMATION_MULTI | $.gQ.ANIMATION_HORIZONTAL | $.gQ.ANIMATION_ONCE,
        offsetx : 1 * 18,
        offsety : 0 * 18
    });
    
    var playerEntity = new Entity( 1, playerAnimation );
    
   $("#gamescreen").playground({
        width : AREA_WIDTH,
        height : AREA_HEIGHT,
        refreshRate : 60,
        keyTracker: true
   })
   .addGroup( "mapTiles", { width:640, height:480} )
     .importTilemaps( "images/art/test.json" )
   .end()  
   .addGroup( "player", {posx: AREA_WIDTH / 2, posy: AREA_HEIGHT / 2, width: 18, height: 18 } )
        .addSprite( "player", { animation: playerAnimation , width: 18, height: 18 } )
   .end();
   
    var socket = io.connect( 'http://localhost:3000' );
    socket.on( 'news', function( data ) {
        console.log( data );
        socket.emit( 'my other event', { my: 'data' });
    });
    var mapTiles = $( "#mapTiles" );
    socket.on( 'map', function( data ) {
        // pretend to load map right now. We need to manually load data.map.
        console.log( data.map );
             
    });
  
    $( document ).keydown( function( e ) {
       
       var map = $( "#mapTiles" );   
       
       switch ( e.keyCode ) {
           case 65: // this is left
            map.x( 1, true );
            break;
           case 87: // this is up
            map.y( 1 , true );
            break;
           case 68: // this is right
            map.x( -1, true );
            console.log("going right!");
            break;
           case 83: // this is down
            map.y( -1, true );
            break;
       } 
    });

    $.playground().startGame(function() {
    });
});
