var express = require( 'express' );
var router = express.Router();

/* This method will return Get a map. 
 *
 * */
router.get( 'map', function( req, res) {
    // respond with a map resource
    res.send( '' )
    
});

module.exports = router;
