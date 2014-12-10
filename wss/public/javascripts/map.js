/**
 * @author beyzend
 */
define( function () {
   
   // gets Tile Animations 
   function getTileAnimations( tileset ) {
       
       var tileAnims = [];
       var atlasHeight = tileset.imageheight / tileset.tileheight;
       var atlasWidth = tileset.imagewidth / tileset.tilewidth;
       
       for ( y = 0; y < atlasHeight; ++y ) {
           for ( x = 0; x < atlasWidth; ++x ) {
               tileAnims.push( new $.gQ.Animation( {
                   imageURL : tileset.image,
                   numOfFrame : 1,
                   delta : tileset.tilewidth,
                   distance : 0,
                   type : $.gQ.ANIMATION_MULTI | $.gQ.ANIMATION_HORIZONTAL | $.gQ.ANIMATION_ONCE,
                   offsetx : x * tileset.tilewidth,
                   offsety : y * tileset.tileheight
               }));
           }
       }
       return tileAnims;
   }
   
   function getLayerDefsFromLayer( layer ) {
       
       var layerDefs = [];
       
       for ( y = 0; y < layer.height; ++y ) {
           var row = [];
           for ( x = 0; x < layer.width; ++x ) {
               row.push( layer.data[ y * layer.width + x ] );
           }
           layerDefs.push( row );
       }
       return layerDefs;
   }
   
   function addTilemap( mapGroup, mapName, jsonMap ) {
       var tileAnims = getTileAnimations( jsonMap.tilesets[ 0 ] );
       var layerDefs = getLayerDefsFromLayer( jsonMap.layers[ 0 ] );
       
       mapGroup.addTilemap( mapName, layerDefs, tileAnims, {
           width : jsonMap.tilewidth,
           height : jsonMap.tileheight,
           sizex : jsonMap.width,
           sizey : jsonMap.height
       });
   }
   
   return {
       addTilemapToGroup : addTilemap
   }; 
});
