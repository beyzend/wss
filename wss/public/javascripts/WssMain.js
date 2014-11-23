var onInit = function() {
   
    var tileAnims = [];
    // Figure out something cute and functional tomorrow here.
    for ( y = 0; y < 16; ++y )
    {
        for( x = 0; x < 16; ++x )
        {
            tileAnims.push( new $.gQ.Animation( {
                imageURL: "images/art/ironhand.png",
                numberOfFrame: 1,
                delta: 18,
                distance: 0,
                type: $.gQ.ANIMATION_MULTI | $.gQ.ANIMATION_HORIZONTAL | $.gQ.ANIMATION_ONCE,
                offsetx: x * 18,
                offsety: y * 18
            }));
        }
    }
    
    var tileDef = Array.apply( null, {length:25}).map( function(){
       return Array.apply( null, {length:25} ).map( function() {
           return 2;
       });
    });
    
    
    var anims = [];
    
    $("#gamescreen").playground({
        width: 640,
        height: 480,
        refreshRate: 60
    })
    //.importTilemaps( "images/art/test.json" );
    .addTilemap( "basicTile", tileDef, tileAnims,
        {
           width: 18,
           height: 18,
           sizex: 25,
           sizey: 25 
        });
    
    $.playground().startGame( function(){});
    
  
    
};
