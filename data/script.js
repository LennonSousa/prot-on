async function getStatusLocal() {
  try {
    const response = await fetch("device", {
      method: "GET",
    });

    if (!response.ok) {
      throw new Error(t("error_fetch"));
    }

    const data = await response.json();

    criaListaDisp(data);
  } catch (error) {
    alert(error.message);
  }
}

// --- i18n translations ---
const supportedLangs = ["pt", "en"];
let currentLang = localStorage.getItem("lang") || "pt";
let translations = {};

async function loadTranslations() {
  try {
    const res = await fetch("/translations.json");
    if (res.ok) translations = await res.json();
  } catch (e) {
    console.warn("Failed to load translations.json", e);
  }
}

function t(key) {
  return (translations[currentLang] && translations[currentLang][key]) || key;
}

function getFlagSVG(lang) {
  if (lang === "pt") {
    return '<img src="/flag_pt.png" alt="Português" class="flag-img" onerror="fallbackFlag(this, \"pt\")" />';
  }
  return '<img src="/flag_en.png" alt="English" class="flag-img" onerror="fallbackFlag(this, \"en\")" />';
}

function fallbackFlag(imgElem, lang) {
  try {
    var span = document.createElement("span");
    span.className = "flag " + (lang === "pt" ? "flag-pt" : "flag-en");

    if (lang === "pt") {
      var parts = [
        "pt-diamond",
        "pt-circle",
        "pt-band",
        "pt-star pt-star-1",
        "pt-star pt-star-2",
        "pt-star pt-star-3",
        "pt-star pt-star-4",
        "pt-star pt-star-5",
      ];
      parts.forEach(function (cls) {
        var s = document.createElement("span");
        s.className = cls;
        span.appendChild(s);
      });
    } else {
      var parts = [
        "en-saltire-white",
        "en-saltire-red",
        "en-vert",
        "en-vert-red",
        "en-horz",
        "en-horz-red",
      ];
      parts.forEach(function (cls) {
        var s = document.createElement("span");
        s.className = cls;
        span.appendChild(s);
      });
    }

    imgElem.parentNode.replaceChild(span, imgElem);
  } catch (e) {
    console.warn("fallbackFlag error", e);
  }
}

function applyTranslations() {
  document.title = t("title");

  var btnAdd = document.getElementById("tooltipAddDevice");
  if (btnAdd) btnAdd.setAttribute("title", t("add_device"));

  document.querySelectorAll('[title="Instagram"]').forEach(function (el) {
    el.setAttribute("title", t("instagram"));
  });
  document.querySelectorAll('[title="Facebook"]').forEach(function (el) {
    el.setAttribute("title", t("facebook"));
  });

  $('[data-toggle="tooltip"]').tooltip("dispose").tooltip();

  // modal editar
  var ssid = document.getElementById("ssidRede");
  if (ssid) ssid.innerHTML = t("edit");
  var labelNomeDisp = document.getElementById("labelNomeDisp");
  if (labelNomeDisp) labelNomeDisp.innerText = t("device_name");
  var labelIpDisp = document.getElementById("labelIpDisp");
  if (labelIpDisp) labelIpDisp.innerText = t("ip");
  var editarSaveBtn = document.getElementById("editarSaveBtn");
  if (editarSaveBtn) editarSaveBtn.innerText = t("save");
  var btExcluir = document.getElementById("btExcluir");
  if (btExcluir) btExcluir.innerText = t("delete");
  var editarCloseBtn = document.getElementById("editarCloseBtn");
  if (editarCloseBtn) editarCloseBtn.innerText = t("close");

  // modal novo dispositivo
  var novoSalvarBtn = document.getElementById("novoSalvarBtn");
  if (novoSalvarBtn) novoSalvarBtn.innerText = t("save");
  var novoCloseBtn = document.getElementById("novoCloseBtn");
  if (novoCloseBtn) novoCloseBtn.innerText = t("close");
  var labelNomeNovoDisp = document.getElementById("labelNomeNovoDisp");
  if (labelNomeNovoDisp) labelNomeNovoDisp.innerText = t("device_name");
  var modalAddTitle = document.getElementById("modalAddTitle");
  if (modalAddTitle) modalAddTitle.innerText = t("new");
  var labelIpNovoDisp = document.getElementById("labelIpNovoDisp");
  if (labelIpNovoDisp) labelIpNovoDisp.innerText = t("ip");
  var novoCampoObrigatorio = document.getElementById("novoCampoObrigatorio");
  if (novoCampoObrigatorio)
    novoCampoObrigatorio.innerText = t("field_required");
  var novoPaginaAviso = document.getElementById("novoPaginaAviso");
  if (novoPaginaAviso) novoPaginaAviso.innerText = t("save_will_refresh");

  // modal alarmes / novo alarme
  var btnNovo = document.getElementById("btnNovoAlarme");
  if (btnNovo) btnNovo.innerText = t("new");
  var labelNomeNovoAlarme = document.getElementById("labelNomeNovoAlarme");
  if (labelNomeNovoAlarme) labelNomeNovoAlarme.innerText = t("alarm_name");
  var nomeNovoAlarmeFeedback = document.getElementById(
    "nomeNovoAlarmeFeedback",
  );
  if (nomeNovoAlarmeFeedback)
    nomeNovoAlarmeFeedback.innerText = t("alarm_name_required");
  var labelHoraNovo = document.getElementById("labelHoraNovo");
  if (labelHoraNovo) labelHoraNovo.innerText = t("hour");
  var labelMinutoNovo = document.getElementById("labelMinutoNovo");
  if (labelMinutoNovo) labelMinutoNovo.innerText = t("minute");
  var labelAcaoNovo = document.getElementById("labelAcaoNovo");
  if (labelAcaoNovo) labelAcaoNovo.innerText = t("action");
  var labelAtivoNovo = document.getElementById("labelAtivoNovo");
  if (labelAtivoNovo) labelAtivoNovo.innerText = t("status");
  var novoAlarmeSalvarBtn = document.getElementById("novoAlarmeSalvarBtn");
  if (novoAlarmeSalvarBtn) novoAlarmeSalvarBtn.innerText = t("save");
  var novoAlarmeCloseBtn = document.getElementById("novoAlarmeCloseBtn");
  if (novoAlarmeCloseBtn) novoAlarmeCloseBtn.innerText = t("close");

  var selectAcaoNovo = document.getElementById("selectAcaoNovo");
  if (selectAcaoNovo) {
    if (selectAcaoNovo.options.length >= 2) {
      selectAcaoNovo.options[0].innerHTML = t("off");
      selectAcaoNovo.options[1].innerHTML = t("on");
    }
  }
  var selectAtivoNovo = document.getElementById("selectAtivoNovo");
  if (selectAtivoNovo) {
    if (selectAtivoNovo.options.length >= 2) {
      selectAtivoNovo.options[0].innerHTML = t("disabled");
      selectAtivoNovo.options[1].innerHTML = t("enabled");
    }
  }

  // toast
  var toastSmall = document.getElementById("toastSmall");
  if (toastSmall) toastSmall.innerText = t("now");
  var toastTitle = document.getElementById("toastTitle");
  if (toastTitle) toastTitle.innerText = "Prot-On";
  var toastBody = document.getElementById("toastBody");
  if (toastBody) toastBody.innerText = t("alarm_created");

  document.querySelectorAll(".sr-only").forEach(function (el) {
    el.innerText = t("waiting");
  });

  var langLabelIcon = document.getElementById("langLabelIcon");
  var langLabelText = document.getElementById("langLabelText");
  if (langLabelIcon && langLabelText) {
    if (currentLang === "pt") {
      langLabelIcon.innerHTML = getFlagSVG("pt");
      langLabelText.innerText = t("lang_pt");
    } else {
      langLabelIcon.innerHTML = getFlagSVG("en");
      langLabelText.innerText = t("lang_en");
    }
  }
  var langOptPt = document.getElementById("langOptPt");
  if (langOptPt) {
    langOptPt.innerHTML =
      '<span class="lang-icon">' +
      getFlagSVG("pt") +
      '</span><span class="lang-text">' +
      t("lang_pt") +
      "</span>";
    if (currentLang === "pt") langOptPt.classList.add("active");
    else langOptPt.classList.remove("active");
  }
  var langOptEn = document.getElementById("langOptEn");
  if (langOptEn) {
    langOptEn.innerHTML =
      '<span class="lang-icon">' +
      getFlagSVG("en") +
      '</span><span class="lang-text">' +
      t("lang_en") +
      "</span>";
    if (currentLang === "en") langOptEn.classList.add("active");
    else langOptEn.classList.remove("active");
  }

  document.querySelectorAll('[aria-label="Fechar"]').forEach(function (el) {
    el.setAttribute("aria-label", t("close"));
  });

  var salvandoAviso = document.getElementById("salvandoAviso");
  if (salvandoAviso) salvandoAviso.innerText = t("save_will_refresh");

  document.querySelectorAll("button").forEach(function (b) {
    var txt = (b.innerText || "").trim();
    if (txt === "Fechar" || txt === "Close") b.innerText = t("close");
    if (txt === "Salvar" || txt === "Save") b.innerText = t("save");
    if (txt === "Excluir" || txt === "Delete") b.innerText = t("delete");
    if (txt === "Novo" || txt === "New") b.innerText = t("new");
  });

  var excluirAviso = document.getElementById("excluirAviso");
  if (excluirAviso) excluirAviso.innerText = t("delete_confirm");
  var btExcluirSim = document.getElementById("btExcluirSim");
  if (btExcluirSim) btExcluirSim.innerText = t("yes");
  var btExcluirNao = document.getElementById("btExcluirNao");
  if (btExcluirNao) btExcluirNao.innerText = t("no");

  var welcomeTitle = document.getElementById("welcomeTitle");
  if (welcomeTitle) welcomeTitle.innerText = t("welcome");
  var welcomeIntro = document.getElementById("welcomeIntro");
  if (welcomeIntro) welcomeIntro.innerText = t("welcome_intro");
  var followSteps = document.getElementById("followSteps");
  if (followSteps) followSteps.innerText = t("follow_steps");
  var step1Btn = document.getElementById("step1Btn");
  if (step1Btn) step1Btn.innerText = t("step1_title");
  var step2Btn = document.getElementById("step2Btn");
  if (step2Btn) step2Btn.innerText = t("step2_title");
  var step3Btn = document.getElementById("step3Btn");
  if (step3Btn) step3Btn.innerText = t("step3_title");
  var labelNomeDisp0 = document.getElementById("labelNomeDisp0");
  if (labelNomeDisp0) labelNomeDisp0.innerText = t("dont_worry");
  var nomeDisp0 = document.getElementById("nomeDisp0");
  if (nomeDisp0) nomeDisp0.placeholder = t("input_placeholder");
  var emailHelp = document.getElementById("emailHelp");
  if (emailHelp) emailHelp.innerText = t("example_help");
  var btSavePrimeiro = document.getElementById("btSavePrimeiro");
  if (btSavePrimeiro) btSavePrimeiro.innerText = t("save");
  var btProcuraRedes = document.getElementById("btProcuraRedes");
  if (btProcuraRedes) btProcuraRedes.innerText = t("search_networks");
  var labelSenhaRede = document.getElementById("labelSenhaRede");
  if (labelSenhaRede) labelSenhaRede.innerText = t("enter_password");
  var btConectar = document.getElementById("btConectar");
  if (btConectar) btConectar.innerText = t("connect");
  var btFinaliza = document.getElementById("btFinaliza");
  if (btFinaliza) btFinaliza.innerText = t("finalize");
  var passo3Lb = document.getElementById("passo3Lb");
  if (passo3Lb) passo3Lb.innerText = t("dont_worry");
  var navHomeLink = document.getElementById("navHomeLink");
  if (navHomeLink)
    navHomeLink.innerHTML =
      t("home") + ' <span class="sr-only">' + t("current") + "</span>";

  try {
    if (window.$ && $("#modalAlarmes").is(":visible")) {
      var id = $("#modalAlarmes").data("id");
      if (id) procuraAlarmes(id);
    }
  } catch (e) {}

  document.documentElement.lang = currentLang === "en" ? "en" : "pt-br";
}

function setLang(lang) {
  if (supportedLangs.indexOf(lang) !== -1) {
    currentLang = lang;
    localStorage.setItem("lang", lang);
    applyTranslations();
  }
}

window.addEventListener("DOMContentLoaded", async function () {
  await loadTranslations();

  var langBtn = document.getElementById("langDropdownBtn");
  if (langBtn) {
    var langLabel = document.getElementById("langLabel");
    if (langLabel)
      langLabel.innerText = currentLang === "pt" ? t("lang_pt") : t("lang_en");
    document.querySelectorAll(".lang-option").forEach(function (el) {
      el.addEventListener("click", function (e) {
        e.preventDefault();
        var l = el.getAttribute("data-lang");
        setLang(l);
      });
    });
  } else {
    var sel = document.getElementById("langSelect");
    if (sel) {
      sel.value = currentLang;
      sel.addEventListener("change", function (e) {
        setLang(e.target.value);
      });
    }
  }
  applyTranslations();
});

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

      alert(t("error_create_device") + " " + data?.message);
    }
  } catch (error) {
    document.getElementById("novoDispBtns").style.display = "inline-block";
    document.getElementById("novoDispAguarde").style.display = "none";

    alert(t("error_add_device") + " " + error);
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

      alert(t("error_edit_device") + " " + data?.message);
    }
  } catch (error) {
    document.getElementById("editarDispBtns").style.display = "inline-block";
    document.getElementById("editarDispAguarde").style.display = "none";

    alert(t("error_edit_device") + " " + error);
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

      alert(t("error_delete_device") + " " + data?.message);
    }
  } catch (error) {
    document.getElementById("editarDispBtns").style.display = "inline-block";
    document.getElementById("editarDispAguarde").style.display = "none";

    alert(t("error_delete_device") + " " + error);
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
      throw new Error(t("error_change_state"));
    }

    const data = await response.json();

    if (data.status == "0") {
      document.getElementById(btn).className = "btn btn-dark";
    } else {
      document.getElementById(btn).className = "btn btn-warning";
    }
  } catch (error) {
    alert(t("error_change_state") + " " + error);
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
      throw new Error(t("error_fetch"));
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
    divMsg.innerText = t("no_alarms");
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

    inputNome.oninput = function () {
      this.classList.remove("is-invalid");
      var fb = document.getElementById("nomeAlarmeFeedback" + schedule.id);
      if (fb) fb.innerText = t("alarm_name_required");
    };
    colunaNome.appendChild(inputNome);

    var divFeedback = document.createElement("div");
    divFeedback.className = "invalid-feedback";
    divFeedback.id = "nomeAlarmeFeedback" + schedule.id;
    divFeedback.innerText = t("alarm_name_required");
    colunaNome.appendChild(divFeedback);

    // Horário
    var colunaHora = document.createElement("div");
    colunaHora.className = "col-6";

    var colunaMinuto = document.createElement("div");
    colunaMinuto.className = "col-6";

    var spanHora = document.createElement("label");
    spanHora.innerHTML = t("hour");
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
    spanMinuto.innerHTML = t("minute");
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
    optionAcao01.innerHTML = t("off");
    optionAcao01.value = "0";
    selectAcao.appendChild(optionAcao01);

    var optionAcao02 = document.createElement("option");
    optionAcao02.innerHTML = t("on");
    optionAcao02.value = "1";
    selectAcao.appendChild(optionAcao02);
    selectAcao.selectedIndex = schedule.action;

    var spanAtivo = document.createElement("label");
    spanAtivo.innerHTML = "Estado";
    var selectAtivo = document.createElement("select");
    selectAtivo.className = "form-control";
    selectAtivo.id = "selectAtivo" + schedule.id;

    var optionAtivo01 = document.createElement("option");
    optionAtivo01.innerHTML = t("disabled");
    optionAtivo01.value = "0";
    selectAtivo.appendChild(optionAtivo01);

    var optionAtivo02 = document.createElement("option");
    optionAtivo02.innerHTML = t("enabled");
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
    var nameInput = document.getElementById("nomeNovoAlarme");
    var name = nameInput ? nameInput.value.trim() : "";

    if (!name) {
      if (nameInput) {
        nameInput.classList.add("is-invalid");
        var fb = document.getElementById("nomeNovoAlarmeFeedback");
        if (fb) fb.innerText = t("alarm_name_required");
        nameInput.focus();
      }
      document.getElementById("novoAlarmeBtns").style.display = "inline-block";
      document.getElementById("novoAlarmeAguarde").style.display = "none";
      return;
    }

    var hour = String(document.getElementById("selectHoraNovo").selectedIndex);
    var minute = String(
      document.getElementById("selectMinutoNovo").selectedIndex,
    );

    var actionEl = document.getElementById("selectAcaoNovo");
    var action =
      actionEl && actionEl.value !== undefined
        ? String(actionEl.value)
        : String(actionEl.selectedIndex);
    var activeEl = document.getElementById("selectAtivoNovo");
    var active =
      activeEl && activeEl.value !== undefined
        ? String(activeEl.value)
        : String(activeEl.selectedIndex);

    if (action !== "0" && action !== "1") action = "0";
    if (active !== "0" && active !== "1") active = "1";

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
      showToast(t("alarm_created"));

      document.getElementById("novoAlarmeBtns").style.display = "inline-block";
      document.getElementById("novoAlarmeAguarde").style.display = "none";
    } else {
      const data = await response.json();

      document.getElementById("novoAlarmeBtns").style.display = "inline-block";
      document.getElementById("novoAlarmeAguarde").style.display = "none";

      alert(t("error_create_alarm") + " " + data?.message);
    }
  } catch (error) {
    document.getElementById("novoAlarmeBtns").style.display = "inline-block";
    document.getElementById("novoAlarmeAguarde").style.display = "none";

    alert(t("error_add_alarm") + " " + error);
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
  var nameInput = document.getElementById("nomeAlarme" + id);
  var name = nameInput ? nameInput.value.trim() : "";
  if (!name) {
    if (nameInput) {
      nameInput.classList.add("is-invalid");
      var fb = document.getElementById("nomeAlarmeFeedback" + id);
      if (fb) fb.innerText = t("alarm_name_required");
      nameInput.focus();
    }
    return;
  }

  var hour = String(document.getElementById("selectHora" + id).selectedIndex);
  var minute = String(
    document.getElementById("selectMinuto" + id).selectedIndex,
  );

  var actionEl = document.getElementById("selectAcao" + id);
  var action =
    actionEl && actionEl.value !== undefined
      ? String(actionEl.value)
      : String(actionEl.selectedIndex);
  var activeEl = document.getElementById("selectAtivo" + id);
  var active =
    activeEl && activeEl.value !== undefined
      ? String(activeEl.value)
      : String(activeEl.selectedIndex);

  if (action !== "0" && action !== "1") action = "0";
  if (active !== "0" && active !== "1") active = "1";

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
      showToast(t("alarm_edited"));
    }
  } catch (error) {
    alert(t("error_edit_alarm") + " " + error);
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
      showToast(t("alarm_deleted"));
    }
  } catch (error) {
    alert(t("error_delete_alarm") + " " + error);
    document.getElementById("progressoExcluir" + id).style.display = "none";
    document.getElementById("btnExcluir" + id).style.display = "inline-block";
  }
}

// Alarmes fim
