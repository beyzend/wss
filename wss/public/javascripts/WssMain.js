$(function() {
  
    var tileAnims = [];
    // Figure out something cute and functional tomorrow here.
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

    var anims = [];

    $("#gamescreen").playground({
        width : 640,
        height : 480,
        refreshRate : 60,
        keyTracker: true
    })
    //.addGroup( "mapTiles", {width:640, height:480} )
    //.importTilemaps( "images/art/test.json" );
    
    .addTilemap("mapTiles", tileDef, tileAnims, {
        width : 18,
        height : 18,
        sizex : 100,
        sizey : 100
    });
    

    

    $( document ).keydown( function( e ) {
       
       var map = $( "#mapTiles" );   
       
       switch ( e.keyCode ) {
           case 65: // this is left
            map.x( -1, true );
            break;
           case 87: // this is up
            map.y( -1 , true );
            break;
           case 68: // this is right
            map.x( 1, true );
            console.log("going right!");
            break;
           case 83: // this is down
            map.y( 1, true );
            break;
       } 
    });

    $.playground().startGame(function() {
    });
}); 

