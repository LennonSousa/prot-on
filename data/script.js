// JavaScript source code

function getStatusLocal() {
    //Vari�vel que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, fun��o que ser� executada quando a requisi��o terminar
    xhttp.onload = criaListaDisp;
    //Abrimos e enviamos a requisi��o na rota 'pinList'

    xhttp.open("GET", "estadodispositivos", true);
    xhttp.send();
}

function criaListaDisp() {
    //Se a requisi��o foi bem sucedida
    if (this.status == 200) {
        //Select com a lista de pinos
        var listaDispositivos = document.getElementById("sessao_dispositivos");

        //Cria um objeto a partir do json que nos foi retornado
        var json = JSON.parse(this.responseText);
        var count = json.count;

        //Esta � a lista de pinos em uso, salvos no ESP, retornados por ele na requisi��o em 'pinList'
        dispositivos = json.dispositivos;

        //Para cada pino em uso
        for (var i = 0; i < dispositivos.length; i++) {
            //Mant�m uma refer�ncia �s informa��es do pino de forma que seja poss�vel acess�-lo no callback
            let dispositivo = dispositivos[i];

            //Cria um novo bot�o para alterar o estado deste pino
            var linhaPrincipal = document.createElement("div");
            linhaPrincipal.className = "row align-items-center div-dispositivos";

            var colunaEsquerda = document.createElement("div");
            colunaEsquerda.className = "col-9";

            var colunaDireita = document.createElement("div");
            colunaDireita.className = "col-3";

            var linha01ColunaEsquerda = document.createElement("div");
            linha01ColunaEsquerda.className = "row";

            var linha02ColunaEsquerda = document.createElement("div");
            linha02ColunaEsquerda.className = "row";

            var linha01ColunaDireita = document.createElement("div");
            linha01ColunaDireita.className = "row";

            // Nome
            var colunaNome = document.createElement("div");
            colunaNome.className = "col-12 col-sm-6";
            var spanNome = document.createElement("span");
            spanNome.innerHTML = dispositivo.nome;
            colunaNome.appendChild(spanNome);

            // V�rios
            var linhaColunaVarios = document.createElement("div");
            linhaColunaVarios.className = "row";

            var colunaVarios01 = document.createElement("div");
            colunaVarios01.className = "col-6";

            var colunaVarios02 = document.createElement("div");
            colunaVarios02.className = "col-6";

            var btnVarios01 = document.createElement("button");
            btnVarios01.className = "btn btn-outline-info btn-sm";
            btnVarios01.setAttribute("data-toggle", "modal");
            btnVarios01.setAttribute("data-target", "#modalAlarmes");
            btnVarios01.setAttribute("data-nome", dispositivo.nome);
            btnVarios01.setAttribute("data-id", dispositivo.id);

            var btnVarios02 = document.createElement("button");
            btnVarios02.className = "btn btn-outline-info btn-sm";
            btnVarios02.setAttribute("data-toggle", "modal");
            btnVarios02.setAttribute("data-target", "#modalEditar");
            btnVarios02.setAttribute("data-id", dispositivo.id);
            btnVarios02.setAttribute("data-nome", dispositivo.nome);
            btnVarios02.setAttribute("data-ip", dispositivo.ip);

            var spanVarios01 = document.createElement("span");
            spanVarios01.className = "oi oi-timer";

            var spanVarios02 = document.createElement("span");
            spanVarios02.className = "oi oi-wrench";

            btnVarios01.appendChild(spanVarios01);
            btnVarios02.appendChild(spanVarios02);

            colunaVarios01.appendChild(btnVarios01);
            colunaVarios02.appendChild(btnVarios02);

            // Estado
            var colunaEstado = document.createElement("div");
            colunaEstado.className = "col-12";

            var btn = document.createElement("button");
            var spanBotaoEstado = document.createElement("span");
            spanBotaoEstado.className = "oi oi-power-standby";

            btn.id = "btn" + i;
            btn.setAttribute("onclick", "sendData('" + ("btn" + i) + "', '" + dispositivo.id + "')");
            if (dispositivo.estado == "0") {
                btn.className = "btn btn-dark";
            }
            else if (dispositivo.estado == "1") {
                btn.className = "btn btn-warning";
            }
            else if (dispositivo.estado == "n/a") {
                btn.className = "btn btn-outline-dark";
                btn.disabled = true;
                spanBotaoEstado.className = "oi oi-question-mark";
            }

            btn.appendChild(spanBotaoEstado);
            colunaEstado.appendChild(btn);

            //Adicionamos o bot�o em tela
            linha01ColunaEsquerda.appendChild(colunaNome);

            linha02ColunaEsquerda.appendChild(colunaVarios01);
            linha02ColunaEsquerda.appendChild(colunaVarios02);

            linha01ColunaDireita.appendChild(colunaEstado);

            colunaEsquerda.appendChild(linha01ColunaEsquerda);
            colunaEsquerda.appendChild(linha02ColunaEsquerda);

            colunaDireita.appendChild(linha01ColunaDireita);

            linhaPrincipal.appendChild(colunaEsquerda);
            linhaPrincipal.appendChild(colunaDireita);

            listaDispositivos.appendChild(linhaPrincipal);
        }
    }
    else {
        alert("Erro ao recuperar informa��es");
    }
}

// Adicionar um dispositivo
function novoDisp() {
    var xhttp = new XMLHttpRequest();
    var nome = document.getElementById("nomeNovoDisp").value;
    var ip = document.getElementById("ipNovoDisp").value;

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            location.reload();
        }
    };

    xhttp.open("GET", "novodisp?nome=" + nome + "&ip=" + ip, true);
    xhttp.send();
}

// Editar o dispositivo 0 no primeiro acesso
function dispPrimeiroAcesso() {
    var xhttp = new XMLHttpRequest();
    var nome = document.getElementById("nomeDisp0").value;

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            location.reload();
        }
    };

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("salvoAviso").innerHTML = "Salvo com sucesso!";
        }
    };

    xhttp.open("GET", "editadisp?id=0&nome=" + nome + "&editacompleto=0", true);
    xhttp.send();
}

// Editar um dispositivo
function editarDisp() {
    var xhttp = new XMLHttpRequest();
    var id = document.getElementById("idDisp").value;
    var nome = document.getElementById("nomeDisp").value;
    var fixo = "0";

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            location.reload();
        }
    };

    xhttp.open("GET", "editadisp?id=" + id + "&nome=" + nome + "&editacompleto=0", true);
    xhttp.send();
}

function excluirConfirma() {
    document.getElementById("divExcluir").style.display = "inline";
}

function excluirSim() {
    var xhttp = new XMLHttpRequest();
    var id = document.getElementById("idDisp").value;
    var fixo = "0";

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            location.reload();
        }
    };

    xhttp.open("GET", "excluidisp?id=" + id, true);
    xhttp.send();
}

function excluirNao() {
    document.getElementById("divExcluir").style.display = "none";
}

function sendData(btn, id) {
    var xhttp = new XMLHttpRequest();
    var modificaPara = "";
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (this.responseText == "0") {
                document.getElementById(btn).className = "btn btn-dark";
            }
            else {
                document.getElementById(btn).className = "btn btn-warning";
            }
        }
    };

    if (document.getElementById(btn).className == "btn btn-dark") {
        modificaPara = "1";
    }
    else if (document.getElementById(btn).className == "btn btn-warning") {
        modificaPara = "0";
    }

    xhttp.open("GET", "modifica?id=" + id + "&estado=" + modificaPara, true);
    xhttp.send();
}

function procuraRedes() {
    //Vari�vel que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, fun��o que ser� executada quando a requisi��o terminar
    xhttp.onload = listaRedes;
    //Abrimos e enviamos a requisi��o na rota 'pinList'

    xhttp.open("GET", "procuraredes", true);
    xhttp.send();
}

//Fun��o executada quando o ESP responder a requsi��o na rota 'pinList'
function listaRedes() {
    //Se a requisi��o foi bem sucedida
    if (this.status == 200) {
        //Select com a lista de pinos
        var listaRedesBotoes = document.getElementById("div_botoes_redes");

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
        alert("Erro ao recuperar informa��es");
    }
}

function conectarRede(ssid, senha) {
    var listaRedesBotoes = document.getElementById("div_botoes_redes");
    var ssidTexto = document.getElementById(ssid).innerHTML;
    var senhaTexto = document.getElementById(senha).value;

    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (this.responseText != "0") {
                document.getElementById("conectandoAviso").innerHTML = "Conectado com sucesso!";
                document.getElementById("passo3Lb").innerHTML = "Esse � o endere�o do seu dispositivo, anote-o para poder acessa-lo depois";
                document.getElementById("ipDisp0").value = "http://" + this.responseText;
                document.getElementById("passo3Ajuda").innerHTML = "O seu dispositivo est� pronto, agora clique em Finalizar, ele ir� reiniciar " +
                    "automaticamente e ent�o voc� poder� acess�-lo inserindo o endere�o em qualquer navegador de internet.";
                document.getElementById("btFinaliza").disabled = false;
            }
        }
        else if (this.readyState == 4 && this.status == 401) {
            document.getElementById("conectandoAviso").innerHTML = "N�o foi poss�vel conectar!";
        }
    };
    xhttp.open("GET", "conectarede?ssid=" + ssidTexto + "&senha=" + senhaTexto, true);
    xhttp.send();
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

// Alarmes in�cio
function procuraAlarmes(id) {

    //Vari�vel que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, fun��o que ser� executada quando a requisi��o terminar
    xhttp.onload = listaAlarmes;
    //Abrimos e enviamos a requisi��o na rota 'pinList'

    xhttp.open("GET", "procuralarmes?idDispAlarme=" + id, true);
    xhttp.send();
}

//Fun��o executada quando o ESP responder a requsi��o na rota 'pinList'
function listaAlarmes() {
    //Se a requisi��o foi bem sucedida
    if (this.status == 200) {
        //Select com a lista de pinos
        var listaAlarmes = document.getElementById("sessao_alarmes");

        //Cria um objeto a partir do json que nos foi retornado
        var json = JSON.parse(this.responseText);
        var count = json.count;

        //Esta � a lista de pinos em uso, salvos no ESP, retornados por ele na requisi��o em 'pinList'
        alarmes = json.alarmes;

        //Para cada pino em uso
        for (var i = 0; i < alarmes.length; i++) {
            //Mant�m uma refer�ncia �s informa��es do pino de forma que seja poss�vel acess�-lo no callback
            let alarme = alarmes[i];

            //Cria um novo bot�o para alterar o estado deste pino
            var linhaPrincipal = document.createElement("div");
            linhaPrincipal.className = "row align-items-center div-dispositivos";

            var colunaEsquerda = document.createElement("div");
            colunaEsquerda.className = "col-4";

            var colunaMeio = document.createElement("div");
            colunaMeio.className = "col-4";

            var colunaDireita = document.createElement("div");
            colunaDireita.className = "col-4";

            var linha01ColunaEsquerda = document.createElement("div");
            linha01ColunaEsquerda.className = "row";

            var linha02ColunaEsquerda = document.createElement("div");
            linha02ColunaEsquerda.className = "row";

            var linha01ColunaMeio = document.createElement("div");
            linha01ColunaMeio.className = "row";

            var linha02ColunaMeio = document.createElement("div");
            linha02ColunaMeio.className = "row";

            var linha01ColunaDireita = document.createElement("div");
            linha01ColunaDireita.className = "row";

            var linha02ColunaDireita = document.createElement("div");
            linha02ColunaDireita.className = "row";

            // Nome
            var colunaNome = document.createElement("div");
            colunaNome.className = "col-12 col-sm-6";
            var spanNome = document.createElement("span");
            spanNome.innerHTML = alarme.nome;
            colunaNome.appendChild(spanNome);

            // Hor�rio
            var colunaHorario = document.createElement("div");
            colunaHorario.className = "col-12 col-sm-6";

            var selectHora = document.createElement("select");
            selectHora.className = "form-control";
            selectHora.id = "selectHora" + alarme.id;
            for (var x = 0; x <= 23; x++) {
                var optionHora = document.createElement("option");
                optionHora.innerHTML = x;
            }
            selectHora.appendChild(optionHora);

            var selectMinuto = document.createElement("select");
            selectMinuto.className = "form-control";
            selectMinuto.id = "selectMinuto" + alarme.id;
            for (var y = 0; y <= 59; y++) {
                var optionMinuto = document.createElement("option");
                optionMinuto.innerHTML = x;
            }
            selectMinuto.appendChild(optionHora);

            colunaHorario.appendChild(selectHora);
            colunaHorario.appendChild(selectMinuto);

            // Salvar
            var colunaSalvar = document.createElement("div");
            colunaSalvar.className = "col-12";

            var btn = document.createElement("button");
            var spanBotaoEstado = document.createElement("span");
            spanBotaoEstado.className = "oi oi-task";

            btn.id = "btn" + i;
            btn.setAttribute("onclick", "sendData('" + ("btn" + i) + "', '" + alarme.id + "')");
            btn.className = "btn btn-success";

            btn.appendChild(spanBotaoEstado);
            colunaSalvar.appendChild(btn);

            //Adicionamos o bot�o em tela
            linha01ColunaEsquerda.appendChild(colunaNome);
            linha02ColunaEsquerda.appendChild(colunaHorario);

            linha01ColunaDireita.appendChild(colunaSalvar);

            colunaEsquerda.appendChild(linha01ColunaEsquerda);
            colunaEsquerda.appendChild(linha02ColunaEsquerda);

            colunaDireita.appendChild(linha01ColunaDireita);

            linhaPrincipal.appendChild(colunaEsquerda);
            linhaPrincipal.appendChild(colunaDireita);

            listaAlarmes.appendChild(linhaPrincipal);
        }
    }
    else {
        alert("Erro ao recuperar informa��es");
    }
}
// Alarmes fim