#include <ESP8266WiFi.h>

// WiFi credentials
const char* ssid = "Abdalrahman-Phone";
const char* password = "123456789";

// Motor pins
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
#define ENA D0
#define ENB D5
#define BUZZER D6

WiFiServer server(80);

String currentCommand = "Stopped";
int speedLeft = 1023;   // Max PWM for ENA (left motor)
int speedRight = 1023;  // Max PWM for ENB (right motor)

// --- Nokia Tune at Startup ---
void nokiaTune() {
  int melody[] = { 659, 698, 523, 587, 784, 740, 587, 659, 523 };
  int noteDurations[] = { 150, 150, 150, 150, 300, 150, 150, 150, 300 }; // ms
  for (int i = 0; i < 9; i++) {
    tone(BUZZER, melody[i]);
    delay(noteDurations[i]);
    noTone(BUZZER);
    delay(30);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Robot Web Server IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  stopMotors();
  nokiaTune(); // Play Nokia tune at startup
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
  currentCommand = "Stopped";
}

void moveForward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speedLeft * 0.7);
  analogWrite(ENB, speedRight * 0.7);
  currentCommand = "Moving Forward";
  delayMicroseconds(200);
  stopMotors();
}

void moveBackward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedLeft * 0.7);
  analogWrite(ENB, speedRight * 0.7);
  currentCommand = "Moving Backward";
  delayMicroseconds(200);
  stopMotors();
}

void turnLeft() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);
  currentCommand = "Turning Left";
  delayMicroseconds(200);
  stopMotors();
}

void turnRight() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);
  currentCommand = "Turning Right";
  delayMicroseconds(200);
  stopMotors();
}

void buzz() {
  digitalWrite(BUZZER, HIGH);
  delayMicroseconds(200);
  digitalWrite(BUZZER, LOW);
  currentCommand = "Buzzed";
}

String processor(const String& var){
  if(var == "STATUS"){
    return currentCommand;
  } else if(var == "SPEEDLEFT"){
    return String(map(speedLeft, 0, 1023, 0, 100)); // % value
  } else if(var == "SPEEDRIGHT"){
    return String(map(speedRight, 0, 1023, 0, 100)); // % value
  }
  return String();
}

void handleSpeed(String request) {
  int idxL = request.indexOf("speedL=");
  int idxR = request.indexOf("speedR=");
  if (idxL != -1) {
    int val = request.substring(idxL + 7).toInt();
    speedLeft = map(constrain(val, 0, 100), 0, 100, 0, 1023);
  }
  if (idxR != -1) {
    int val = request.substring(idxR + 7).toInt();
    speedRight = map(constrain(val, 0, 100), 0, 100, 0, 1023);
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/speed?") != -1) {
      handleSpeed(request);
    }

    if (request.indexOf("/F") != -1) moveForward();
    else if (request.indexOf("/B") != -1) moveBackward();
    else if (request.indexOf("/L") != -1) turnLeft();
    else if (request.indexOf("/R") != -1) turnRight();
    else if (request.indexOf("/S") != -1) stopMotors();
    else if (request.indexOf("/BUZ") != -1) buzz();

    if (request.indexOf("GET /status") != -1) {
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print(currentCommand);
      delay(10);
      client.stop();
      return;
    }

    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>NodeMCU WiFi Robot Controller</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Montserrat:wght@600&display=swap');
  :root {
    --green: #2ecc71;
    --red: #e74c3c;
    --blue: #3498db;
    --orange: #f39c12;
    --gray: #7f8c8d;
    --yellow: #f1c40f;
    --dark-bg: #121212;
    --light-bg: #f0f0f0;
    --text-light: #fff;
    --text-dark: #222;
  }
  body {
    margin: 0; padding: 0;
    background: var(--dark-bg);
    font-family: 'Montserrat', sans-serif;
    color: var(--text-light);
    display: flex;
    flex-direction: column;
    min-height: 100vh;
    align-items: center;
    justify-content: flex-start;
    transition: background 0.3s, color 0.3s;
  }
  body.light {
    background: var(--light-bg);
    color: var(--text-dark);
  }
  header {
    background: rgba(0,0,0,0.4);
    width: 100%;
    padding: 20px 0;
    text-align: center;
    font-size: 1.8em;
    font-weight: 700;
    letter-spacing: 2px;
    box-shadow: 0 4px 10px rgba(0,0,0,0.3);
    user-select: none;
    transition: background 0.3s, color 0.3s;
  }
  body.light header {
    background: #ddd;
    color: var(--text-dark);
  }
  main {
    flex: 1;
    width: 100%;
    max-width: 480px;
    padding: 20px;
    box-sizing: border-box;
  }
  .status {
    background: rgba(255,255,255,0.15);
    border-radius: 12px;
    padding: 15px;
    margin-bottom: 25px;
    font-size: 1.3em;
    text-align: center;
    box-shadow: 0 0 15px rgba(255,255,255,0.2);
    user-select: none;
    transition: background 0.3s, color 0.3s;
  }
  body.light .status {
    background: #eee;
    color: var(--text-dark);
    box-shadow: 0 0 15px rgba(0,0,0,0.1);
  }
  .button-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    grid-gap: 20px;
  }
  button {
    position: relative;
    background: var(--green);
    border: none;
    border-radius: 15px;
    box-shadow: 0 6px #27ae60;
    color: white;
    font-size: 1.3em;
    padding: 20px 0;
    cursor: pointer;
    transition: all 0.3s ease;
    user-select: none;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    overflow: hidden;
    outline: none;
  }
  button:active {
    box-shadow: 0 3px #1b5e20;
    transform: translateY(3px);
  }
  button:hover {
    filter: brightness(1.15);
  }
  button:focus::after {
    content: '';
    position: absolute;
    top: -5px; left: -5px; right: -5px; bottom: -5px;
    border: 3px solid #fff;
    border-radius: 18px;
    pointer-events: none;
  }
  button#F { background: var(--green); box-shadow: 0 6px #27ae60; }
  button#B { background: var(--red); box-shadow: 0 6px #c0392b; }
  button#L { background: var(--blue); box-shadow: 0 6px #2980b9; }
  button#R { background: var(--orange); box-shadow: 0 6px #d35400; }
  button#S { background: var(--gray); box-shadow: 0 6px #636e72; }
  button#BUZ { background: var(--yellow); color: #000; box-shadow: 0 6px #b7950b; }
  button span.icon {
    font-size: 2em;
    margin-bottom: 8px;
  }
  footer {
    width: 100%;
    text-align: center;
    padding: 15px 0;
    background: rgba(0,0,0,0.2);
    font-size: 0.9em;
    color: #ddd;
    user-select: none;
    transition: background 0.3s, color 0.3s;
  }
  body.light footer {
    background: #ddd;
    color: var(--text-dark);
  }
  .speed-controls {
    margin-top: 30px;
    background: rgba(255,255,255,0.15);
    border-radius: 15px;
    padding: 20px;
    box-shadow: 0 0 15px rgba(255,255,255,0.2);
    user-select: none;
    transition: background 0.3s, color 0.3s;
  }
  body.light .speed-controls {
    background: #eee;
    color: var(--text-dark);
    box-shadow: 0 0 15px rgba(0,0,0,0.1);
  }
  .speed-controls label {
    display: block;
    font-weight: 600;
    margin-bottom: 8px;
  }
  .speed-controls input[type=range] {
    width: 100%;
    -webkit-appearance: none;
    height: 8px;
    border-radius: 5px;
    background: #444;
    outline: none;
    margin-bottom: 15px;
    transition: background 0.3s;
  }
  body.light .speed-controls input[type=range] {
    background: #ccc;
  }
  .speed-controls input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 24px;
    height: 24px;
    background: var(--green);
    cursor: pointer;
    border-radius: 50%;
    border: 2px solid #fff;
    transition: background 0.3s;
  }
  .speed-controls input[type=range]:active::-webkit-slider-thumb {
    background: #27ae60;
  }
  .speed-controls input[type=range]::-moz-range-thumb {
    width: 24px;
    height: 24px;
    background: var(--green);
    cursor: pointer;
    border-radius: 50%;
    border: 2px solid #fff;
    transition: background 0.3s;
  }
  .speed-controls input[type=range]:active::-moz-range-thumb {
    background: #27ae60;
  }
  .speed-value {
    text-align: right;
    font-weight: 600;
    margin-top: -12px;
    margin-bottom: 12px;
    font-size: 0.9em;
  }
  .toggle-theme {
    margin-top: 20px;
    text-align: center;
  }
  .toggle-theme button {
    background: transparent;
    border: 2px solid var(--text-light);
    color: var(--text-light);
    font-size: 1em;
    padding: 10px 20px;
    border-radius: 25px;
    cursor: pointer;
    transition: all 0.3s ease;
  }
  .toggle-theme button:hover {
    background: var(--text-light);
    color: var(--dark-bg);
  }
  body.light .toggle-theme button {
    border-color: var(--text-dark);
    color: var(--text-dark);
  }
  body.light .toggle-theme button:hover {
    background: var(--text-dark);
    color: var(--text-light);
  }
  @media (max-width: 400px) {
    button {
      font-size: 1.1em;
      padding: 18px 0;
    }
    .speed-controls {
      padding: 15px;
    }
  }
</style>
</head>
<body>
<header>NodeMCU WiFi Robot Controller</header>
<main>
  <div class="status" aria-live="polite" aria-atomic="true">Status: <strong id="statusText">)rawliteral" + currentCommand + R"rawliteral(</strong></div>
  <div class="button-grid" role="group" aria-label="Robot control buttons">
    <button id="F" aria-label="Move Forward" onclick="sendCommand('/F')"><span class="icon">&#8593;</span>Forward</button>
    <button id="L" aria-label="Turn Left" onclick="sendCommand('/L')"><span class="icon">&#8592;</span>Left</button>
    <button id="S" aria-label="Stop" onclick="sendCommand('/S')"><span class="icon">&#9632;</span>Stop</button>
    <button id="R" aria-label="Turn Right" onclick="sendCommand('/R')"><span class="icon">&#8594;</span>Right</button>
    <button id="B" aria-label="Move Backward" onclick="sendCommand('/B')"><span class="icon">&#8595;</span>Backward</button>
    <button id="BUZ" aria-label="Buzz Buzzer" onclick="sendCommand('/BUZ')"><span class="icon">&#128276;</span>Buzzer</button>
  </div>
  <section class="speed-controls" aria-label="Motor speed controls">
    <label for="speedLeft">Left Motor Speed: <span id="speedLeftValue">100</span>%</label>
    <input type="range" id="speedLeft" min="0" max="100" value="100" oninput="updateSpeed('L', this.value)" />
    <label for="speedRight">Right Motor Speed: <span id="speedRightValue">100</span>%</label>
    <input type="range" id="speedRight" min="0" max="100" value="100" oninput="updateSpeed('R', this.value)" />
  </section>
  <div class="toggle-theme">
    <button id="toggleThemeBtn" aria-pressed="false" aria-label="Toggle dark/light mode">Toggle Light/Dark Mode</button>
  </div>
</main>
<footer>© 2025 NodeMCU Robot Project by ENG.Abdalrahman Othman</footer>
<script>
  function sendCommand(cmd) {
    fetch(cmd).then(() => updateStatus());
  }
  function updateStatus() {
    fetch('/status')
      .then(response => response.text())
      .then(text => {
        document.getElementById('statusText').textContent = text;
      })
      .catch(() => {
        document.getElementById('statusText').textContent = 'Disconnected';
      });
  }
  let speedTimeout;
  function updateSpeed(motor, value) {
    if (motor === 'L') {
      document.getElementById('speedLeftValue').textContent = value;
    } else {
      document.getElementById('speedRightValue').textContent = value;
    }
    clearTimeout(speedTimeout);
    speedTimeout = setTimeout(() => {
      const speedL = document.getElementById('speedLeft').value;
      const speedR = document.getElementById('speedRight').value;
      fetch(`/speed?speedL=${speedL}&speedR=${speedR}`);
    }, 300);
  }
  const body = document.body;
  const toggleBtn = document.getElementById('toggleThemeBtn');
  function setTheme(isLight) {
    if (isLight) {
      body.classList.add('light');
      toggleBtn.setAttribute('aria-pressed', 'true');
    } else {
      body.classList.remove('light');
      toggleBtn.setAttribute('aria-pressed', 'false');
    }
  }
  toggleBtn.addEventListener('click', () => {
    const isLight = body.classList.toggle('light');
    localStorage.setItem('themeLight', isLight ? '1' : '0');
    toggleBtn.setAttribute('aria-pressed', isLight ? 'true' : 'false');
  });
  window.onload = () => {
    const saved = localStorage.getItem('themeLight');
    setTheme(saved === '1');
    updateStatus();
  };
  setInterval(updateStatus, 1500);
</script>
</body>
</html>
)rawliteral";

    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    client.print(html);
    delay(10);
    client.stop();
  }
}
