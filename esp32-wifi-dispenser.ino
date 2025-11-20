/*
 * Smart AI-Powered Pill Dispenser - ESP32 WiFi Web Server
 * 
 * Features:
 * - WiFi Web Server hosting HTML control interface
 * - REST API for remote control
 * - Stepper motor control for compartment rotation
 * - Servo motor for pill dispensing
 * - LED indicators and buzzer feedback
 * - Scheduled dispensing with time-based triggers
 * - Real-time status updates
 * 
 * Hardware Setup:
 * - ESP32 Dev Module
 * - Stepper Motor via ULN2003 Driver (IN1-4 to GPIO 18,19,21,22)
 * - Servo Motor (Signal to GPIO 13)
 * - Push Button (GPIO 14)
 * - Buzzer (GPIO 25)
 * - LEDs: Green (26), Yellow (27), Red (32)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Stepper.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <time.h>

// ========== WiFi Configuration ==========
const char* ssid = "YOUR_WIFI_SSID";        // Replace with your WiFi name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password

// ========== Pin Definitions ==========
#define STEPPER_IN1 18
#define STEPPER_IN2 19
#define STEPPER_IN3 21
#define STEPPER_IN4 22
#define SERVO_PIN 13
#define BUTTON_PIN 14
#define BUZZER_PIN 25
#define LED_GREEN 26
#define LED_YELLOW 27
#define LED_RED 32

// ========== Configuration ==========
#define STEPS_PER_REV 2048
#define NUM_COMPARTMENTS 6
#define STEPS_PER_COMPARTMENT (STEPS_PER_REV / NUM_COMPARTMENTS)
#define SERVO_CLOSED 0
#define SERVO_OPEN 90
#define DISPENSE_DELAY 1000

// ========== Global Objects ==========
WebServer server(80);
Stepper stepper(STEPS_PER_REV, STEPPER_IN1, STEPPER_IN3, STEPPER_IN2, STEPPER_IN4);
Servo gateServo;

// ========== Global Variables ==========
int currentCompartment = 0;
int pillCount[NUM_COMPARTMENTS] = {25, 12, 5, 18, 3, 20};
String compartmentNames[NUM_COMPARTMENTS] = {
  "Morning Vitamin", "Afternoon Med", "Evening Tablet", 
  "Night Capsule", "Emergency Med", "Supplement"
};

struct Schedule {
  bool enabled;
  int hour;
  int minute;
  int compartment;
  bool days[7]; // Mon-Sun
};

Schedule schedules[10]; // Up to 10 schedules
int scheduleCount = 0;

int totalDispensedToday = 0;
unsigned long lastDispenseTime = 0;
unsigned long systemStartTime = 0;

// ========== HTML Interface (Minified) ==========
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Pill Dispenser</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:#f5f5f5;padding:20px}
.container{max-width:1200px;margin:0 auto;background:white;border-radius:10px;padding:20px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}
h1{color:#333;margin-bottom:20px;text-align:center}
.status-bar{background:#4CAF50;color:white;padding:15px;border-radius:5px;margin-bottom:20px;display:flex;justify-content:space-around;flex-wrap:wrap}
.status-item{text-align:center;margin:5px}
.compartments{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:15px;margin-bottom:20px}
.compartment{background:#f9f9f9;padding:15px;border-radius:8px;border:2px solid #ddd;transition:all 0.3s}
.compartment.active{border-color:#4CAF50;background:#e8f5e9}
.compartment.low{border-color:#FF9800;background:#fff3e0}
.compartment.empty{border-color:#F44336;background:#ffebee}
.compartment h3{color:#333;margin-bottom:10px;font-size:14px}
.pill-count{font-size:24px;font-weight:bold;color:#4CAF50;margin:10px 0}
.controls{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:15px;margin-bottom:20px}
.control-panel{background:#f9f9f9;padding:20px;border-radius:8px}
.control-panel h2{color:#333;margin-bottom:15px;font-size:18px}
button{background:#4CAF50;color:white;border:none;padding:12px 24px;border-radius:5px;cursor:pointer;font-size:14px;transition:background 0.3s;width:100%;margin-top:10px}
button:hover{background:#45a049}
button.danger{background:#F44336}
button.danger:hover{background:#da190b}
button.secondary{background:#2196F3}
button.secondary:hover{background:#0b7dda}
input,select{width:100%;padding:10px;margin:8px 0;border:1px solid #ddd;border-radius:4px;font-size:14px}
.schedule-list{max-height:300px;overflow-y:auto}
.schedule-item{background:white;padding:10px;margin:5px 0;border-radius:5px;border:1px solid #ddd;display:flex;justify-content:space-between;align-items:center}
.notification{position:fixed;top:20px;right:20px;background:#4CAF50;color:white;padding:15px 20px;border-radius:5px;box-shadow:0 2px 10px rgba(0,0,0,0.2);display:none;z-index:1000;animation:slideIn 0.3s}
@keyframes slideIn{from{transform:translateX(400px)}to{transform:translateX(0)}}
.days-selector{display:flex;gap:5px;flex-wrap:wrap;margin:10px 0}
.day-btn{padding:5px 10px;background:#e0e0e0;border:1px solid #ccc;border-radius:3px;cursor:pointer;font-size:12px}
.day-btn.selected{background:#4CAF50;color:white;border-color:#4CAF50}
@media(max-width:768px){.compartments{grid-template-columns:repeat(2,1fr)}.controls{grid-template-columns:1fr}}
</style>
</head>
<body>
<div class="container">
<h1>🏥 Smart Pill Dispenser</h1>
<div class="status-bar">
<div class="status-item"><strong>Status:</strong> <span id="conn-status">Connected</span></div>
<div class="status-item"><strong>Current:</strong> Compartment <span id="current-comp">0</span></div>
<div class="status-item"><strong>Today:</strong> <span id="dispensed-today">0</span> pills</div>
<div class="status-item"><strong>Uptime:</strong> <span id="uptime">--</span></div>
</div>
<div class="compartments" id="compartments"></div>
<div class="controls">
<div class="control-panel">
<h2>⚡ Manual Control</h2>
<label>Select Compartment:</label>
<select id="manual-comp">
<option value="0">Compartment 0</option>
<option value="1">Compartment 1</option>
<option value="2">Compartment 2</option>
<option value="3">Compartment 3</option>
<option value="4">Compartment 4</option>
<option value="5">Compartment 5</option>
</select>
<button onclick="dispenseNow()">💊 Dispense Now</button>
<button class="secondary" onclick="rotateToCompartment()">🔄 Rotate to Selected</button>
</div>
<div class="control-panel">
<h2>📅 Add Schedule</h2>
<label>Time:</label>
<input type="time" id="sched-time">
<label>Compartment:</label>
<select id="sched-comp">
<option value="0">Compartment 0</option>
<option value="1">Compartment 1</option>
<option value="2">Compartment 2</option>
<option value="3">Compartment 3</option>
<option value="4">Compartment 4</option>
<option value="5">Compartment 5</option>
</select>
<label>Days:</label>
<div class="days-selector" id="days-selector">
<span class="day-btn selected" data-day="0">Mon</span>
<span class="day-btn selected" data-day="1">Tue</span>
<span class="day-btn selected" data-day="2">Wed</span>
<span class="day-btn selected" data-day="3">Thu</span>
<span class="day-btn selected" data-day="4">Fri</span>
<span class="day-btn selected" data-day="5">Sat</span>
<span class="day-btn selected" data-day="6">Sun</span>
</div>
<button onclick="addSchedule()">➕ Add Schedule</button>
</div>
<div class="control-panel">
<h2>📋 Schedules</h2>
<div class="schedule-list" id="schedule-list">No schedules yet</div>
</div>
</div>
</div>
<div class="notification" id="notification"></div>
<script>
let selectedDays=[1,1,1,1,1,1,1];
document.querySelectorAll('.day-btn').forEach((btn,i)=>{
btn.onclick=()=>{
btn.classList.toggle('selected');
selectedDays[i]=btn.classList.contains('selected')?1:0;
}
});
function loadData(){
fetch('/api/status').then(r=>r.json()).then(data=>{
document.getElementById('current-comp').textContent=data.currentCompartment;
document.getElementById('dispensed-today').textContent=data.totalDispensedToday;
document.getElementById('uptime').textContent=data.uptime;
let html='';
data.compartments.forEach((c,i)=>{
let status='';
if(c.count===0)status='empty';
else if(c.count<=5)status='low';
if(i===data.currentCompartment)status+=' active';
html+=`<div class="compartment ${status}">
<h3>Compartment ${i}</h3>
<div class="pill-count">${c.count} pills</div>
<input type="number" min="0" max="50" value="${c.count}" onchange="updateCount(${i},this.value)">
<button onclick="refill(${i})">🔄 Refill</button>
</div>`;
});
document.getElementById('compartments').innerHTML=html;
loadSchedules();
});
}
function loadSchedules(){
fetch('/api/schedules').then(r=>r.json()).then(data=>{
if(data.schedules.length===0){
document.getElementById('schedule-list').innerHTML='No schedules yet';
return;
}
let html='';
data.schedules.forEach((s,i)=>{
html+=`<div class="schedule-item">
<span>${s.time} - Comp ${s.compartment}</span>
<button class="danger" onclick="deleteSchedule(${i})">Delete</button>
</div>`;
});
document.getElementById('schedule-list').innerHTML=html;
});
}
function dispenseNow(){
const comp=document.getElementById('manual-comp').value;
fetch(`/api/dispense?compartment=${comp}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function rotateToCompartment(){
const comp=document.getElementById('manual-comp').value;
fetch(`/api/rotate?compartment=${comp}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function updateCount(comp,count){
fetch(`/api/setcount?compartment=${comp}&count=${count}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function refill(comp){
fetch(`/api/refill?compartment=${comp}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function addSchedule(){
const time=document.getElementById('sched-time').value;
const comp=document.getElementById('sched-comp').value;
if(!time){showNotification('Please select time');return;}
const[h,m]=time.split(':');
fetch(`/api/addschedule?hour=${h}&minute=${m}&compartment=${comp}&days=${selectedDays.join('')}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function deleteSchedule(id){
fetch(`/api/delschedule?id=${id}`).then(r=>r.json()).then(data=>{
showNotification(data.message);
loadData();
});
}
function showNotification(msg){
const notif=document.getElementById('notification');
notif.textContent=msg;
notif.style.display='block';
setTimeout(()=>notif.style.display='none',3000);
}
setInterval(loadData,5000);
loadData();
</script>
</body>
</html>
)rawliteral";

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Smart Pill Dispenser WiFi Server ===");
  
  // Initialize Hardware
  stepper.setSpeed(10);
  gateServo.attach(SERVO_PIN);
  gateServo.write(SERVO_CLOSED);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_GREEN, HIGH);
    playSuccessTone();
  } else {
    Serial.println("\nWiFi Connection Failed!");
    digitalWrite(LED_RED, HIGH);
    playErrorTone();
  }
  
  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/dispense", handleDispense);
  server.on("/api/rotate", handleRotate);
  server.on("/api/setcount", handleSetCount);
  server.on("/api/refill", handleRefill);
  server.on("/api/schedules", handleGetSchedules);
  server.on("/api/addschedule", handleAddSchedule);
  server.on("/api/delschedule", handleDeleteSchedule);
  
  server.begin();
  Serial.println("Web Server Started!");
  Serial.println("Access the interface at: http://" + WiFi.localIP().toString());
  
  systemStartTime = millis();
  
  // Initialize time (NTP)
  configTime(19800, 0, "pool.ntp.org"); // IST timezone
}

// ========== Main Loop ==========
void loop() {
  server.handleClient();
  
  // Check for scheduled dispenses
  checkSchedules();
  
  // Check physical button
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50); // Debounce
    dispensePill(currentCompartment);
  }
  lastButtonState = buttonState;
  
  delay(100);
}

// ========== Web Handlers ==========
void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

void handleStatus() {
  StaticJsonDocument<1024> doc;
  doc["currentCompartment"] = currentCompartment;
  doc["totalDispensedToday"] = totalDispensedToday;
  doc["uptime"] = getUptime();
  
  JsonArray comps = doc.createNestedArray("compartments");
  for (int i = 0; i < NUM_COMPARTMENTS; i++) {
    JsonObject comp = comps.createNestedObject();
    comp["id"] = i;
    comp["name"] = compartmentNames[i];
    comp["count"] = pillCount[i];
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleDispense() {
  int comp = server.arg("compartment").toInt();
  if (comp >= 0 && comp < NUM_COMPARTMENTS) {
    rotateToCompartment(comp);
    delay(500);
    dispensePill(comp);
    server.send(200, "application/json", "{\"message\":\"Dispensing from compartment " + String(comp) + "\"}");
  } else {
    server.send(400, "application/json", "{\"message\":\"Invalid compartment\"}");
  }
}

void handleRotate() {
  int comp = server.arg("compartment").toInt();
  if (comp >= 0 && comp < NUM_COMPARTMENTS) {
    rotateToCompartment(comp);
    server.send(200, "application/json", "{\"message\":\"Rotated to compartment " + String(comp) + "\"}");
  } else {
    server.send(400, "application/json", "{\"message\":\"Invalid compartment\"}");
  }
}

void handleSetCount() {
  int comp = server.arg("compartment").toInt();
  int count = server.arg("count").toInt();
  if (comp >= 0 && comp < NUM_COMPARTMENTS && count >= 0 && count <= 50) {
    pillCount[comp] = count;
    server.send(200, "application/json", "{\"message\":\"Count updated\"}");
  } else {
    server.send(400, "application/json", "{\"message\":\"Invalid parameters\"}");
  }
}

void handleRefill() {
  int comp = server.arg("compartment").toInt();
  if (comp >= 0 && comp < NUM_COMPARTMENTS) {
    pillCount[comp] = 30;
    server.send(200, "application/json", "{\"message\":\"Compartment " + String(comp) + " refilled\"}");
  } else {
    server.send(400, "application/json", "{\"message\":\"Invalid compartment\"}");
  }
}

void handleGetSchedules() {
  StaticJsonDocument<2048> doc;
  JsonArray scheds = doc.createNestedArray("schedules");
  
  for (int i = 0; i < scheduleCount; i++) {
    JsonObject sched = scheds.createNestedObject();
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", schedules[i].hour, schedules[i].minute);
    sched["time"] = timeStr;
    sched["compartment"] = schedules[i].compartment;
    sched["enabled"] = schedules[i].enabled;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAddSchedule() {
  if (scheduleCount >= 10) {
    server.send(400, "application/json", "{\"message\":\"Maximum schedules reached\"}");
    return;
  }
  
  Schedule newSched;
  newSched.hour = server.arg("hour").toInt();
  newSched.minute = server.arg("minute").toInt();
  newSched.compartment = server.arg("compartment").toInt();
  newSched.enabled = true;
  
  String daysStr = server.arg("days");
  for (int i = 0; i < 7; i++) {
    newSched.days[i] = (daysStr[i] == '1');
  }
  
  schedules[scheduleCount++] = newSched;
  server.send(200, "application/json", "{\"message\":\"Schedule added\"}");
}

void handleDeleteSchedule() {
  int id = server.arg("id").toInt();
  if (id >= 0 && id < scheduleCount) {
    for (int i = id; i < scheduleCount - 1; i++) {
      schedules[i] = schedules[i + 1];
    }
    scheduleCount--;
    server.send(200, "application/json", "{\"message\":\"Schedule deleted\"}");
  } else {
    server.send(400, "application/json", "{\"message\":\"Invalid schedule ID\"}");
  }
}

// ========== Hardware Functions ==========
void rotateToCompartment(int targetCompartment) {
  if (targetCompartment == currentCompartment) return;
  
  int compartmentDiff = targetCompartment - currentCompartment;
  int stepsToMove = compartmentDiff * STEPS_PER_COMPARTMENT;
  
  digitalWrite(LED_YELLOW, HIGH);
  stepper.step(stepsToMove);
  digitalWrite(LED_YELLOW, LOW);
  
  currentCompartment = targetCompartment;
  playTone(BUZZER_PIN, 1500, 100);
  digitalWrite(LED_GREEN, HIGH);
  delay(200);
  digitalWrite(LED_GREEN, LOW);
}

void dispensePill(int compartment) {
  if (pillCount[compartment] <= 0) {
    digitalWrite(LED_RED, HIGH);
    playErrorTone();
    delay(500);
    digitalWrite(LED_RED, LOW);
    return;
  }
  
  if (pillCount[compartment] <= 5) {
    digitalWrite(LED_YELLOW, HIGH);
    playTone(BUZZER_PIN, 800, 300);
    delay(100);
  }
  
  digitalWrite(LED_GREEN, HIGH);
  gateServo.write(SERVO_OPEN);
  playTone(BUZZER_PIN, 2000, 200);
  delay(DISPENSE_DELAY);
  gateServo.write(SERVO_CLOSED);
  digitalWrite(LED_GREEN, LOW);
  
  pillCount[compartment]--;
  totalDispensedToday++;
  lastDispenseTime = millis();
  
  playSuccessTone();
}

void checkSchedules() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 30000) return; // Check every 30 seconds
  lastCheck = millis();
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  int currentDay = timeinfo.tm_wday == 0 ? 6 : timeinfo.tm_wday - 1; // Convert to Mon=0
  
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].enabled && 
        schedules[i].hour == currentHour && 
        schedules[i].minute == currentMinute &&
        schedules[i].days[currentDay]) {
      rotateToCompartment(schedules[i].compartment);
      delay(500);
      dispensePill(schedules[i].compartment);
    }
  }
}

// ========== Utility Functions ==========
String getUptime() {
  unsigned long uptime = (millis() - systemStartTime) / 1000;
  int days = uptime / 86400;
  int hours = (uptime % 86400) / 3600;
  int minutes = (uptime % 3600) / 60;
  
  String result = "";
  if (days > 0) result += String(days) + "d ";
  result += String(hours) + "h " + String(minutes) + "m";
  return result;
}

void playTone(int pin, int frequency, int duration) {
  int halfPeriod = 1000000 / frequency / 2;
  long cycles = (long)frequency * duration / 1000;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(pin, LOW);
    delayMicroseconds(halfPeriod);
  }
}

void playSuccessTone() {
  playTone(BUZZER_PIN, 1800, 150);
  delay(50);
  playTone(BUZZER_PIN, 2000, 150);
}

void playErrorTone() {
  playTone(BUZZER_PIN, 500, 500);
  delay(100);
  playTone(BUZZER_PIN, 500, 500);
}
