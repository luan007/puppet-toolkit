console.log("Starting as thin client");

var os = require('os');
var qs = require('querystring');
var platform = os.platform(); // 'darwin'
var ioclient = require('socket.io-client')
var io = ioclient("http://" + opts.ip + ":" + opts.port + "/client?" + qs.encode(opts))

io.connect();
io.on("reboot", () => {
    //console.log("reboot!"); return;
    if (platform !== 'darwin') {
        try {
            require('child_process').execSync(`shutdown /r /t 0`)
        } catch (e) {}
    } else {
        try {
            require('child_process').execSync(`shutdown -r now`)
        } catch (e) {}
    }
});

io.on("off", () => {
    //console.log("off!"); return;
    if (platform !== 'darwin') {
        try {
            require('child_process').execSync(`shutdown /s /t 0`)
        } catch (e) {}
    } else {
        try {
            require('child_process').execSync(`shutdown -h now`)
        } catch (e) {}
    }
});

// app.post("/reboot", (req, res) => {
//     res.json({}).end();
//     if (platform !== 'darwin') {
//         try {
//             require('child_process').execSync(`shutdown /r /t 0`)
//         } catch (e) {}
//     } else {
//         try {
//             require('child_process').execSync(`shutdown -r now`)
//         } catch (e) {}
//     }
// });

// app.post("/off", (req, res) => {
//     res.json({}).end();
//     if (platform !== 'darwin') {
//         try {
//             require('child_process').execSync(`shutdown /s /t 0`)
//         } catch (e) {}
//     } else {
//         try {
//             require('child_process').execSync(`shutdown -h now`)
//         } catch (e) {}
//     }
// });

// app.listen(3838);