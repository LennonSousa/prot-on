// Editar o dispositivo 0 no primeiro acesso
async function dispPrimeiroAcesso() {
  try {
    var name = document.getElementById("nomeDisp0").value;
    document.getElementById("nomeDispAguarde").style.display = "inline-block";

    var body = {
      name,
    };

    const response = await fetch("/first-setting/device", {
      method: "PUT",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(body),
    });

    if (!response.ok) {
      document.getElementById("nomeDispAguarde").style.display = "none";
      document.getElementById("salvoAviso").innerHTML = "Salvo com sucesso!";
      document.getElementById("collapseOne").className = "collapse";
      document.getElementById("collapseTwo").className = "collapse show";
    } else {
      const data = await response.json();

      document.getElementById("nomeDispAguarde").style.display = "none";
      alert("Erro ao editar dispositivo: " + data?.message);
    }
  } catch (error) {
    document.getElementById("nomeDispAguarde").style.display = "none";
    alert("Erro ao editar dispositivo: " + error);
    return;
  }
}

async function procuraRedes() {
  try {
    document.getElementById("redesDispAguarde").style.display = "inline-block";

    const response = await fetch("/wireless", {
      method: "GET",
    });

    document.getElementById("redesDispAguarde").style.display = "none";

    const data = await response.json();

    if (!response.ok || response.status !== 201) {
      document.getElementById("redesDispAguarde").style.display = "none";
      alert("Erro ao recuperar informações: " + data?.message);

      return;
    }

    listaRedes({ data });
  } catch (error) {
    document.getElementById("redesDispAguarde").style.display = "none";
    alert("Erro ao recuperar informações: " + error);
  }
}

function listaRedes({ data }) {
  document.getElementById("redesDispAguarde").style.display = "none";

  var listaRedesBotoes = document.getElementById("div_botoes_redes");
  listaRedesBotoes.innerHTML = "";

  const redes = data.results;

  for (var i = 0; i < data.count; i++) {
    const rede = redes[i];

    var btn = document.createElement("button");

    btn.className = "btn btn-outline-info";
    btn.setAttribute("data-toggle", "modal");
    btn.setAttribute("data-target", "#modalConectar");
    btn.setAttribute("data-ssid", rede.ssid);

    btn.innerHTML = rede.ssid + " (" + rede.secure + ")";

    listaRedesBotoes.appendChild(btn);
  }
}

async function conectarRede(ssid, senha) {
  try {
    var ssidTexto = document.getElementById(ssid).innerHTML;
    var senhaTexto = document.getElementById(senha).value;

    document.getElementById("conectaDispAguarde").style.display =
      "inline-block";

    const response = await fetch("/wireless", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ ssid: ssidTexto, password: senhaTexto }),
    });

    const data = await response.json();

    if (response.status !== 201) {
      document.getElementById("conectaDispAguarde").style.display = "none";
      document.getElementById("conectandoAviso").innerHTML =
        error.message || "Não foi possível conectar!";

      return response.json();
    }

    if (response.ok && data.baseUrl) {
      document.getElementById("conectaDispAguarde").style.display = "none";
      document.getElementById("conectandoAviso").innerHTML =
        "Conectado com sucesso!";
      document.getElementById("passo3Lb").innerHTML =
        "Esse é o endereço do seu dispositivo, anote-o para poder acessa-lo depois";
      document.getElementById("ipDisp0").value = data.baseUrl;
      document.getElementById("passo3Ajuda").innerHTML =
        "O seu dispositivo está pronto, agora clique em Finalizar, ele irá reiniciar automaticamente e então você poderá acessá-lo inserindo o endereço em qualquer navegador de internet.";
      document.getElementById("btFinaliza").disabled = false;
      $("#modalConectar").modal("hide");
      document.getElementById("collapseTwo").className = "collapse";
      document.getElementById("collapseThree").className = "collapse show";
    }
  } catch (error) {
    document.getElementById("conectaDispAguarde").style.display = "none";
    document.getElementById("conectandoAviso").innerHTML =
      error.message || "Não foi possível conectar!";
  }
}

async function concluiConfig() {
  try {
    const response = await fetch("/first-setting/finish", {
      method: "POST",
    });

    if (response.ok) {
      window.location = document.getElementById("ipDisp0").value;
    } else {
      alert("Erro ao finalizar configuração");
    }
  } catch (error) {
    alert("Erro de conexão: " + error);
  }
}
