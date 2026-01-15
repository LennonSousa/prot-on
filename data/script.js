// JavaScript source code

function getStatusLocal() {
    //Variável que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, função que será executada quando a requisição terminar
    xhttp.onload = criaListaDisp;
    //Abrimos e enviamos a requisição na rota 'pinList'

    xhttp.open("GET", "estadodispositivos", true);
    xhttp.send();
}

function criaListaDisp() {
    //Se a requisição foi bem sucedida
    if (this.status == 200) {
        //Select com a lista de pinos
        var listaDispositivos = document.getElementById("sessao_dispositivos");

        //Cria um objeto a partir do json que nos foi retornado
        var json = JSON.parse(this.responseText);
        var count = json.count;

        //Esta é a lista de dispositivos salvos no ESP
        dispositivos = json.dispositivos;

        //Para cada pino em uso
        for (var i = 0; i < dispositivos.length; i++) {
            //Mantém uma referência às informações do pino de forma que seja possível acessá-lo no callback
            let dispositivo = dispositivos[i];

            //Cria um novo botão para alterar o estado deste pino
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

            // Vários
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

            //Adicionamos o botão em tela
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
        alert("Erro ao recuperar informações");
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

// Alarmes início
function procuraAlarmes(id) {

    //Variável que faz o request http
    var xhttp = new XMLHttpRequest();
    //Callback, função que será executada quando a requisição terminar
    xhttp.onload = listaAlarmes;
    //Abrimos e enviamos a requisição na rota 'pinList'

    xhttp.open("GET", "procuralarmes?idDispAlarme=" + id, true);
    xhttp.send();
}

//Função executada quando o ESP responder a requsição na rota 'pinList'
function listaAlarmes() {
    //Se a requisição foi bem sucedida
    if (this.status == 200) {
        //Select com a lista de pinos
        var listaAlarmes = document.getElementById("sessao_alarmes");

        while (listaAlarmes.hasChildNodes()) {
            listaAlarmes.removeChild(listaAlarmes.firstChild);
        }

        //Cria um objeto a partir do json que nos foi retornado
        var json = JSON.parse(this.responseText);
        var count = json.count;

        //Esta é a lista de pinos em uso, salvos no ESP, retornados por ele na requisição em 'pinList'
        alarmes = json.alarmes;

        //Para cada pino em uso
        for (var i = 0; i < alarmes.length; i++) {
            //Mantém uma referência às informações do pino de forma que seja possível acessá-lo no callback
            let alarme = alarmes[i];

            //Cria um novo botão para alterar o estado deste pino
            var linhaPrincipal = document.createElement("div");
            linhaPrincipal.className = "row align-items-center div-dispositivos";
            linhaPrincipal.id = "linhaAlarme" + alarme.id;

            var colunaEsquerda = document.createElement("div");
            colunaEsquerda.className = "col-12 col-lg-4 form-group";

            var colunaMeio = document.createElement("div");
            colunaMeio.className = "col-8 col-lg-4 form-group";

            var colunaDireita = document.createElement("div");
            colunaDireita.className = "col-4 form-group";

            var linha01ColunaEsquerda = document.createElement("div");
            linha01ColunaEsquerda.className = "row";

            var linha02ColunaEsquerda = document.createElement("div");
            linha02ColunaEsquerda.className = "row";

            var linha01ColunaMeio = document.createElement("div");
            linha01ColunaMeio.className = "row";

            var linha02ColunaMeio = document.createElement("div");
            linha02ColunaMeio.className = "row";

            var linha01ColunaDireita = document.createElement("div");
            linha01ColunaDireita.className = "row form-group";

            var linha02ColunaDireita = document.createElement("div");
            linha02ColunaDireita.className = "row form-group";

            // ID
            var colunaNome = document.createElement("div");
            colunaNome.className = "col-12";
            //var spanId = document.createElement("span");
            //spanId.id = "idAlarme" + alarme.id;
            //spanId.innerText = alarme.id;
            //colunaNome.appendChild(spanId);

            // Nome
            var inputNome = document.createElement("input");
            inputNome.id = "nomeAlarme" + alarme.id;
            inputNome.type = "text";
            inputNome.className = "form-control";
            inputNome.value = alarme.nome;
            colunaNome.appendChild(inputNome);

            // Horário
            var colunaHora = document.createElement("div");
            colunaHora.className = "col-6";

            var colunaMinuto = document.createElement("div");
            colunaMinuto.className = "col-6";

            var spanHora = document.createElement("label");
            spanHora.innerHTML = "Hora";
            var selectHora = document.createElement("select");
            selectHora.className = "form-control";
            selectHora.id = "selectHora" + alarme.id;
            for (var x = 0; x <= 23; x++) {
                var optionHora = document.createElement("option");
                optionHora.innerHTML = x;
                optionHora.value = x;
                selectHora.appendChild(optionHora);
            }
            selectHora.selectedIndex = alarme.hora;

            var spanMinuto = document.createElement("label");
            spanMinuto.innerHTML = "Minuto";
            var selectMinuto = document.createElement("select");
            selectMinuto.className = "form-control";
            selectMinuto.id = "selectMinuto" + alarme.id;
            for (var y = 0; y <= 59; y++) {
                var optionMinuto = document.createElement("option");
                optionMinuto.innerHTML = y;
                optionMinuto.value = y;
                selectMinuto.appendChild(optionMinuto);
            }
            selectMinuto.selectedIndex = alarme.minuto;

            colunaHora.appendChild(spanHora);
            colunaHora.appendChild(selectHora);
            colunaMinuto.appendChild(spanMinuto);
            colunaMinuto.appendChild(selectMinuto);

            // Ação e Ativo
            var colunaAcao = document.createElement("div");
            colunaAcao.className = "col-12";

            var colunaAtivo = document.createElement("div");
            colunaAtivo.className = "col-12";

            var spanAcao = document.createElement("label");
            spanAcao.innerHTML = "Ação";
            var selectAcao = document.createElement("select");
            selectAcao.className = "form-control";
            selectAcao.id = "selectAcao" + alarme.id;

            var optionAcao01 = document.createElement("option");
            optionAcao01.innerHTML = "Desligar";
            optionAcao01.value = "0";
            selectAcao.appendChild(optionAcao01);

            var optionAcao02 = document.createElement("option");
            optionAcao02.innerHTML = "Ligar";
            optionAcao02.value = "1";
            selectAcao.appendChild(optionAcao02);
            selectAcao.selectedIndex = alarme.acao;

            var spanAtivo = document.createElement("label");
            spanAtivo.innerHTML = "Estado";
            var selectAtivo = document.createElement("select");
            selectAtivo.className = "form-control";
            selectAtivo.id = "selectAtivo" + alarme.id;

            var optionAtivo01 = document.createElement("option");
            optionAtivo01.innerHTML = "Desativado";
            optionAtivo01.value = "0";
            selectAtivo.appendChild(optionAtivo01);

            var optionAtivo02 = document.createElement("option");
            optionAtivo02.innerHTML = "Ativado";
            optionAtivo02.value = "1";
            selectAtivo.appendChild(optionAtivo02);
            selectAtivo.selectedIndex = alarme.ativo;

            colunaAcao.appendChild(spanAcao);
            colunaAcao.appendChild(selectAcao);
            colunaAtivo.appendChild(spanAtivo);
            colunaAtivo.appendChild(selectAtivo);

            // Salvar
            var colunaSalvar = document.createElement("div");
            colunaSalvar.className = "col-12";

            var btnSalvar = document.createElement("button");
            btnSalvar.className = "btn btn-success";
            btnSalvar.id = "btnSalvar" + alarme.id;
            btnSalvar.setAttribute("onclick", "editarAlarme('" + alarme.id + "')");

            var spanBotaoSalvar = document.createElement("span");
            spanBotaoSalvar.className = "oi oi-task";

            btnSalvar.appendChild(spanBotaoSalvar);

            var divProgressoSalvar = document.createElement("div");
            divProgressoSalvar.className = "spinner-border text-success";
            divProgressoSalvar.id = "progresso" + alarme.id;
            divProgressoSalvar.setAttribute("role", "status");
            divProgressoSalvar.style.display = "none";

            var spanProgressoSalvar = document.createElement("span");
            spanProgressoSalvar.className = "sr-only";
            spanProgressoSalvar.innerHTML = "Salvando...";

            divProgressoSalvar.appendChild(spanProgressoSalvar);

            colunaSalvar.appendChild(btnSalvar);
            colunaSalvar.appendChild(divProgressoSalvar);

            // Excluir
            var colunaExcluir = document.createElement("div");
            colunaExcluir.className = "col-12";

            var btnExcluir = document.createElement("button");
            btnExcluir.className = "btn btn-danger";
            btnExcluir.id = "btnExcluir" + alarme.id;
            btnExcluir.setAttribute("onclick", "excluirAlarme('" + alarme.id + "')");

            var spanBotaoExcluir = document.createElement("span");
            spanBotaoExcluir.className = "oi oi-trash";

            btnExcluir.appendChild(spanBotaoExcluir);

            var divProgressoExcluir = document.createElement("div");
            divProgressoExcluir.className = "spinner-border text-danger";
            divProgressoExcluir.id = "progressoExcluir" + alarme.id;
            divProgressoExcluir.setAttribute("role", "status");
            divProgressoExcluir.style.display = "none";

            var spanProgressoExcluir = document.createElement("span");
            spanProgressoExcluir.className = "sr-only";
            spanProgressoExcluir.innerHTML = "Excluindo...";

            divProgressoExcluir.appendChild(spanProgressoExcluir);

            colunaExcluir.appendChild(btnExcluir);
            colunaExcluir.appendChild(divProgressoExcluir);

            //Adicionamos o botão em tela
            linha01ColunaEsquerda.appendChild(colunaNome);
            linha02ColunaEsquerda.appendChild(colunaHora);
            linha02ColunaEsquerda.appendChild(colunaMinuto);

            linha01ColunaDireita.appendChild(colunaSalvar);
            linha02ColunaDireita.appendChild(colunaExcluir);

            colunaEsquerda.appendChild(linha01ColunaEsquerda);
            colunaEsquerda.appendChild(linha02ColunaEsquerda);

            linha01ColunaMeio.appendChild(colunaAcao);
            linha02ColunaMeio.appendChild(colunaAtivo);

            colunaMeio.appendChild(linha01ColunaMeio);
            colunaMeio.appendChild(linha02ColunaMeio);

            colunaDireita.appendChild(linha01ColunaDireita);
            colunaDireita.appendChild(linha02ColunaDireita);

            linhaPrincipal.appendChild(colunaEsquerda);
            linhaPrincipal.appendChild(colunaMeio);
            linhaPrincipal.appendChild(colunaDireita);

            listaAlarmes.appendChild(linhaPrincipal);
        }
    }
    else {
        alert("Erro ao recuperar informações");
    }
}

// Editar um alarme
function editarAlarme(id) {
    var xhttp = new XMLHttpRequest();
    var nome = document.getElementById("nomeAlarme" + id).value;
    var hora = document.getElementById("selectHora" + id).selectedIndex;
    var minuto = document.getElementById("selectMinuto" + id).selectedIndex;
    var acao = document.getElementById("selectAcao" + id).selectedIndex;
    var ativo = document.getElementById("selectAtivo" + id).selectedIndex;

    document.getElementById("btnSalvar" + id).style.display = "none";
    document.getElementById("progresso" + id).style.display = "inline-block";

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("progresso" + id).style.display = "none";
            document.getElementById("btnSalvar" + id).style.display = "inline-block";
        }
    };

    xhttp.open("GET", "editaalarme?id=" + id + "&nome=" + nome + "&hora=" + hora + "&minuto=" + minuto + "&acao=" + acao + "&ativo=" + ativo, true);
    xhttp.send();
}

// Excluir um alarme
function excluirAlarme(id) {
    var xhttp = new XMLHttpRequest();
    document.getElementById("btnExcluir" + id).style.display = "none";
    document.getElementById("progressoExcluir" + id).style.display = "inline-block";

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("progressoExcluir" + id).style.display = "none";
            document.getElementById("linhaAlarme" + id).style.opacity = 0;
            document.getElementById("linhaAlarme" + id).style.display = "none";
            
        }
    };

    xhttp.open("GET", "excluialarme?id=" + id, true);
    xhttp.send();
}

function retiraAlarme(id) {
    
}
// Alarmes fim