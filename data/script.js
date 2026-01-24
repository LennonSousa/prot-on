async function getStatusLocal() {
  try {
    const response = await fetch("device", {
      method: "GET",
    });

    if (!response.ok) {
      throw new Error("Erro ao recuperar informações");
    }

    const data = await response.json();

    criaListaDisp(data);
  } catch (error) {
    alert(error.message);
  }
}

function criaListaDisp(dispositivosJson) {
  //Select com a lista de pinos
  var listaDispositivos = document.getElementById("sessao_dispositivos");

  //Esta é a lista de dispositivos salvos no ESP
  const dispositivos = dispositivosJson;

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
    spanNome.innerHTML = dispositivo.name;
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
    btnVarios01.setAttribute("data-nome", dispositivo.name);
    btnVarios01.setAttribute("data-id", dispositivo.id);

    var btnVarios02 = document.createElement("button");
    btnVarios02.className = "btn btn-outline-info btn-sm";
    btnVarios02.setAttribute("data-toggle", "modal");
    btnVarios02.setAttribute("data-target", "#modalEditar");
    btnVarios02.setAttribute("data-id", dispositivo.id);
    btnVarios02.setAttribute("data-nome", dispositivo.name);
    btnVarios02.setAttribute("data-ip", dispositivo.ip);
    btnVarios02.setAttribute("data-main", dispositivo.main);

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
    btn.setAttribute(
      "onclick",
      "sendData('" + ("btn" + i) + "', '" + dispositivo.id + "')",
    );

    if (dispositivo.status == "0") {
      btn.className = "btn btn-dark";
    } else if (dispositivo.status == "1") {
      btn.className = "btn btn-warning";
    } else if (dispositivo.status == "error") {
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

// Adicionar um dispositivo
async function novoDisp() {
  try {
    document.getElementById("novoDispBtns").style.display = "none";
    document.getElementById("novoDispAguarde").style.display = "inline-block";

    var name = document.getElementById("nomeNovoDisp").value;
    var ip = document.getElementById("ipNovoDisp").value;

    var body = {
      name,
      ip,
    };

    const response = await fetch("/device", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(body),
    });

    if (response.ok) {
      location.reload();
    } else {
      const data = await response.json();

      document.getElementById("novoDispBtns").style.display = "inline-block";
      document.getElementById("novoDispAguarde").style.display = "none";

      alert("Erro ao criar dispositivo: " + data?.message);
    }
  } catch (error) {
    document.getElementById("novoDispBtns").style.display = "inline-block";
    document.getElementById("novoDispAguarde").style.display = "none";

    alert("Erro ao adicionar dispositivo: " + error);
  }
}

// Editar um dispositivo
async function editarDisp() {
  document.getElementById("divExcluir").style.display = "none";
  document.getElementById("editarDispBtns").style.display = "none";
  document.getElementById("editarDispAguarde").style.display = "inline-block";

  var id = document.getElementById("idDisp").value;
  var name = document.getElementById("nomeDisp").value;
  var ip = document.getElementById("ipDisp").value;

  var body = {
    name,
    ip,
  };

  try {
    const response = await fetch(`device?id=${encodeURIComponent(id)}`, {
      method: "PUT",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(body),
    });

    if (response.ok) {
      location.reload();
    } else {
      const data = await response.json();

      document.getElementById("editarDispBtns").style.display = "inline-block";
      document.getElementById("editarDispAguarde").style.display = "none";

      alert("Erro ao editar dispositivo: " + data?.message);
    }
  } catch (error) {
    document.getElementById("editarDispBtns").style.display = "inline-block";
    document.getElementById("editarDispAguarde").style.display = "none";

    alert("Erro ao editar dispositivo: " + error);
  }
}

function excluirConfirma() {
  document.getElementById("divExcluir").style.display = "inline";
}

async function excluirSim() {
  try {
    document.getElementById("divExcluir").style.display = "none";
    document.getElementById("editarDispBtns").style.display = "none";
    document.getElementById("editarDispAguarde").style.display = "inline-block";

    var id = document.getElementById("idDisp").value;

    const response = await fetch(`device?id=${encodeURIComponent(id)}`, {
      method: "DELETE",
    });

    if (response.ok) {
      location.reload();
    } else {
      const data = await response.json();

      document.getElementById("editarDispBtns").style.display = "inline-block";
      document.getElementById("editarDispAguarde").style.display = "none";

      alert("Erro ao excluir dispositivo: " + data?.message);
    }
  } catch (error) {
    document.getElementById("editarDispBtns").style.display = "inline-block";
    document.getElementById("editarDispAguarde").style.display = "none";

    alert("Erro ao excluir dispositivo: " + error);
  }
}

function excluirNao() {
  document.getElementById("divExcluir").style.display = "none";
}

async function sendData(btn, id) {
  let modificaPara = "";
  if (document.getElementById(btn).className == "btn btn-dark") {
    modificaPara = "1";
  } else if (document.getElementById(btn).className == "btn btn-warning") {
    modificaPara = "0";
  }
  try {
    const response = await fetch(
      `device/status?id=${encodeURIComponent(id)}&status=${encodeURIComponent(
        modificaPara,
      )}`,
      {
        method: "PUT",
      },
    );

    if (!response.ok) {
      throw new Error("Erro ao modificar estado");
    }

    const data = await response.json();

    if (data.status == "0") {
      document.getElementById(btn).className = "btn btn-dark";
    } else {
      document.getElementById(btn).className = "btn btn-warning";
    }
  } catch (error) {
    alert("Erro ao modificar estado: " + error);
  }
}

// Alarmes início
async function procuraAlarmes(id) {
  try {
    const response = await fetch(
      `schedule?deviceId=${encodeURIComponent(id)}`,
      {
        method: "GET",
      },
    );
    if (!response.ok) {
      throw new Error("Erro ao recuperar informações");
    }

    const data = await response.json();

    listaAlarmes(data);
  } catch (error) {
    alert(error.message);
  }
}

function listaAlarmes(schedulesJson) {
  //Select com a lista de pinos
  var listaAlarmes = document.getElementById("sessao_alarmes");

  while (listaAlarmes.hasChildNodes()) {
    listaAlarmes.removeChild(listaAlarmes.firstChild);
  }

  // Se não há alarmes, exibe mensagem
  if (!schedulesJson || schedulesJson.length === 0) {
    var divMsg = document.createElement("div");
    divMsg.className = "alert alert-info";
    divMsg.innerText = "Nenhum alarme registrado.";
    listaAlarmes.appendChild(divMsg);
    return;
  }

  for (var i = 0; i < schedulesJson.length; i++) {
    let schedule = schedulesJson[i];

    var linhaPrincipal = document.createElement("div");
    linhaPrincipal.className = "row align-items-center div-dispositivos";
    linhaPrincipal.id = "linhaAlarme" + schedule.id;

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

    // Nome
    var inputNome = document.createElement("input");
    inputNome.id = "nomeAlarme" + schedule.id;
    inputNome.type = "text";
    inputNome.className = "form-control";
    inputNome.value = schedule.name;
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
    selectHora.id = "selectHora" + schedule.id;
    for (var x = 0; x <= 23; x++) {
      var optionHora = document.createElement("option");
      optionHora.innerHTML = x;
      optionHora.value = x;
      selectHora.appendChild(optionHora);
    }
    selectHora.selectedIndex = schedule.hour;

    var spanMinuto = document.createElement("label");
    spanMinuto.innerHTML = "Minuto";
    var selectMinuto = document.createElement("select");
    selectMinuto.className = "form-control";
    selectMinuto.id = "selectMinuto" + schedule.id;
    for (var y = 0; y <= 59; y++) {
      var optionMinuto = document.createElement("option");
      optionMinuto.innerHTML = y;
      optionMinuto.value = y;
      selectMinuto.appendChild(optionMinuto);
    }
    selectMinuto.selectedIndex = schedule.minute;

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
    selectAcao.id = "selectAcao" + schedule.id;

    var optionAcao01 = document.createElement("option");
    optionAcao01.innerHTML = "Desligar";
    optionAcao01.value = "0";
    selectAcao.appendChild(optionAcao01);

    var optionAcao02 = document.createElement("option");
    optionAcao02.innerHTML = "Ligar";
    optionAcao02.value = "1";
    selectAcao.appendChild(optionAcao02);
    selectAcao.selectedIndex = schedule.action;

    var spanAtivo = document.createElement("label");
    spanAtivo.innerHTML = "Estado";
    var selectAtivo = document.createElement("select");
    selectAtivo.className = "form-control";
    selectAtivo.id = "selectAtivo" + schedule.id;

    var optionAtivo01 = document.createElement("option");
    optionAtivo01.innerHTML = "Desativado";
    optionAtivo01.value = "0";
    selectAtivo.appendChild(optionAtivo01);

    var optionAtivo02 = document.createElement("option");
    optionAtivo02.innerHTML = "Ativado";
    optionAtivo02.value = "1";
    selectAtivo.appendChild(optionAtivo02);
    selectAtivo.selectedIndex = schedule.active;

    colunaAcao.appendChild(spanAcao);
    colunaAcao.appendChild(selectAcao);
    colunaAtivo.appendChild(spanAtivo);
    colunaAtivo.appendChild(selectAtivo);

    // Salvar
    var colunaSalvar = document.createElement("div");
    colunaSalvar.className = "col-12";

    var btnSalvar = document.createElement("button");
    btnSalvar.className = "btn btn-success";
    btnSalvar.id = "btnSalvar" + schedule.id;
    btnSalvar.setAttribute("onclick", "editarAlarme('" + schedule.id + "')");

    var spanBotaoSalvar = document.createElement("span");
    spanBotaoSalvar.className = "oi oi-task";

    btnSalvar.appendChild(spanBotaoSalvar);

    var divProgressoSalvar = document.createElement("div");
    divProgressoSalvar.className = "spinner-border text-success";
    divProgressoSalvar.id = "progresso" + schedule.id;
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
    btnExcluir.id = "btnExcluir" + schedule.id;
    btnExcluir.setAttribute("onclick", "excluirAlarme('" + schedule.id + "')");

    var spanBotaoExcluir = document.createElement("span");
    spanBotaoExcluir.className = "oi oi-trash";

    btnExcluir.appendChild(spanBotaoExcluir);

    var divProgressoExcluir = document.createElement("div");
    divProgressoExcluir.className = "spinner-border text-danger";
    divProgressoExcluir.id = "progressoExcluir" + schedule.id;
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

// Adicionar um alarme (novo)
async function novoAlarme() {
  try {
    document.getElementById("novoAlarmeBtns").style.display = "none";
    document.getElementById("novoAlarmeAguarde").style.display = "inline-block";

    var deviceId = document.getElementById("deviceIdNovo").value;
    var name = document.getElementById("nomeNovoAlarme").value;
    var hour = String(document.getElementById("selectHoraNovo").selectedIndex);
    var minute = String(
      document.getElementById("selectMinutoNovo").selectedIndex,
    );
    var action = document.getElementById("selectAcaoNovo").selectedIndex;
    var active = document.getElementById("selectAtivoNovo").selectedIndex;

    var body = {
      deviceId,
      name,
      hour,
      minute,
      action,
      active,
    };

    const response = await fetch("/schedule", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(body),
    });

    if (response.ok) {
      $("#modalNovoAlarme").modal("hide");
      var deviceName =
        document.getElementById("nomeDispNovoAlarme").innerHTML || "";
      $("#modalAlarmes").data("id", deviceId);
      $("#modalAlarmes").data("nome", deviceName);
      $("#modalAlarmes").modal("show");
      showToast("Alarme criado com sucesso.");

      document.getElementById("novoAlarmeBtns").style.display = "inline-block";
      document.getElementById("novoAlarmeAguarde").style.display = "none";
    } else {
      const data = await response.json();

      document.getElementById("novoAlarmeBtns").style.display = "inline-block";
      document.getElementById("novoAlarmeAguarde").style.display = "none";

      alert("Erro ao criar alarme: " + data?.message);
    }
  } catch (error) {
    document.getElementById("novoAlarmeBtns").style.display = "inline-block";
    document.getElementById("novoAlarmeAguarde").style.display = "none";

    alert("Erro ao adicionar alarme: " + error);
  }
}

function showToast(message) {
  var toastEl = document.getElementById("toastAlarme");
  if (!toastEl) return;
  toastEl.querySelector(".toast-body").innerText = message;
  $("#toastAlarme").toast({ delay: 2000 });
  $("#toastAlarme").toast("show");
}

// Editar um alarme
async function editarAlarme(id) {
  var name = document.getElementById("nomeAlarme" + id).value;
  var hour = String(document.getElementById("selectHora" + id).selectedIndex);
  var minute = String(
    document.getElementById("selectMinuto" + id).selectedIndex,
  );
  var action = document.getElementById("selectAcao" + id).selectedIndex;
  var active = document.getElementById("selectAtivo" + id).selectedIndex;

  var body = {
    name,
    hour,
    minute,
    action,
    active,
  };

  document.getElementById("btnSalvar" + id).style.display = "none";
  document.getElementById("progresso" + id).style.display = "inline-block";

  try {
    const response = await fetch(`schedule?id=${encodeURIComponent(id)}`, {
      method: "PUT",
      body: JSON.stringify(body),
      headers: {
        "Content-Type": "application/json",
      },
    });
    if (response.ok) {
      document.getElementById("progresso" + id).style.display = "none";
      document.getElementById("btnSalvar" + id).style.display = "inline-block";
      showToast("Alarme editado com sucesso.");
    }
  } catch (error) {
    alert("Erro ao editar alarme: " + error);
    document.getElementById("progresso" + id).style.display = "none";
    document.getElementById("btnSalvar" + id).style.display = "inline-block";
  }
}

// Excluir um alarme
async function excluirAlarme(id) {
  document.getElementById("btnExcluir" + id).style.display = "none";
  document.getElementById("progressoExcluir" + id).style.display =
    "inline-block";

  try {
    const response = await fetch(`schedule?id=${encodeURIComponent(id)}`, {
      method: "DELETE",
    });

    if (response.ok) {
      document.getElementById("progressoExcluir" + id).style.display = "none";
      document.getElementById("linhaAlarme" + id).style.opacity = 0;
      document.getElementById("linhaAlarme" + id).style.display = "none";
      showToast("Alarme excluído com sucesso.");
    }
  } catch (error) {
    alert("Erro ao excluir alarme: " + error);
    document.getElementById("progressoExcluir" + id).style.display = "none";
    document.getElementById("btnExcluir" + id).style.display = "inline-block";
  }
}

// Alarmes fim
