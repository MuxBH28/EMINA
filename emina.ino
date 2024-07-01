#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <UrlEncode.h>
#include <WiFiClient.h>

const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";
String phoneNumber = "YOUR PHONE NUMBER";
String apiKey = "YOUR API KEY";
#define DHTPIN 5
#define DHTTYPE    DHT11

DHT dht(DHTPIN, DHTTYPE);

#define led1 14
#define led2 4
#define led3 12
#define buzzer 13

float t = 0.0;
float h = 0.0;

AsyncWebServer server(80);

unsigned long previousMillis = 0;

const long interval = 10000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <link rel="shortcut icon" href="https://i.ibb.co/bBG3dCS/logotext.png" type="image/x-icon">
  <link rel="icon" href="https://i.ibb.co/bBG3dCS/logotext.png" type="image/x-icon">
  <title>EMINA - OV BETA</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
</head>
<body class="jumbotron">
<div class="text-center">
<h2>
  <small class="text-muted">Vatrodojava Online App: </small> EMINA <br><small>Emergency Mapping Interior Network Arduino</small>
</h2>
  <img src="https://i.ibb.co/bBG3dCS/logotext.png">
  <hr>
  <h1 style="color:red;" id="alert"></h1>
  <h2 style="color:#e05c2c;" id="alert2"></h2>
  <h3>
    <i class="fas fa-thermometer-half" style="color:#FF0000;"></i> 
    <span class="dht-labels">Temperatura</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </h3>
  <h3>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Vlažnost zraka:</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </h3>
  <p>
    <a href="https://vatrodojava.rf.gd/#prijava" class="btn btn-outline-danger btn-lg btn-block" target="_blank" role="button">Prijavi požar</a>
  </p>
  <hr>
  <p>
    Having trouble? <a href="mailto:sehicmuhammed7@proton.me">Contact</a>
  </p>
  <small>
    Muhammed Šehić - All Rights Reserved © <script>document.write(new Date().getFullYear());</script>.
  </small>
</div>
</body>
<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/popper.js@1.12.9/dist/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@4.0.0/dist/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 2000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 2000 ) ;

setInterval(function ( ) {
  var alert = document.getElementById("alert");
  var alert2 = document.getElementById("alert2");
  var temp = document.getElementById("temperature").innerHTML;
  temp.toString();
    if (20>temp){
      alert.innerText = "";
      alert2.innerText = "";
    }
    if (temp>=20){
      alert.innerText = "";
      alert2.innerText = "Upozorenje: Obavite provjeru!";
    }
    if (temp>25){
      alert.innerText = "UPOZORENJE: Moguć požar!";
      alert2.innerText = "";
    }
}, 2000 ) ;
</script>
</html>)rawliteral";

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}
void sendMessage(String message){

  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
void setup(){
  Serial.begin(115200);
  dht.begin();
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(buzzer, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("______________________________________");
  Serial.println("Vatrodojava Online App: EMINA");
  Serial.println("BETA Version");
  Serial.println("______________________________________");
  Serial.println("");
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Loading...");
  }

  Serial.println("IP adddress:");
  Serial.println(WiFi.localIP());
  sendMessage("Online Vatrodojava Web App");
  sendMessage("Status: ON");
  sendMessage("IP Address: ");
  sendMessage(WiFi.localIP().toString());
  sendMessage("Disclaimer: Ova poruka je automatska i poslana je jer koristite usluge servisa Online Vatrodojava. Ako mislite da postoji greška, molimo prijavite na linku ispod!");
  sendMessage("https://vatrodojava.rf.gd/");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  server.begin();
}
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
      sendMessage("Upozorenje: Greška sa senzorom!");
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
    }
    else {
      t = newT;
      Serial.println("Temperature: ");
      Serial.println(t);
    }
    float newH = dht.readHumidity();
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
      sendMessage("Upozorenje: Greška sa senzorom!");
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
    }
    else {
      h = newH;
      Serial.println("Humidity:");
      Serial.println(h);
    }
    if (20>=t){
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      noTone(buzzer);
    }
    if (t>20){
      sendMessage("Upozorenje: Molimo uradite provjeru!");
      sendMessage("https://vatrodojava.rf.gd/");
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
      tone(buzzer, 1000);
      delay(500);
      noTone(buzzer);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
    }
    if (t>25){
      sendMessage("UPOZORENJE: Moguć požar!");
      sendMessage("https://vatrodojava.rf.gd/");
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, HIGH);
      tone(buzzer, 1000);
    }
  }
}
