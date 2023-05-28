const express = require('express');
const app = express();
const port = 80;
let timerval = 0;

let timerId; // 全局变量用于存储计时器ID
let isPaused = false; // 表示计时器是否处于暂停状态
let isstart = false;
app.use(express.static('web'));
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/web/index.html');
});
const server = app.listen(port, () => {
    console.log(`Server is running on port ${port}`);
});
const SocketServer = require("ws").Server;
const wss = new SocketServer({ server });

wss.on("connection", (ws) => {

    ws.on("message", (event) => {
        let res = JSON.parse(event.toString());
        if (res.get == "add") {
            if (!isstart) {
                timerval += res.data;

                if (timerval > 5999) {
                    timerval = 0;
                    senderror("時間超出範圍");
                }
                sendtime(timerval)
            } else {
                senderror("時間不得於計時中更改");
            }
        }
        if (res.get == "sub") {
            if (!isstart) {
                timerval -= res.data;
                if (timerval < 0) {
                    timerval = 0;
                    senderror("時間不得為負數");
                }
                sendtime(timerval)
            } else {
                senderror("時間不得於計時中更改");
            }
        }
        if (res.get == "start") {
            start(timerval)
        }
        if (res.get == "stop") {
            stop();
            timerval = 0;
            sendtime(timerval)
        }
    });
    ws.on("close", () => {
        console.log("有人斷開連線");
    });
});

function send(data) {
    let clients = wss.clients;
    clients.forEach((client) => {
        let sendData = data
        client.send(sendData);//回去的資料
    });
}

function start(seconds) {
    if (isstart) {
        isPaused = !isPaused;
    } else {
        isstart = true;
        if (seconds === 0) {
            positiveTimer(); // 执行正数计时
            return;
        }

        let currentSeconds = seconds;

        // 定义计时器函数
        function countdown() {
            if (currentSeconds === 0) {
                clearInterval(timerId); // 停止计时器
                console.log("計時結束"); // 替换为你要执行的操作
                isstart = false;
                sendtime(timerval)
                return;
            }

            if (!isPaused) {
                currentSeconds--;
                sendtime(currentSeconds); // 发送当前秒数
            }
        }

        sendtime(currentSeconds); // 初始秒数
        timerId = setInterval(countdown, 1000); // 每秒执行一次计时器函数
    }
}
function positiveTimer() {
    let currentSeconds = 0;

    // 定义计时器函数
    function startTimer() {
        if (!isPaused) {
            currentSeconds++;
            sendtime(currentSeconds); // 发送当前秒数
        }
    }

    sendtime(currentSeconds); // 初始秒数
    timerId = setInterval(startTimer, 1000); // 每秒执行一次计时器函数
}


function stop() {
    clearInterval(timerId); // 停止计时器
    console.log("計時已停止"); // 替换为你要执行的操作
    isstart = false;
    isPaused = false; // 重置暂停状态
}




function senderror(data) {
    send(JSON.stringify({ get: "error", data: data }))
}
function sendtime(sec) {
    send(JSON.stringify({ get: "updatetime", data: sec }))
}