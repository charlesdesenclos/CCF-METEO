document.getElementById('connexion').hidden = false;
document.getElementById('table').hidden = true;

/* WebSocket connexion */
document.getElementById('connect').addEventListener('click', function() {
    var serverName  = document.getElementById('serverName').value;
    var hostName    = document.getElementById('hostName').value;
    var userName    = document.getElementById('userName').value;
    var password    = document.getElementById('password').value;
    var dbName      = document.getElementById('dbName').value;

    var socket = new WebSocket("ws://"+ serverName +":1234");


    /* WebSocket open */
    socket.onopen = function() {
        document.getElementById('connexion').hidden = true;
        document.getElementById('table').hidden = false;

        var data = hostName+";"+userName+";"+password+";"+dbName;
        socket.send(data);

        console.log("Client WebSocket: Nouvelle connexion");
    };

    /* WebSocket close */
    socket.onclose = function() {
        document.getElementById('connexion').hidden = false;
        document.getElementById('table').hidden = true;

        console.log("Client WebSocket: Deconnexion");
    };

    /* WebSocket error */
    socket.onerror = function(error) {
        console.error(error);
    };
            
    /* Receive message of the server */
    socket.onmessage = function(event) {
        var message = event.data;
        var regex = /[^;]+/g;
        const messageContent = [message.match(regex)];

        var tblBody = document.getElementById('tbody')
        var row = document.createElement("tr");
        for(var i = 0; i < messageContent[0].length; i++) {
            var cell = document.createElement("td");
            var cellText = document.createTextNode(messageContent[0][i])
            cell.appendChild(cellText);
            row.appendChild(cell);
        }
        tblBody.appendChild(row);
        tblBody.scrollTop = tblBody.scrollHeight;
    };

    /* WebSocket disconnexion */
    document.getElementById('disconnect').addEventListener('click', function() {
        socket.close();
    });
});