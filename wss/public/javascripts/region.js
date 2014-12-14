define(function() {
    this.regionData = {},
    this.cameraPosition = [];
    
    function updatePositionRelative(id, relPosition) {
        var currentPosition = this.regionData[id];
        currentPosition[0] += relPosition[0];
        currentPosition[1] += relPosition[1];
        this.regionData[id] = currentPosition;
    }
    
    function updatePosition(id, position) {
        this.regionData[id] = position;
    }
    
    function init(regionData) {
        this.regionData = regionData;
    }
    
    function updateEntityPositions(index, entityPosition) {
        this.regionData[entityPosition.id] = [entityPosition.x, entityPosition.y];
    };
    
    function transformSprite(key, position) {
        var sprite = $("#"+key);
        //console.log("sprite: " + sprite);
        //console.log("position: " + position);
        var x = position[0] + this.cameraPosition[0];
        var y = position[1] + this.cameraPosition[1];
        //console.log("camera x, y: " + x + ", " + y);
        sprite.x(x); // transform into camera space.
        sprite.y(y);
    }
    
    function updateSprites(cameraPosition) {
        // Get player position
        var playerPosition = this.regionData[-1];    
        this.cameraPosition = [-playerPosition[0] + cameraPosition[0], -playerPosition[1] + cameraPosition[1]];
        // I'm not sure how most efficiently to upate the sprites right now.
        $.each(this.regionData, transformSprite.bind(this));
        
        var mapGroup = $("#mapTiles");
        
        //update map
        mapGroup.x(this.cameraPosition[0]);
        mapGroup.y(this.cameraPosition[1]);
        
    }
    
    return {
      init: init,
      updatePositionRelative: updatePositionRelative,
      updatePosition: updatePosition,
      updateEntityPositions: updateEntityPositions,
      updateSprites: updateSprites  
    };
  
});
