let url = 'ws://localhost'
var ws = new WebSocket(url)
// 監聽連線狀態
ws.onopen = () => {
    //src2.innerHTML = "<h2>已與伺服器連線</h2>"
}
ws.onclose = () => {
    //src2.innerHTML = "<h2>伺服器連線失敗，請重新整理</h2>"
}
//接收 Server 發送的訊息
ws.onmessage = event => {
    let res = JSON.parse(event.data);
    if (res.get == "updatetime") {
        let number = formatSecond(res.data);
        let timer = document.getElementById("timer");
        timer.innerHTML = number;
    }
    if (res.get == "error") {
        alert(res.data);
    }
}
function start() {
    ws.send(JSON.stringify({ get: "start" }));
}
function stop() {
    ws.send(JSON.stringify({ get: "stop" }));
}
function addtime(sec) {
    ws.send(JSON.stringify({ get: "add", data: sec }));
}
function subtime(sec) {
    ws.send(JSON.stringify({ get: "sub", data: sec }));
}
function formatSecond(seconds) {
    var minutes = Math.floor(seconds / 60); // 计算分钟数
    var remainingSeconds = seconds % 60; // 计算剩余的秒数

    var formattedMinutes = ('0' + minutes).slice(-2); // 格式化分钟数，确保始终有两位数字
    var formattedSeconds = ('0' + remainingSeconds).slice(-2); // 格式化秒数，确保始终有两位数字

    return formattedMinutes + ':' + formattedSeconds; // 返回格式化的分钟和秒数
}
