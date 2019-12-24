// var express = require('express');
// var app = express();

// app.get("/", (req, res) => {
//     res.send({
//         state: 0
//     }).end();
//     console.log(req.url);
// });

// app.listen(9999);

var arg = require('arg');

function parse_args(rawArgs) {
    const args = arg({
        '--server': Boolean,
        '--ip': String,
        '--desc': String,
        '--id': String,
        '--port': Number
    }, {
        argv: rawArgs.slice(2),
    });
    return {
        server: args['--server'] || false,
        port: args['--port'] || 9999,
        ip: args['--ip'],
        desc: args['--desc'] || "Generic Device",
        name: args['--name'] || "Default_Id"
    };
}

var parsed = parse_args(process.argv);
global.opts = parsed;
console.log(global.opts);
if (parsed.server) {
    require('./fat-server.js');
} else {
    require('./thin-client.js');
}