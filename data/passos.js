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

    if (response.ok) {
      document.getElementById("nomeDispAguarde").style.display = "none";
      document.getElementById("salvoAviso").innerHTML = t("saved_success");
      document.getElementById("collapseOne").className = "collapse";
      document.getElementById("collapseTwo").className = "collapse show";
    } else {
      const data = await response.json();

      document.getElementById("nomeDispAguarde").style.display = "none";
      alert(t("error_edit_device") + " " + data?.message);
    }
  } catch (error) {
    document.getElementById("nomeDispAguarde").style.display = "none";
    alert(t("error_edit_device") + " " + error);
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

    if (!response.ok || response.status !== 200) {
      document.getElementById("redesDispAguarde").style.display = "none";
      alert(t("error_fetch") + " " + data?.message);

      return;
    }

    listaRedes({ data });
  } catch (error) {
    document.getElementById("redesDispAguarde").style.display = "none";
    alert(t("error_fetch") + " " + error);
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

    const networkSecurity = rede.secure ? t("secure") : t("opened");

    btn.innerHTML = rede.ssid + " (" + networkSecurity + ")";

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
        data?.message || t("cannot_connect");

      return;
    }

    if (response.ok && data.baseUrl) {
      document.getElementById("conectaDispAguarde").style.display = "none";
      document.getElementById("conectandoAviso").innerHTML =
        t("connected_success");
      document.getElementById("passo3Lb").innerHTML = t("device_address_info");
      document.getElementById("ipDisp0").value = data.baseUrl;
      document.getElementById("passo3Ajuda").innerHTML = t("device_ready_info");
      document.getElementById("btFinaliza").disabled = false;
      $("#modalConectar").modal("hide");
      document.getElementById("collapseTwo").className = "collapse";
      document.getElementById("collapseThree").className = "collapse show";
    }
  } catch (error) {
    document.getElementById("conectaDispAguarde").style.display = "none";
    document.getElementById("conectandoAviso").innerHTML =
      error.message || t("cannot_connect");
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
      alert(t("error_finish_config"));
    }
  } catch (error) {
    alert(t("error_connection") + " " + error);
  }
}
