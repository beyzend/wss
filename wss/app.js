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
    
console.log("Collecting updates from weather server...");
subscriber.subscribe("");
subscriber.connect("tcp://127.0.0.1:4200");

// Setup socket
var io = require('socket.io')(server);
var total_temp = 0;
var temps = 0;
    
io.on('connection', function(socket) {
    socket.emit('news', {
        hello : 'world'
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
       // Pretend we're doing some marshalling here, send it off as json.
       var pieces = data.toString().split(" ");
       socket.emit('regionData', {
           ids : pieces
       }); 
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
