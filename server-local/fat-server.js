var app = require('express')();
var server = require('http').Server(app);
var io = require('socket.io')(server);
var servestatic = require('serve-static');


var controllers = io.of("controller");
var managed_state = {};
var managed_socket = {};

controllers.on("connection", function (socket) {
    socket.on("control", (c) => {
        if (managed_socket[c.name]) {
            managed_socket[c.name].emit(c.command, c.args);
        }
    });
    socket.on("broadcast", (c) => {
        clients.emit(c.command, c.args);
    });
    socket.emit("update", managed_state);
})

function broadcast() {
    controllers.emit("update", managed_state);
}

var clients = io.of("client");
clients.on('connection', function (socket) {
    var q = socket.handshake.query;
    managed_state[q.name] = q;

    q.operations = {
        "关机": {
            cmd: "off",
            request_confirm: true
        },
        "重新启动": {
            cmd: "reboot",
            request_confirm: true
        },
    };

    managed_socket[q.name] = socket;
    console.log(q.name, "connected");
    broadcast();
    socket.on("disconnect", () => {
        delete managed_socket[q.name];
        delete managed_state[q.name];
        broadcast();
        console.log(q.name, "disconnected");
    });
});



//for board only
// app.get("/", (req, res) => {
//     var s = Math.random(); //
//     res.send({
//         state: managed_state[s] && managed_state[s].powerOn
//     }).end();
//     console.log(req.url);
// });

app.use(servestatic(__dirname + "/public"));

server.listen(opts.port, () => {
    console.log("Server started at", opts.port);
});