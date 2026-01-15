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
  //Variável que faz o request http
  var xhttp = new XMLHttpRequest();
  //Callback, funçãoo que será executada quando a requisição terminar
  xhttp.onreadystatechange = listaRedes;
  //Abrimos e enviamos a requisição na rota 'pinList'

  xhttp.open("GET", "procuraredes", true);
  document.getElementById("redesDispAguarde").style.display = "inline-block";
  xhttp.send();
}

//Função executada quando o ESP responder a requsição na rota 'pinList'
function listaRedes() {
  if (this.readyState === 4) {
    //Se a requisição foi bem sucedida
    if (this.status === 200) {
      document.getElementById("redesDispAguarde").style.display = "none";

      //Select com a lista de pinos
      var listaRedesBotoes = document.getElementById("div_botoes_redes");
      listaRedesBotoes.innerHTML = "";

      //Cria um objeto a partir do json que nos foi retornado
      var json = JSON.parse(this.responseText);

      //Esta é a lista de pinos em uso, salvos no ESP, retornados por ele na requisição em 'pinList'
      const redes = json.results;

      //Para cada pino em uso
      for (var i = 0; i < redes.length; i++) {
        //Mantém uma referência das informações do pino de forma que seja possível acessá-lo no callback
        const rede = redes[i];
        //Cria um novo botão para alterar o estado deste pino
        var btn = document.createElement("button");
        btn.className = "btn btn-outline-info";
        btn.setAttribute("data-toggle", "modal");
        btn.setAttribute("data-target", "#modalConectar");
        btn.setAttribute("data-ssid", rede.ssid);

        //O texto que aparecer no botãoo será o nome dado ao pino
        btn.innerHTML = rede.ssid + " (" + rede.secure + ")";
        //Adicionamos o botão em tela
        listaRedesBotoes.appendChild(btn);
      }
    } else {
      document.getElementById("redesDispAguarde").style.display = "none";
      alert("Erro ao recuperar informações");
    }
  }
}

function conectarRede(ssid, senha) {
  //Variável que faz o request http
  var xhttp = new XMLHttpRequest();
  //Callback, função que será executada quando a requisição terminar
  xhttp.onreadystatechange = conectaRedesResp;
  //Abrimos e enviamos a requisição na rota 'pinList'
  var ssidTexto = document.getElementById(ssid).innerHTML;
  var senhaTexto = document.getElementById(senha).value;

  xhttp.open(
    "POST",
    "conectarede?ssid=" + ssidTexto + "&senha=" + senhaTexto,
    true
  );
  document.getElementById("conectaDispAguarde").style.display = "inline-block";
  xhttp.send();
}

function conectaRedesResp() {
  if (this.readyState === 4) {
    //Se a requisição foi bem sucedida
    if (this.status === 201) {
      var response = JSON.parse(this.responseText);

      if (response?.baseUrl) {
        document.getElementById("conectaDispAguarde").style.display = "none";
        document.getElementById("conectandoAviso").innerHTML =
          "Conectado com sucesso!";
        document.getElementById("passo3Lb").innerHTML =
          "Esse é o endereço do seu dispositivo, anote-o para poder acessa-lo depois";
        document.getElementById("ipDisp0").value = response.baseUrl;
        document.getElementById("passo3Ajuda").innerHTML =
          "O seu dispositivo está pronto, agora clique em Finalizar, ele irá reiniciar " +
          "automaticamente e então você poderá acessá-lo inserindo o endereço em qualquer navegador de internet.";
        document.getElementById("btFinaliza").disabled = false;
        $("#modalConectar").modal("hide");
        document.getElementById("collapseTwo").className = "collapse";
        document.getElementById("collapseThree").className = "collapse show";
      }
    } else {
      document.getElementById("conectaDispAguarde").style.display = "none";
      document.getElementById("conectandoAviso").innerHTML =
        "Não foi possível conectar!";
    }
  }
}

function concluiConfig() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState === 4 && this.status === 200) {
      window.location = document.getElementById("ipDisp0").value;
    }
  };
  xhttp.open("GET", "finalizaconfig", true);
  xhttp.send();
}
