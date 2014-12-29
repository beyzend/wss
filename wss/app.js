var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

var fs = require("fs"), json;

var routes = require('./routes/index');
var users = require('./routes/users');

var app = express();

var debug = require('debug')('wss');
app.set('port', process.env.PORT || 3000);

var server = app.listen(3000, function() {
    debug('Express server listening on port ' + server.address().port);
});

// Setup ZMQ
var zmq = require('zmq');
    
var subscriber = zmq.socket('sub');
    
console.log("Subscribing to region data");
subscriber.subscribe("");
subscriber.connect("tcp://127.0.0.1:4200");

var requester = zmq.socket('req');
console.log("Connecting to region requester");

// Setup socket
var io = require('socket.io')(server);
var total_temp = 0;
var temps = 0;

//io.set("log level", 3);

    
io.on('connection', function(socket) {
   
   //socket.set("log level", 3);
   
    socket.emit('news', {
        hello : 'Hell World from node.js socket.io server'
    });
    socket.on('my other event', function(data) {
        console.log(data);
    });

    function readJsonFileSync(filepath, encoding) {
        if ( typeof (encoding ) == 'undefined') {
            encoding = 'utf8';
        }
        var file = fs.readFileSync(filepath, encoding);
        return JSON.parse(file);
    }

    function loadTestMapSync(filePath, encoding) {
        var file = fs.readFileSync(filePath, ( typeof (encoding ) === 'undefined' ? 'utf8' : encoding ));
        return JSON.parse(file);
    }


    socket.emit('map', {
        map : loadTestMapSync(__dirname + '/public/images/art/test.json')
    });
    
    
    
    subscriber.on('message', function(data) {
       // Send the data off.
       var jsonData = JSON.parse(data.toString());
       socket.emit('regionData', jsonData);
    });
    
    //requester.connect("tcp://localhost:4201");
    
    requester.on('message', function(data) {
        // Received a reply.
        
        var response = JSON.parse(data);
        console.log("request message data" + data);
        console.log("request message received!\n" + response.id);
        if (response.type === 1) {
            socket.emit('EntityAddedEvent', {
               id: response.id 
            });
        }
        else if (response.type === 2) {
            // console.log("RemoveEntity response received!");
        }
    });
    // Since both client and server is javascript we can just define 
    //compatible Objects.
    socket.on('AddEntityEvent', function(data) {
        
        //var response = JSON.parse(data);
        console.log("request add entity from client!");
        socket.emit('EntityAddedEvent', {
           id: 0 
        });
        
        // console.log("AddEntityEvent")
        // var addEntityRequest = {
            // "type": 1,
            // "data": {
//                 
            // }
        // };
//         
        // requester.send(JSON.stringify(addEntityRequest)); 
    });
    
    socket.on('RemoveEntityEvent', function(data) {
        var removeEntityRequest = {
            "type": 2,
            "data": {
                "id": data.id
            }
        };
        
        requester.send(JSON.stringify(removeEntityRequest));
    });
     
});



// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
//app.use(favicon(__dirname + '/public/favicon.ico'));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
    extended : false
}));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', routes);
app.use('/users', users);

// catch 404 and forward to error handler
app.use(function(req, res, next) {
    var err = new Error('Not Found');
    err.status = 404;
    next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
    app.use(function(err, req, res, next) {
        res.status(err.status || 500);
        res.render('error', {
            message : err.message,
            error : err
        });
    });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
        message : err.message,
        error : {}
    });
});

module.exports = app;
