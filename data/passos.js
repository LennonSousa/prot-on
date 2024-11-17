// Editar o dispositivo 0 no primeiro acesso
function dispPrimeiroAcesso() {
    var xhttp = new XMLHttpRequest();
    var nome = document.getElementById("nomeDisp0").value;
    document.getElementById("nomeDispAguarde").style.display = "inline-block";
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            location.reload();
        }
    };

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("nomeDispAguarde").style.display = "none";
            document.getElementById("salvoAviso").innerHTML = "Salvo com sucesso!";
            document.getElementById("collapseOne").className = "collapse";
            document.getElementById("collapseTwo").className = "collapse show";
        }
    };

    xhttp.open("GET", "editadisp?id=0&nome=" + nome + "&editacompleto=0", true);
    xhttp.send();
}

function procuraRedes() {
    //Vari�vel que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, fun��o que ser� executada quando a requisi��o terminar
    xhttp.onload = listaRedes;
    //Abrimos e enviamos a requisi��o na rota 'pinList'

    xhttp.open("GET", "procuraredes", true);
    document.getElementById("redesDispAguarde").style.display = "inline-block";
    xhttp.send();
}

//Fun��o executada quando o ESP responder a requsi��o na rota 'pinList'
function listaRedes() {
    //Se a requisi��o foi bem sucedida
    if (this.status == 200) {
        document.getElementById("redesDispAguarde").style.display = "none";

        //Select com a lista de pinos
        var listaRedesBotoes = document.getElementById("div_botoes_redes");
        listaRedesBotoes.innerHTML = "";

        //Cria um objeto a partir do json que nos foi retornado
        var json = JSON.parse(this.responseText);
        var count = json.count;

        //Esta � a lista de pinos em uso, salvos no ESP, retornados por ele na requisi��o em 'pinList'
        redes = json.redes;

        //Para cada pino em uso
        for (var i = 0; i < redes.length; i++) {
            //Mant�m uma refer�ncia �s informa��es do pino de forma que seja poss�vel acess�-lo no callback
            let rede = redes[i];
            //Cria um novo bot�o para alterar o estado deste pino
            var btn = document.createElement("button");
            btn.className = "btn btn-outline-info";
            btn.setAttribute("data-toggle", "modal");
            btn.setAttribute("data-target", "#modalConectar");
            btn.setAttribute("data-ssid", rede.ssid);

            //btn.onclick = function (e) {

            //}

            //O texto que aparecer� no bot�o ser� o nome dado ao pino
            btn.innerHTML = rede.ssid + " (" + rede.seguranca + ")";
            //Adicionamos o bot�o em tela
            listaRedesBotoes.appendChild(btn);
        }
    }
    else {
        document.getElementById("redesDispAguarde").style.display = "none";
        alert("Erro ao recuperar informa��es");
    }
}

function conectarRede(ssid, senha) {
    //Vari�vel que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, fun��o que ser� executada quando a requisi��o terminar
    xhttp.onreadystatechange = conectaRedesResp;
    //Abrimos e enviamos a requisi��o na rota 'pinList'
    var ssidTexto = document.getElementById(ssid).innerHTML;
    var senhaTexto = document.getElementById(senha).value;

    xhttp.open("GET", "conectarede?ssid=" + ssidTexto + "&senha=" + senhaTexto, true);
    document.getElementById("conectaDispAguarde").style.display = "inline-block";
    xhttp.send();
}

function conectaRedesResp() {
    //Se a requisi��o foi bem sucedida
    if (this.status == 200) {
        if (this.responseText.search("http") != -1) {
            document.getElementById("conectaDispAguarde").style.display = "none";
            document.getElementById("conectandoAviso").innerHTML = "Conectado com sucesso!";
            document.getElementById("passo3Lb").innerHTML = "Esse � o endere�o do seu dispositivo, anote-o para poder acessa-lo depois";
            document.getElementById("ipDisp0").value = "http://" + this.responseText;
            document.getElementById("passo3Ajuda").innerHTML = "O seu dispositivo est� pronto, agora clique em Finalizar, ele ir� reiniciar " +
                "automaticamente e ent�o voc� poder� acess�-lo inserindo o endere�o em qualquer navegador de internet.";
            document.getElementById("btFinaliza").disabled = false;
            $('#modalConectar').modal('hide');
            document.getElementById("collapseTwo").className = "collapse";
            document.getElementById("collapseThree").className = "collapse show";
        }
    }
    if (this.status == 0) {
        document.getElementById("conectaDispAguarde").style.display = "none";
        document.getElementById("conectandoAviso").innerHTML = "N�o foi poss�vel conectar!";
    }
}

function concluiConfig() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            window.location = document.getElementById("ipDisp0").value;
        }
    };
    xhttp.open("GET", "finalizaconfig", true);
    xhttp.send();
}