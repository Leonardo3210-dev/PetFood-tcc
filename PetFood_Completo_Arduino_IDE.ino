#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <time.h>

// ===== PETFOOD: PINOS DO HARDWARE =====
const int PINO_SERVO = 18;
const int PINO_TRIG = 23;
const int PINO_ECHO = 19;  // ECHO do HC-SR04 deve chegar ao ESP32 por divisor de tensão

// ===== REDE WI-FI DO PETFOOD =====
const char* WIFI_AP_NOME = "PetFood";
const char* WIFI_AP_SENHA = "12345678";

// ===== CONFIGURACOES =====
const float ALTURA_RESERVATORIO_CM = 20.0f;
const int ANGULO_FECHADO = 0;
const int ANGULO_ABERTO = 90;
const unsigned long TIMEOUT_ECHO_US = 30000UL;
const int MAX_HORARIOS = 8;

// Manaus: UTC-4, sem horario de verao.
const long GMT_OFFSET_SEC = -4L * 60L * 60L;
const int DAYLIGHT_OFFSET_SEC = 0;

WebServer server(80);
Servo servoRacao;
Preferences preferences;

String horarios[MAX_HORARIOS];
int quantidadeHorarios = 0;
unsigned long duracaoServoMs = 1500;
String wifiSSIDSalvo = "";
String wifiSenhaSalva = "";
String ultimoSSIDTentado = "";
bool servoAberto = false;
unsigned long fecharServoEm = 0;
unsigned long ultimaLeituraSensor = 0;
float ultimaDistanciaCm = -1.0f;
int ultimoNivelPercentual = -1;
bool ultimaLeituraValida = false;
int ultimoMinutoExecutado = -1;
int ultimoDiaExecutado = -1;

// O HTML/CSS/JavaScript do painel fica embutido neste mesmo arquivo .ino.
const char pagina[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#123c2d">
  <title>PetFood | Alimentação inteligente</title>
  <style>
    :root{--green:#176b4d;--green2:#23845e;--mint:#e8f6ee;--ink:#19352a;--muted:#718078;--bg:#f4f7f4;--card:#fff;--line:#e4ebe5;--orange:#f3a640;--red:#c84949;--shadow:0 12px 35px rgba(25,53,42,.07)}
    *{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font-family:Inter,ui-sans-serif,system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}
    button,input,select{font:inherit}button{cursor:pointer}button:disabled{opacity:.55;cursor:not-allowed}
    .app{min-height:100vh;display:grid;grid-template-columns:245px 1fr}
    aside{background:#123c2d;color:#fff;padding:27px 18px;display:flex;flex-direction:column;gap:28px}
    .brand{display:flex;align-items:center;gap:12px;padding:0 8px}.logo{width:43px;height:43px;border-radius:14px;background:#d7f3df;color:#176b4d;display:grid;place-items:center;font-size:25px}.brand h1{font-size:21px;margin:0;letter-spacing:-.5px}.brand p{margin:3px 0 0;color:#b9d9c8;font-size:11px}
    .nav-label{color:#8fb6a1;text-transform:uppercase;font-size:10px;font-weight:800;letter-spacing:1.4px;padding:0 12px}
    nav{display:grid;gap:7px}.nav-btn{border:0;background:transparent;color:#c7dfd1;padding:13px 14px;text-align:left;border-radius:12px;display:flex;align-items:center;gap:12px;font-weight:650}.nav-btn .ico{font-size:19px;width:22px;text-align:center}.nav-btn.active,.nav-btn:hover{background:#245742;color:white}
    .side-bottom{margin-top:auto;border:1px solid #35644f;border-radius:15px;padding:14px;background:#194632}.side-bottom strong{display:block;font-size:13px}.side-bottom small{display:block;color:#b5d4c1;margin-top:6px;line-height:1.5}
    main{padding:30px clamp(18px,4vw,48px);min-width:0;max-width:1500px;width:100%;margin:0 auto}.topbar{display:flex;justify-content:space-between;align-items:center;gap:15px;margin-bottom:27px}.eyebrow{font-size:12px;color:var(--muted);font-weight:700}.topbar h2{margin:5px 0 0;font-size:clamp(23px,3vw,31px);letter-spacing:-1px}.top-actions{display:flex;align-items:center;gap:10px}.connection{display:flex;align-items:center;gap:8px;background:#fff;border:1px solid var(--line);padding:10px 13px;border-radius:30px;font-size:12px;font-weight:700}.dot{width:8px;height:8px;border-radius:50%;background:#d39a37}.dot.online{background:#2aaf70;box-shadow:0 0 0 4px #e3f7eb}.icon-button{border:1px solid var(--line);background:#fff;width:41px;height:41px;border-radius:13px}
    .page{display:none}.page.active{display:block}.welcome{display:flex;align-items:center;justify-content:space-between;gap:20px;background:linear-gradient(115deg,#176b4d,#2d956b);border-radius:22px;padding:26px 29px;color:white;overflow:hidden;position:relative;margin-bottom:20px}.welcome:after{content:"🐾";position:absolute;right:24%;font-size:100px;opacity:.11;transform:rotate(-20deg)}.welcome h3{font-size:25px;margin:0 0 8px;letter-spacing:-.6px}.welcome p{margin:0;color:#d8f0e2;max-width:520px;line-height:1.55;font-size:14px}.welcome .primary{position:relative;z-index:1;white-space:nowrap;background:white;color:var(--green);border:0;border-radius:12px;padding:13px 17px;font-weight:800}
    .grid{display:grid;gap:17px}.stats{grid-template-columns:repeat(3,minmax(0,1fr));margin-bottom:18px}.card{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:21px;box-shadow:var(--shadow);min-width:0}.stat-top{display:flex;justify-content:space-between;align-items:center;color:var(--muted);font-size:12px;font-weight:700}.stat-icon{width:38px;height:38px;border-radius:12px;background:var(--mint);display:grid;place-items:center;font-size:19px}.stat-value{font-size:29px;font-weight:850;letter-spacing:-1px;margin:12px 0 5px}.stat-note{font-size:12px;color:var(--muted)}.stat-note.good{color:var(--green2)}
    .main-grid{grid-template-columns:minmax(0,1.35fr) minmax(280px,.85fr)}.card-head{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:17px}.card h3{margin:0;font-size:16px;letter-spacing:-.25px}.card-head p{margin:5px 0 0;color:var(--muted);font-size:12px}.text-button{border:0;background:transparent;color:var(--green);font-weight:800;font-size:12px}
    .level-wrap{display:flex;align-items:center;gap:20px}.level-ring{width:120px;height:120px;border-radius:50%;display:grid;place-items:center;flex-shrink:0;background:conic-gradient(var(--green2) var(--level,70%),#e7eee8 0);position:relative}.level-ring:before{content:"";position:absolute;inset:10px;border-radius:50%;background:#fff}.level-ring div{position:relative;text-align:center}.level-ring strong{display:block;font-size:26px;letter-spacing:-1px}.level-ring small{font-size:10px;color:var(--muted)}.level-details{flex:1}.level-details strong{font-size:15px}.level-details p{color:var(--muted);font-size:12px;line-height:1.55}.status-pill{display:inline-flex;align-items:center;gap:6px;padding:7px 10px;border-radius:20px;background:var(--mint);color:var(--green);font-size:11px;font-weight:800}.status-pill.warn{background:#fff2dc;color:#93601a}.status-pill.bad{background:#fde9e9;color:var(--red)}
    .feed-actions{display:grid;grid-template-columns:repeat(3,1fr);gap:9px;margin:15px 0}.portion{border:1px solid var(--line);border-radius:12px;padding:12px 5px;background:#fff;color:var(--ink);font-weight:800}.portion small{display:block;color:var(--muted);font-size:10px;font-weight:600;margin-top:3px}.portion.selected{border-color:var(--green2);background:var(--mint);color:var(--green)}.feed-btn{width:100%;padding:14px;border:0;border-radius:12px;background:var(--green);color:#fff;font-weight:800}.feed-btn:hover,.primary:hover{filter:brightness(1.04)}.hint{font-size:11px;color:var(--muted);line-height:1.5;margin:10px 0 0}
    .meal-list{display:grid;gap:10px}.meal{display:flex;align-items:center;gap:12px;padding:12px;border:1px solid var(--line);border-radius:13px}.meal-icon{width:40px;height:40px;border-radius:12px;background:#fff1d9;display:grid;place-items:center;font-size:19px}.meal-info{flex:1}.meal-info strong{font-size:13px;display:block}.meal-info small{font-size:11px;color:var(--muted)}.meal-time{font-weight:850;font-size:14px}
    .section-title{margin:0 0 6px;font-size:22px;letter-spacing:-.6px}.section-subtitle{margin:0 0 22px;color:var(--muted);font-size:13px}.schedule-layout{grid-template-columns:minmax(0,1.2fr) minmax(260px,.8fr)}.form-row{display:grid;grid-template-columns:1fr 1fr;gap:12px}.field{display:grid;gap:7px;margin-bottom:14px}.field label{font-size:12px;font-weight:750;color:#455b4d}.field input,.field select{width:100%;padding:12px;border:1px solid #dbe5dd;border-radius:11px;background:#fff;color:var(--ink);outline:none}.field input:focus,.field select:focus{border-color:var(--green2);box-shadow:0 0 0 3px #e3f5e9}.form-actions{display:flex;gap:9px;flex-wrap:wrap}.btn{border:0;border-radius:11px;padding:12px 15px;font-weight:800}.btn.green{background:var(--green);color:#fff}.btn.light{background:var(--mint);color:var(--green)}.btn.danger{background:#fff0f0;color:var(--red)}.schedule-item{display:flex;align-items:center;gap:12px;border:1px solid var(--line);border-radius:13px;padding:13px;margin-bottom:10px}.schedule-time{font-size:18px;font-weight:850;min-width:72px}.schedule-desc{flex:1}.schedule-desc strong{font-size:12px;display:block}.schedule-desc small{color:var(--muted);font-size:11px}.remove{border:0;background:#fff0f0;color:var(--red);border-radius:9px;width:34px;height:34px;font-weight:900}
    .settings-grid{grid-template-columns:repeat(2,minmax(0,1fr))}.status-row{display:flex;justify-content:space-between;gap:12px;padding:13px 0;border-bottom:1px solid var(--line);font-size:13px}.status-row:last-child{border-bottom:0}.status-row span{color:var(--muted)}.history-empty{padding:30px 15px;text-align:center;color:var(--muted);font-size:13px}.history-item{padding:13px 0;border-bottom:1px solid var(--line);display:flex;justify-content:space-between;gap:12px;font-size:13px}.history-item:last-child{border-bottom:0}.history-item small{display:block;color:var(--muted);margin-top:4px}
    .toast{position:fixed;bottom:22px;right:22px;background:#173e2e;color:white;padding:14px 18px;border-radius:13px;box-shadow:var(--shadow);max-width:min(380px,calc(100vw - 35px));font-size:13px;z-index:10;display:none}.toast.show{display:block;animation:fade .2s ease}@keyframes fade{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:none}}.mobile-menu{display:none}
    @media(max-width:950px){.app{grid-template-columns:1fr}aside{display:none}main{padding:22px 18px}.stats{grid-template-columns:repeat(3,minmax(0,1fr))}.main-grid,.schedule-layout{grid-template-columns:1fr}.mobile-menu{display:inline-grid;place-items:center}.topbar{align-items:flex-start}.top-actions{flex-wrap:wrap;justify-content:flex-end}}
    @media(max-width:600px){.stats{grid-template-columns:1fr 1fr}.stats .card:last-child{grid-column:span 2}.welcome{padding:22px;align-items:flex-start;flex-direction:column}.welcome:after{right:4%;top:5px}.welcome h3{font-size:22px}.level-wrap{align-items:flex-start}.level-ring{width:100px;height:100px}.level-ring strong{font-size:22px}.settings-grid{grid-template-columns:1fr}.topbar h2{font-size:23px}.connection{padding:9px 10px}.form-row{grid-template-columns:1fr}.card{padding:17px}}
  </style>
</head>
<body>
<div class="app">
  <aside>
    <div class="brand"><div class="logo">🐾</div><div><h1>PetFood</h1><p>ALIMENTAÇÃO INTELIGENTE</p></div></div>
    <div class="nav-label">Menu principal</div>
    <nav>
      <button class="nav-btn active" data-page="inicio"><span class="ico">⌂</span> Visão geral</button>
      <button class="nav-btn" data-page="agenda"><span class="ico">◷</span> Agendamentos</button>
      <button class="nav-btn" data-page="historico"><span class="ico">▤</span> Histórico</button>
      <button class="nav-btn" data-page="maquina"><span class="ico">⚙</span> Máquina e conexão</button>
    </nav>
    <div class="side-bottom"><strong>🐶 Cuidado todos os dias</strong><small>Seu companheiro bem alimentado, mesmo quando sua rotina muda.</small></div>
  </aside>
  <main>
    <header class="topbar">
      <div style="display:flex;gap:12px;align-items:center"><button class="icon-button mobile-menu" id="mobileMenu" aria-label="Abrir menu">☰</button><div><div class="eyebrow">PAINEL DE CONTROLE / PETFOOD</div><h2 id="pageTitle">Visão geral</h2></div></div>
      <div class="top-actions"><div class="connection"><span class="dot" id="connectionDot"></span><span id="connectionText">Conectando à máquina...</span></div><button class="icon-button" id="refreshBtn" title="Atualizar dados">↻</button></div>
    </header>

    <section class="page active" id="page-inicio">
      <div class="welcome"><div><h3>Olá! 🐕 Hora de cuidar do seu pet.</h3><p>Acompanhe o nível de ração e controle a alimentação da máquina em um só lugar.</p></div><button class="primary" id="quickFeed">🍽️ Alimentar agora</button></div>
      <div class="grid stats">
        <div class="card"><div class="stat-top"><span>Nível de ração</span><span class="stat-icon">🥣</span></div><div class="stat-value" id="statLevel">--%</div><div class="stat-note" id="statLevelNote">Aguardando leitura do sensor</div></div>
        <div class="card"><div class="stat-top"><span>Próxima alimentação</span><span class="stat-icon">◷</span></div><div class="stat-value" id="statNext">--:--</div><div class="stat-note">Conforme os horários salvos</div></div>
        <div class="card"><div class="stat-top"><span>Estado da máquina</span><span class="stat-icon">⚙️</span></div><div class="stat-value" style="font-size:22px" id="statMachine">Verificando</div><div class="stat-note" id="statMachineNote">Comunicação com ESP32</div></div>
      </div>
      <div class="grid main-grid">
        <div class="card">
          <div class="card-head"><div><h3>Reservatório de ração</h3><p>Leitura em tempo real do sensor ultrassônico</p></div><button class="text-button" id="sensorRefresh">Atualizar</button></div>
          <div class="level-wrap"><div class="level-ring" id="levelRing" style="--level:0%"><div><strong id="levelBig">--%</strong><small>de ração</small></div></div><div class="level-details"><strong id="levelTitle">Esperando o sensor</strong><p id="levelDescription">Verifique se o sensor está conectado e se o ESP32 está ligado.</p><span class="status-pill warn" id="sensorStatus">Sem leitura</span><p class="hint" id="distanceText">Distância: -- cm</p></div></div>
        </div>
        <div class="card">
          <div class="card-head"><div><h3>Alimentação manual</h3><p>Acione o mecanismo quando precisar</p></div><span class="stat-icon">🍽️</span></div>
          <div class="feed-actions"><button class="portion selected" data-seconds="1500" data-label="Padrão">Padrão<small>tempo configurado</small></button><button class="portion" data-seconds="2500" data-label="Longa">Longa<small>mais tempo</small></button><button class="portion" data-seconds="800" data-label="Curta">Curta<small>menos tempo</small></button></div>
          <button class="feed-btn" id="feedBtn">🍲 Liberar ração agora</button><p class="hint">Os botões selecionam a duração de abertura do servo, não uma quantidade exata em gramas.</p>
        </div>
        <div class="card">
          <div class="card-head"><div><h3>Próximos horários</h3><button class="text-button" data-go="agenda">Ver agenda →</button></div></div>
          <div class="meal-list" id="upcomingMeals"><div class="history-empty">Carregando horários...</div></div>
        </div>
        <div class="card">
          <div class="card-head"><div><h3>Resumo da máquina</h3></div><button class="text-button" data-go="maquina">Detalhes →</button></div>
          <div class="status-row"><strong>ESP32</strong><span id="machineConnection">Verificando...</span></div>
          <div class="status-row"><strong>Servo motor</strong><span id="servoState">Pronto para acionar</span></div>
          <div class="status-row"><strong>Sensor de ração</strong><span id="sensorStateShort">Aguardando leitura</span></div>
          <div class="status-row"><strong>Relógio automático</strong><span id="clockState">Não verificado</span></div>
        </div>
      </div>
    </section>

    <section class="page" id="page-agenda">
      <h3 class="section-title">Agendamento de alimentação</h3><p class="section-subtitle">Configure os horários que o ESP32 deve usar para liberar a ração automaticamente.</p>
      <div class="grid schedule-layout">
        <div class="card"><div class="card-head"><div><h3>Horários programados</h3><p>Até 8 horários por dia</p></div><span class="status-pill" id="scheduleCount">0 horários</span></div><div id="scheduleList"><div class="history-empty">Carregando agenda...</div></div></div>
        <div class="card"><div class="card-head"><div><h3>Adicionar horário</h3></div></div>
          <form id="scheduleForm">
            <div class="field"><label for="mealTime">Horário</label><input id="mealTime" type="time" value="08:00" required></div>
            <div class="field"><label for="mealName">Descrição (opcional)</label><input id="mealName" type="text" maxlength="30" placeholder="Ex.: Café da manhã"></div>
            <div class="field"><label for="duration">Tempo de abertura do servo</label><select id="duration"><option value="800">Curto — 0,8 segundo</option><option value="1500" selected>Padrão — 1,5 segundo</option><option value="2500">Longo — 2,5 segundos</option><option value="3000">Extra longo — 3 segundos</option></select></div>
            <div class="form-actions"><button class="btn green" type="submit">＋ Salvar horário</button><button class="btn light" type="button" id="reloadSchedule">Recarregar</button></div>
          </form><p class="hint">A alimentação programada depende do relógio do ESP32 estar sincronizado e do código do back-end executar a agenda.</p>
        </div>
      </div>
    </section>

    <section class="page" id="page-historico">
      <h3 class="section-title">Histórico de atividades</h3><p class="section-subtitle">Eventos registrados nesta sessão do painel. O back-end atual não fornece uma rota de histórico persistente.</p>
      <div class="card"><div class="card-head"><div><h3>Atividades recentes</h3><p>Eventos desta página desde que foi aberta</p></div><button class="text-button" id="clearHistory">Limpar histórico local</button></div><div id="historyList"><div class="history-empty">Nenhuma atividade registrada nesta sessão.</div></div></div>
    </section>

    <section class="page" id="page-maquina">
      <h3 class="section-title">Máquina e conexão</h3><p class="section-subtitle">Estado do PetFood e configuração opcional da rede de internet.</p>
      <div class="grid settings-grid">
        <div class="card"><div class="card-head"><div><h3>Conexão com o PetFood</h3><p>Dados recebidos do ESP32</p></div><span class="stat-icon">📡</span></div>
          <div class="status-row"><strong>Endereço do painel</strong><span id="panelAddress">--</span></div><div class="status-row"><strong>Servidor HTTP</strong><span id="serverStatus">Aguardando</span></div><div class="status-row"><strong>Sensor</strong><span id="sensorStatusMachine">Aguardando</span></div><div class="status-row"><strong>Última leitura</strong><span id="lastReading">--</span></div><div class="status-row"><strong>Relógio sincronizado</strong><span id="ntpStatus">Verificando...</span></div><button class="btn light" style="margin-top:15px" id="testConnection">Testar conexão</button>
        </div>
        <div class="card"><div class="card-head"><div><h3>Wi-Fi com internet</h3><p>Opcional: cadastrar a rede do roteador</p></div><span class="stat-icon">🌐</span></div>
          <form id="wifiForm"><div class="field"><label for="wifiSsid">Nome da rede Wi-Fi (SSID)</label><input id="wifiSsid" maxlength="63" placeholder="Nome da sua rede" autocomplete="off" required></div><div class="field"><label for="wifiPassword">Senha do Wi-Fi</label><input id="wifiPassword" type="password" maxlength="63" placeholder="Senha da rede" autocomplete="new-password"></div><div class="form-actions"><button class="btn green" type="submit">Salvar Wi-Fi</button><button class="btn light" type="button" id="syncClock">Sincronizar relógio</button></div></form>
          <p class="hint">Esta tela envia os dados para <code>/api/wifi</code> e solicita sincronização em <code>/api/ntp</code>. O back-end precisa implementar a conexão STA após salvar a rede. A senha não fica exibida depois do envio.</p>
        </div>
      </div>
    </section>
    <footer style="padding:26px 2px 8px;color:var(--muted);font-size:11px;text-align:center">PetFood · Sistema microcontrolado de alimentação automática · ESP32</footer>
  </main>
</div>
<div class="toast" id="toast" role="status" aria-live="polite"></div>
<script>
(() => {
  "use strict";
  const API = ""; // Página servida pelo ESP32: usa o mesmo endereço automaticamente.
  const $ = (id) => document.getElementById(id);
  let selectedDuration = 1500;
  let schedules = [];
  let historyItems = [];
  let lastLevel = null;
  let lastReadingAt = null;
  let connected = false;
  let toastTimer = null;

  function toast(message) {
    const el = $("toast"); el.textContent = message; el.classList.add("show");
    clearTimeout(toastTimer); toastTimer = setTimeout(() => el.classList.remove("show"), 3800);
  }
  function logActivity(title, detail) {
    historyItems.unshift({title, detail, at: new Date()});
    historyItems = historyItems.slice(0, 30); renderHistory();
  }
  function setConnection(ok, label) {
    connected = ok;
    $("connectionDot").classList.toggle("online", ok);
    $("connectionText").textContent = label || (ok ? "Máquina conectada" : "Máquina desconectada");
    $("statMachine").textContent = ok ? "Conectada" : "Sem conexão";
    $("statMachineNote").textContent = ok ? "Comunicação com ESP32 ativa" : "Verifique o Wi-Fi PetFood";
    $("machineConnection").textContent = ok ? "Conectado" : "Desconectado";
    $("serverStatus").textContent = ok ? "Respondendo" : "Sem resposta";
  }
  async function api(path, options = {}) {
    const response = await fetch(API + path, {
      method: options.method || "GET",
      headers: {"Content-Type":"application/json", ...(options.headers || {})},
      body: options.body === undefined ? undefined : JSON.stringify(options.body),
      cache: "no-store"
    });
    if (!response.ok) throw new Error("HTTP " + response.status);
    const text = await response.text();
    if (!text) return {};
    try { return JSON.parse(text); } catch { return {raw:text}; }
  }
  function formatTime(time) {
    if (typeof time === "string") return time.slice(0,5);
    if (typeof time === "number") return String(Math.floor(time/60)).padStart(2,"0")+":"+String(time%60).padStart(2,"0");
    return "--:--";
  }
  function extractSchedules(data) {
    // Compatível com respostas comuns: {"horarios":["08:00","18:00"]},
    // {"agenda":[...]}, ou lista direta. Campos adicionais são preservados.
    let arr = Array.isArray(data) ? data : (data.horarios || data.agenda || data.schedule || data.times || []);
    if (!Array.isArray(arr)) arr = [];
    return arr.map((item, index) => {
      if (typeof item === "string" || typeof item === "number") return {time:formatTime(item), name:"Refeição "+(index+1), raw:item};
      const time = item.horario ?? item.hora ?? item.time ?? item.h ?? "";
      return {...item, time:formatTime(time), name:item.nome || item.name || ("Refeição "+(index+1)), raw:item};
    }).filter(x => /^\d{2}:\d{2}$/.test(x.time));
  }
  async function loadSchedules() {
    try {
      const data = await api("/api/agenda");
      schedules = extractSchedules(data);
      renderSchedules(); renderUpcoming(); updateNextMeal();
      if (data && !Array.isArray(data) && data.duracao !== undefined) $("duration").value = String(data.duracao);
    } catch (e) {
      $("scheduleList").innerHTML = '<div class="history-empty">Não foi possível carregar a agenda do ESP32. Confira a conexão.</div>';
      $("upcomingMeals").innerHTML = '<div class="history-empty">Agenda indisponível.</div>';
    }
  }
  function renderSchedules() {
    $("scheduleCount").textContent = schedules.length + (schedules.length === 1 ? " horário" : " horários");
    if (!schedules.length) {
      $("scheduleList").innerHTML = '<div class="history-empty">Nenhum horário salvo. Use o formulário ao lado para adicionar.</div>'; return;
    }
    $("scheduleList").innerHTML = schedules.map((s,i) => `
      <div class="schedule-item"><div class="meal-icon">🍽️</div><div class="schedule-time">${escapeHtml(s.time)}</div><div class="schedule-desc"><strong>${escapeHtml(s.name)}</strong><small>Alimentação automática</small></div><button class="remove" data-remove="${i}" aria-label="Remover horário">×</button></div>
    `).join("");
    $("scheduleList").querySelectorAll("[data-remove]").forEach(btn => btn.addEventListener("click", () => removeSchedule(Number(btn.dataset.remove))));
  }
  function renderUpcoming() {
    const sorted = [...schedules].sort((a,b)=>a.time.localeCompare(b.time)).slice(0,3);
    $("upcomingMeals").innerHTML = sorted.length ? sorted.map(s=>`<div class="meal"><div class="meal-icon">🥣</div><div class="meal-info"><strong>${escapeHtml(s.name)}</strong><small>Alimentação programada</small></div><div class="meal-time">${escapeHtml(s.time)}</div></div>`).join("") : '<div class="history-empty">Adicione horários na aba Agendamentos.</div>';
  }
  function updateNextMeal() {
    if (!schedules.length) { $("statNext").textContent = "--:--"; return; }
    const now = new Date(); const current = String(now.getHours()).padStart(2,"0")+":"+String(now.getMinutes()).padStart(2,"0");
    const future = [...schedules].map(s=>s.time).sort().find(t=>t>current);
    $("statNext").textContent = future || [...schedules].map(s=>s.time).sort()[0];
  }
  async function saveSchedules() {
    // O back-end do projeto recebe: {"horarios":["08:00","18:00"],"duracao":1500}
    const payload = {horarios:schedules.map(s=>s.time), duracao:Number($("duration").value || selectedDuration)};
    try {
      await api("/api/agenda", {method:"POST", body:payload});
      toast("Agenda enviada e salva no ESP32."); logActivity("Agenda atualizada", schedules.length+" horário(s) programado(s).");
      await loadSchedules();
    } catch (e) {
      toast("Não foi possível salvar no ESP32. Confira o formato esperado pela rota /api/agenda.");
    }
  }
  async function removeSchedule(index) {
    schedules.splice(index,1); renderSchedules(); renderUpcoming(); updateNextMeal(); await saveSchedules();
  }
  async function refreshLevel() {
    try {
      const data = await api("/api/nivel");
      setConnection(true, "Máquina conectada");
      const valid = data.valido === true || data.valid === true || data.ok === true;
      const level = Number(data.nivel ?? data.level ?? data.percentual);
      const distance = Number(data.distancia ?? data.distance);
      if (!valid || !Number.isFinite(level)) {
        $("sensorStatus").textContent = "Sem leitura válida"; $("sensorStatus").className = "status-pill warn";
        $("levelTitle").textContent = "Sensor sem leitura"; $("levelDescription").textContent = "O ESP32 respondeu, mas não enviou um nível válido. Confira TRIG, ECHO e alimentação do sensor.";
        $("sensorStateShort").textContent = "Sem leitura válida"; $("sensorStatusMachine").textContent = "Sem leitura válida";
        $("statLevel").textContent = "--%"; $("levelBig").textContent = "--%"; $("statLevelNote").textContent = "Verifique o sensor";
        if (Number.isFinite(distance)) $("distanceText").textContent = "Distância: "+distance.toFixed(1)+" cm";
        return;
      }
      lastLevel = Math.max(0,Math.min(100,level)); lastReadingAt = new Date();
      $("levelRing").style.setProperty("--level",lastLevel+"%");
      $("levelBig").textContent = Math.round(lastLevel)+"%"; $("statLevel").textContent = Math.round(lastLevel)+"%";
      $("statLevelNote").textContent = lastLevel <= 20 ? "Ração baixa" : "Leitura recebida do sensor";
      $("levelTitle").textContent = lastLevel <= 20 ? "Ração quase acabando" : lastLevel < 50 ? "Nível intermediário" : "Reservatório com ração";
      $("levelDescription").textContent = lastLevel <= 20 ? "Reabasteça o reservatório para evitar que seu pet fique sem ração." : "O nível está sendo atualizado automaticamente pelo sensor ultrassônico.";
      $("sensorStatus").textContent = lastLevel <= 20 ? "Nível baixo" : "Sensor funcionando"; $("sensorStatus").className = "status-pill"+(lastLevel<=20?" warn":"");
      $("sensorStateShort").textContent = "Funcionando"; $("sensorStatusMachine").textContent = "Funcionando";
      $("distanceText").textContent = Number.isFinite(distance) ? "Distância medida: "+distance.toFixed(1)+" cm" : "Distância: disponível no ESP32";
      $("lastReading").textContent = lastReadingAt.toLocaleTimeString("pt-BR",{hour:"2-digit",minute:"2-digit",second:"2-digit"});
    } catch (e) {
      setConnection(false, "Sem conexão com a máquina");
      $("sensorStatus").textContent = "Sem conexão"; $("sensorStatus").className = "status-pill bad";
      $("levelTitle").textContent = "Máquina não encontrada"; $("levelDescription").textContent = "Conecte o celular ou computador à rede Wi-Fi PetFood e atualize a página.";
      $("sensorStateShort").textContent = "Sem conexão"; $("sensorStatusMachine").textContent = "Sem conexão";
    }
  }
  async function feedNow() {
    if (!connected) { toast("Conecte-se à rede PetFood antes de acionar o servo."); return; }
    const btn = $("feedBtn"); btn.disabled = true; btn.textContent = "⏳ Acionando servo...";
    try {
      // A rota existente pode ignorar a duração enviada e usar o valor salvo na agenda.
      await api("/api/alimentar", {method:"POST", body:{duracao:Number(selectedDuration), tempo:Number(selectedDuration)}});
      $("servoState").textContent = "Acionado agora";
      toast("Comando de alimentação enviado ao ESP32."); logActivity("Alimentação manual", "Comando enviado; duração selecionada: "+selectedDuration+" ms.");
    } catch (e) { toast("Falha ao acionar o servo. Verifique a rota /api/alimentar."); }
    finally { btn.disabled = false; btn.textContent = "🍲 Liberar ração agora"; }
  }
  function renderHistory() {
    const el = $("historyList");
    if (!historyItems.length) { el.innerHTML = '<div class="history-empty">Nenhuma atividade registrada nesta sessão.</div>'; return; }
    el.innerHTML = historyItems.map(h=>`<div class="history-item"><div><strong>${escapeHtml(h.title)}</strong><small>${escapeHtml(h.detail)}</small></div><span>${h.at.toLocaleTimeString("pt-BR",{hour:"2-digit",minute:"2-digit"})}</span></div>`).join("");
  }
  function escapeHtml(value) {
    return String(value ?? "").replace(/[&<>"']/g, ch => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[ch]));
  }
  function openPage(name) {
    document.querySelectorAll(".page").forEach(p=>p.classList.toggle("active",p.id==="page-"+name));
    document.querySelectorAll(".nav-btn").forEach(b=>b.classList.toggle("active",b.dataset.page===name));
    const titles={inicio:"Visão geral",agenda:"Agendamentos",historico:"Histórico",maquina:"Máquina e conexão"};
    $("pageTitle").textContent=titles[name]||"PetFood";
    if(name==="agenda") loadSchedules();
    if(name==="historico") renderHistory();
    if(name==="maquina") $("panelAddress").textContent=location.host || "Arquivo local";
  }
  document.querySelectorAll(".nav-btn").forEach(b=>b.addEventListener("click",()=>openPage(b.dataset.page)));
  document.querySelectorAll("[data-go]").forEach(b=>b.addEventListener("click",()=>openPage(b.dataset.go)));
  $("mobileMenu").addEventListener("click",()=>{const nav=document.querySelector("aside");nav.style.display=nav.style.display==="flex"?"none":"flex";nav.style.position="fixed";nav.style.zIndex="20";nav.style.inset="0 auto 0 0";nav.style.width="260px";});
  document.querySelectorAll(".portion").forEach(b=>b.addEventListener("click",()=>{
    document.querySelectorAll(".portion").forEach(x=>x.classList.remove("selected")); b.classList.add("selected");
    selectedDuration=Number(b.dataset.seconds); $("duration").value=String(selectedDuration);
  }));
  $("feedBtn").addEventListener("click",feedNow); $("quickFeed").addEventListener("click",feedNow);
  $("sensorRefresh").addEventListener("click",refreshLevel); $("refreshBtn").addEventListener("click",async()=>{await refreshLevel();await loadSchedules();toast("Dados atualizados.");});
  $("reloadSchedule").addEventListener("click",loadSchedules);
  $("scheduleForm").addEventListener("submit",async ev=>{
    ev.preventDefault();
    if(schedules.length>=8){toast("O limite de 8 horários foi atingido.");return;}
    const time=$("mealTime").value; if(!time){toast("Escolha um horário.");return;}
    if(schedules.some(s=>s.time===time)){toast("Esse horário já está cadastrado.");return;}
    schedules.push({time,name:$("mealName").value.trim()||"Refeição"});
    schedules.sort((a,b)=>a.time.localeCompare(b.time)); selectedDuration=Number($("duration").value||1500);
    renderSchedules();renderUpcoming();updateNextMeal();await saveSchedules();$("mealName").value="";
  });
  $("clearHistory").addEventListener("click",()=>{historyItems=[];renderHistory();toast("Histórico local limpo.");});
  $("testConnection").addEventListener("click",async()=>{await refreshLevel();if(connected){toast("O ESP32 respondeu corretamente.");logActivity("Teste de conexão","ESP32 respondeu à rota /api/nivel.");}else toast("Não foi possível acessar o ESP32.");});
  $("wifiForm").addEventListener("submit",async ev=>{
    ev.preventDefault(); const ssid=$("wifiSsid").value.trim(), password=$("wifiPassword").value;
    if(!ssid){toast("Informe o nome da rede Wi-Fi.");return;}
    try{
      await api("/api/wifi",{method:"POST",body:{ssid,senha:password,password}});
      $("wifiPassword").value=""; toast("Credenciais enviadas. Se o ESP32 não conectar, o back-end precisa iniciar a conexão Wi-Fi STA após salvá-las.");
      logActivity("Configuração Wi-Fi","Credenciais da rede enviadas ao ESP32.");
    }catch(e){toast("Falha ao enviar Wi-Fi. Confira se a rota /api/wifi aceita os campos ssid e senha.");}
  });
  $("syncClock").addEventListener("click",async()=>{
    try{await api("/api/ntp",{method:"POST",body:{}});toast("Solicitação de sincronização enviada.");logActivity("Sincronização do relógio","Solicitação enviada ao ESP32.");}
    catch(e){toast("Não foi possível solicitar sincronização. O ESP32 precisa estar conectado à internet.");}
  });
  async function checkClock(){
    try{
      const data=await api("/api/estado");
      const synced=Boolean(data.ntpSincronizado ?? data.sincronizado ?? data.ntp ?? data.timeValid);
      $("clockState").textContent=synced?"Sincronizado":"Não sincronizado";
      $("ntpStatus").textContent=synced?"Sincronizado":"Não sincronizado";
    }catch(e){$("clockState").textContent="Indisponível";$("ntpStatus").textContent="Indisponível";}
  }
  async function init(){
    $("panelAddress").textContent=location.host || "Arquivo local";
    await Promise.all([refreshLevel(),loadSchedules(),checkClock()]);
    setInterval(refreshLevel,2000); setInterval(checkClock,15000); setInterval(updateNextMeal,30000);
  }
  init();
})();
</script>
</body>
</html>
)rawliteral";

// ===== FUNCOES AUXILIARES =====
String jsonEscape(const String& valor) {
  String saida;
  saida.reserve(valor.length() + 8);
  for (size_t i = 0; i < valor.length(); i++) {
    char c = valor[i];
    if (c == '"' || c == '\\') { saida += '\\'; saida += c; }
    else if (c == '\n') saida += "\\n";
    else if (c == '\r') saida += "\\r";
    else if ((uint8_t)c >= 32) saida += c;
  }
  return saida;
}

String jsonStringField(const String& body, const char* key) {
  String needle = String("\"") + key + "\"";
  int p = body.indexOf(needle);
  if (p < 0) return "";
  p = body.indexOf(':', p + needle.length());
  if (p < 0) return "";
  p++;
  while (p < (int)body.length() && (body[p] == ' ' || body[p] == '\t')) p++;
  if (p >= (int)body.length() || body[p] != '"') return "";
  p++;
  String out;
  bool escaped = false;
  for (; p < (int)body.length(); p++) {
    char c = body[p];
    if (escaped) {
      if (c == 'n') out += '\n';
      else if (c == 'r') out += '\r';
      else out += c;
      escaped = false;
    } else if (c == '\\') escaped = true;
    else if (c == '"') break;
    else out += c;
  }
  return out;
}

long jsonNumberField(const String& body, const char* key, long fallback) {
  String needle = String("\"") + key + "\"";
  int p = body.indexOf(needle);
  if (p < 0) return fallback;
  p = body.indexOf(':', p + needle.length());
  if (p < 0) return fallback;
  p++;
  while (p < (int)body.length() && (body[p] == ' ' || body[p] == '\t')) p++;
  int end = p;
  if (end < (int)body.length() && body[end] == '-') end++;
  while (end < (int)body.length() && isDigit(body[end])) end++;
  if (end == p) return fallback;
  return body.substring(p, end).toInt();
}

bool relogioSincronizado() {
  struct tm info;
  return getLocalTime(&info, 50);
}

String horaAtualHHMM() {
  struct tm info;
  if (!getLocalTime(&info, 50)) return "";
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", info.tm_hour, info.tm_min);
  return String(buffer);
}

int diaAtual() {
  struct tm info;
  if (!getLocalTime(&info, 50)) return -1;
  return info.tm_yday;
}

float lerDistanciaCm() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);

  unsigned long duracao = pulseIn(PINO_ECHO, HIGH, TIMEOUT_ECHO_US);
  if (duracao == 0) return -1.0f;
  return (float)duracao * 0.0343f / 2.0f;
}

void atualizarSensor() {
  if (millis() - ultimaLeituraSensor < 1000) return;
  ultimaLeituraSensor = millis();

  float distancia = lerDistanciaCm();
  if (distancia < 0 || distancia > 400) {
    ultimaLeituraValida = false;
    ultimaDistanciaCm = -1.0f;
    ultimoNivelPercentual = -1;
    return;
  }

  ultimaDistanciaCm = distancia;
  float nivel = (ALTURA_RESERVATORIO_CM - distancia) / ALTURA_RESERVATORIO_CM * 100.0f;
  if (nivel < 0) nivel = 0;
  if (nivel > 100) nivel = 100;
  ultimoNivelPercentual = (int)(nivel + 0.5f);
  ultimaLeituraValida = true;
}

bool abrirServo(unsigned long tempoMs) {
  if (tempoMs < 200) tempoMs = 200;
  if (tempoMs > 10000) tempoMs = 10000;
  servoRacao.write(ANGULO_ABERTO);
  servoAberto = true;
  fecharServoEm = millis() + tempoMs;
  return true;
}

void atualizarServo() {
  if (servoAberto && (long)(millis() - fecharServoEm) >= 0) {
    servoRacao.write(ANGULO_FECHADO);
    servoAberto = false;
  }
}

void carregarAgenda() {
  preferences.begin("petfood", true);
  quantidadeHorarios = preferences.getInt("qtd", 0);
  if (quantidadeHorarios < 0 || quantidadeHorarios > MAX_HORARIOS) quantidadeHorarios = 0;
  duracaoServoMs = preferences.getULong("duracao", 1500);
  for (int i = 0; i < MAX_HORARIOS; i++) {
    horarios[i] = preferences.getString((String("h") + i).c_str(), "");
  }
  wifiSSIDSalvo = preferences.getString("ssid", "");
  wifiSenhaSalva = preferences.getString("senha", "");
  preferences.end();

  // Reconstrói quantidade a partir dos horários realmente salvos.
  quantidadeHorarios = 0;
  for (int i = 0; i < MAX_HORARIOS; i++) {
    if (horarios[i].length() == 5) quantidadeHorarios++;
  }
}

void salvarAgenda() {
  preferences.begin("petfood", false);
  preferences.putInt("qtd", quantidadeHorarios);
  preferences.putULong("duracao", duracaoServoMs);
  for (int i = 0; i < MAX_HORARIOS; i++) {
    preferences.putString((String("h") + i).c_str(), horarios[i]);
  }
  preferences.end();
}

void conectarWiFiInternet() {
  if (wifiSSIDSalvo.isEmpty()) return;
  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == wifiSSIDSalvo) return;
  if (ultimoSSIDTentado == wifiSSIDSalvo && WiFi.status() != WL_CONNECTED) return;
  ultimoSSIDTentado = wifiSSIDSalvo;
  WiFi.begin(wifiSSIDSalvo.c_str(), wifiSenhaSalva.c_str());
}

void responderJSON(int codigo, const String& json) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(codigo, "application/json; charset=utf-8", json);
}

void rotaRaiz() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", pagina);
}

void rotaNivel() {
  atualizarSensor();
  String json = "{\"valido\":";
  json += (ultimaLeituraValida ? "true" : "false");
  json += ",\"distancia\":";
  json += (ultimaLeituraValida ? String(ultimaDistanciaCm, 1) : "null");
  json += ",\"nivel\":";
  json += (ultimaLeituraValida ? String(ultimoNivelPercentual) : "null");
  json += "}";
  responderJSON(200, json);
}

void rotaAlimentar() {
  String body = server.arg("plain");
  long tempo = jsonNumberField(body, "duracao", -1);
  if (tempo < 0) tempo = jsonNumberField(body, "tempo", -1);
  if (tempo < 0) tempo = duracaoServoMs;
  abrirServo((unsigned long)tempo);
  responderJSON(200, "{\"ok\":true,\"mensagem\":\"Comando enviado ao servo\"}");
}

void rotaAgendaGet() {
  String json = "{\"horarios\":[";
  bool primeiro = true;
  for (int i = 0; i < MAX_HORARIOS; i++) {
    if (horarios[i].length() != 5) continue;
    if (!primeiro) json += ",";
    json += "\"" + jsonEscape(horarios[i]) + "\"";
    primeiro = false;
  }
  json += "],\"duracao\":";
  json += String(duracaoServoMs);
  json += "}";
  responderJSON(200, json);
}

void rotaAgendaPost() {
  String body = server.arg("plain");
  int pos = body.indexOf("\"horarios\"");
  if (pos < 0) pos = body.indexOf("\"agenda\"");
  if (pos < 0) pos = body.indexOf("\"times\"");
  if (pos < 0) {
    responderJSON(400, "{\"ok\":false,\"erro\":\"Campo horarios ausente\"}");
    return;
  }
  int inicio = body.indexOf('[', pos);
  int fim = body.indexOf(']', inicio);
  if (inicio < 0 || fim < 0) {
    responderJSON(400, "{\"ok\":false,\"erro\":\"Lista de horarios invalida\"}");
    return;
  }

  for (int i = 0; i < MAX_HORARIOS; i++) horarios[i] = "";
  quantidadeHorarios = 0;
  int cursor = inicio + 1;
  while (cursor < fim && quantidadeHorarios < MAX_HORARIOS) {
    int aspas1 = body.indexOf('"', cursor);
    if (aspas1 < 0 || aspas1 >= fim) break;
    int aspas2 = body.indexOf('"', aspas1 + 1);
    if (aspas2 < 0 || aspas2 > fim) break;
    String hora = body.substring(aspas1 + 1, aspas2);
    if (hora.length() == 5 && hora[2] == ':' &&
        isDigit(hora[0]) && isDigit(hora[1]) &&
        isDigit(hora[3]) && isDigit(hora[4])) {
      int hh = hora.substring(0, 2).toInt();
      int mm = hora.substring(3, 5).toInt();
      if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59) {
        bool duplicado = false;
        for (int i = 0; i < quantidadeHorarios; i++) if (horarios[i] == hora) duplicado = true;
        if (!duplicado) horarios[quantidadeHorarios++] = hora;
      }
    }
    cursor = aspas2 + 1;
  }

  long dur = jsonNumberField(body, "duracao", duracaoServoMs);
  if (dur < 200) dur = 200;
  if (dur > 10000) dur = 10000;
  duracaoServoMs = (unsigned long)dur;
  salvarAgenda();
  responderJSON(200, "{\"ok\":true,\"quantidade\":" + String(quantidadeHorarios) + "}");
}

void rotaWifiPost() {
  String body = server.arg("plain");
  String ssid = jsonStringField(body, "ssid");
  String senha = jsonStringField(body, "senha");
  if (senha.isEmpty()) senha = jsonStringField(body, "password");
  if (ssid.isEmpty()) {
    responderJSON(400, "{\"ok\":false,\"erro\":\"SSID ausente\"}");
    return;
  }
  wifiSSIDSalvo = ssid;
  wifiSenhaSalva = senha;
  preferences.begin("petfood", false);
  preferences.putString("ssid", wifiSSIDSalvo);
  preferences.putString("senha", wifiSenhaSalva);
  preferences.end();
  ultimoSSIDTentado = "";
  conectarWiFiInternet();
  responderJSON(200, "{\"ok\":true,\"mensagem\":\"Dados Wi-Fi salvos; conexao iniciada\"}");
}

void rotaNtpPost() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.nist.gov", "time.google.com");
    responderJSON(200, "{\"ok\":true,\"mensagem\":\"Sincronizacao solicitada\"}");
  } else {
    responderJSON(503, "{\"ok\":false,\"mensagem\":\"ESP32 ainda nao conectado ao Wi-Fi com internet\"}");
  }
}

void rotaEstado() {
  bool sincronizado = relogioSincronizado();
  String json = "{\"ntpSincronizado\":";
  json += (sincronizado ? "true" : "false");
  json += ",\"wifiConectado\":";
  json += (WiFi.status() == WL_CONNECTED ? "true" : "false");
  json += ",\"ip\":\"";
  json += (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString());
  json += "\"}";
  responderJSON(200, json);
}

void verificarAgendaAutomatica() {
  struct tm info;
  if (!getLocalTime(&info, 20)) return;
  int minutoAtual = info.tm_hour * 60 + info.tm_min;
  int dia = info.tm_yday;
  if (minutoAtual == ultimoMinutoExecutado && dia == ultimoDiaExecutado) return;

  char agora[6];
  snprintf(agora, sizeof(agora), "%02d:%02d", info.tm_hour, info.tm_min);
  for (int i = 0; i < MAX_HORARIOS; i++) {
    if (horarios[i] == String(agora)) {
      // Segurança: não liberar ração automaticamente se o sensor indicar reservatório baixo.
      atualizarSensor();
      if (ultimaLeituraValida && ultimoNivelPercentual > 20) {
        abrirServo(duracaoServoMs);
      }
      ultimoMinutoExecutado = minutoAtual;
      ultimoDiaExecutado = dia;
      break;
    }
  }
}

void configurarRotas() {
  server.on("/", HTTP_GET, rotaRaiz);
  server.on("/api/nivel", HTTP_GET, rotaNivel);
  server.on("/api/alimentar", HTTP_POST, rotaAlimentar);
  server.on("/api/agenda", HTTP_GET, rotaAgendaGet);
  server.on("/api/agenda", HTTP_POST, rotaAgendaPost);
  server.on("/api/wifi", HTTP_POST, rotaWifiPost);
  server.on("/api/ntp", HTTP_POST, rotaNtpPost);
  server.on("/api/estado", HTTP_GET, rotaEstado);
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204);
    } else {
      server.send(404, "text/plain; charset=utf-8", "Rota nao encontrada");
    }
  });
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  digitalWrite(PINO_TRIG, LOW);

  servoRacao.setPeriodHertz(50);
  servoRacao.attach(PINO_SERVO, 500, 2400);
  servoRacao.write(ANGULO_FECHADO);

  carregarAgenda();

  WiFi.mode(WIFI_AP_STA);
  bool apOk = WiFi.softAP(WIFI_AP_NOME, WIFI_AP_SENHA);
  Serial.println();
  Serial.println("===== PETFOOD =====");
  Serial.printf("Ponto de acesso: %s\n", apOk ? "iniciado" : "falhou");
  Serial.printf("Rede: %s\n", WIFI_AP_NOME);
  Serial.printf("Senha: %s\n", WIFI_AP_SENHA);
  Serial.print("Painel: http://");
  Serial.println(WiFi.softAPIP());

  configurarRotas();
  server.begin();
  Serial.println("Servidor HTTP iniciado.");

  if (wifiSSIDSalvo.length()) conectarWiFiInternet();
  if (WiFi.status() == WL_CONNECTED) {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.nist.gov", "time.google.com");
  }
}

void loop() {
  server.handleClient();
  atualizarServo();
  atualizarSensor();

  if (wifiSSIDSalvo.length() && WiFi.status() != WL_CONNECTED) {
    static unsigned long ultimaTentativa = 0;
    if (millis() - ultimaTentativa > 30000UL) {
      ultimaTentativa = millis();
      ultimoSSIDTentado = "";
      conectarWiFiInternet();
    }
  }
  verificarAgendaAutomatica();
  delay(2);
}
