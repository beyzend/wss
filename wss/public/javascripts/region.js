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
    
    function getPosition(id) {
        return this.regionData[id];
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
    
    function render(cameraPosition, thisRegion) {
        // Get player position
        this.cameraPosition = cameraPosition;
        // I'm not sure how most efficiently to upate the sprites right now.
        $.each(this.regionData, transformSprite.bind(this));
    }
    
    return {
      init: init,
      updatePositionRelative: updatePositionRelative,
      updatePosition: updatePosition,
      updateEntityPositions: updateEntityPositions,
      render: render,
      getPosition: getPosition  
    };
  
});
