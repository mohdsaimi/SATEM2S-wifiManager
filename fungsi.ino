void baca_suhu() {
  suhu = mlx.readObjectTempC();
}

void readDistance(int sensor) {
  //Robojax.com code for sharp IR sensor
  float voltage_temp_average = 0;

  for (int i = 0; i < AVERAGE_OF; i++)
  {
    int sensorValue = analogRead(sensorPin[sensor] );
    delay(1);
    voltage_temp_average += sensorValue * MCU_VOLTAGE / 4095.0;

  }
  voltage_temp_average /= AVERAGE_OF;

  // eqution of the fitting curve

  ////33.9 + -69.5x + 62.3x^2 + -25.4x^3 + 3.83x^4
  distance[sensor] = 33.9 + -69.5 * (voltage_temp_average) + 62.3 * pow(voltage_temp_average, 2) + -25.4 * pow(voltage_temp_average, 3) + 3.83 * pow(voltage_temp_average, 4);
  distance[sensor] -= 1;
  distance_v[sensor] = voltage_temp_average;
  //Robojax.com code for sharp IR sensor
}//readDistanc

void merah1_on() {
  digitalWrite(LED1, HIGH);
}
void merah1_off() {
  digitalWrite(LED1, LOW);
}
void hijau1_on() {
  digitalWrite(LED2, HIGH);
}
void hijau1_off() {
  digitalWrite(LED2, LOW);
}
void biru1_on() {
  digitalWrite(LED3, HIGH);
}
void biru1_off() {
  digitalWrite(LED3, LOW);
}
void merah2_on() {
  digitalWrite(LED4, HIGH);
}
void merah2_off() {
  digitalWrite(LED4, LOW);
}
void hijau2_on() {
  digitalWrite(LED5, HIGH);
}
void hijau2_off() {
  digitalWrite(LED5, LOW);
}
void biru2_on() {
  digitalWrite(LED6, HIGH);
}
void biru2_off() {
  digitalWrite(LED6, LOW);
}
void semua_off() {
  merah1_off();
  merah2_off();
  hijau1_off();
  hijau2_off();
  biru1_off();
  biru2_off();
}

void send_data_favoriot() {
  // Data Json dihantar ke Favoriot
  String json = "{\"device_developer_id\":\"" + myDevice + "\",\"data\":{ \"Suhu\":\"" + suhu + "\",\"RFID\":\"" + no_rfid + "\",\"lokasi\":\"" + lokasi + "\" }}";


  if (client.connect(serverAdd, 80)) {
    // HTTP request:
    client.println("POST /v2/streams HTTP/1.1");
    client.println("Host: apiv2.favoriot.com");
    client.print(String("apikey: "));
    client.println(apikey);
    client.println("Content-Type: application/json");
    client.println("cache-control: no-cache");
    client.print("Content-Length: ");
    int thisLength = json.length();
    client.println(thisLength);
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
}

void send_data_database() {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Prepare your HTTP POST request data
  /*String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                           + "&location=" + sensorLocation + "&value1=" + String(myStr_data1)
                           + "&value2=" + String(myStr_data2) + "&value3=" + String(myStr_data3) + "";
  */
  String httpRequestData = "api_key=" + String(apiKeyValue) + "&suhu=" + float(suhu) + "&rfid=" + int(no_rfid) + "&lokasi=" + String(lokasi) + "";

  Serial.print("httpRequestData: ");
  Serial.println(httpRequestData);

  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);


  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void change_wifi() {

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("---Not Ready---");
  lcd.setCursor(0, 1);
  lcd.print("  192.168.4.1");
  hijau1_off();
  hijau2_off();
  ticker.attach(0.6, tick);
  tone(BUZZER_PIN, NOTE_C4, 1000, BUZZER_CHANNEL);

  WiFiManager wm;

  //reset settings - for testing
  //wm.resetSettings();

  // set configportal timeout
  wm.setConfigPortalTimeout(timeout);

  if (!wm.startConfigPortal("SATEM2S")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected :)");
  ticker.detach();
  noTone(BUZZER_PIN, BUZZER_CHANNEL);

}

void tick()
{
  //toggle state
  digitalWrite(LED1, !digitalRead(LED1));     // set pin to the opposite state
  digitalWrite(LED4, !digitalRead(LED4));
}

void check_suhu(){
  if (suhu > 38.0){
    suhu=suhu-1.1;
  }else if (suhu > 37.8){
    suhu=suhu-1;
  }else if (suhu > 37.5){
    suhu=suhu-0.9;
  }else if(suhu > 37.3){
    suhu=suhu-0.8;
  }else if(suhu > 37.1){
    suhu=suhu-0.5;
  }else if(suhu > 36.9){
    suhu=suhu-0.4;
  }else if(suhu > 36.7){
    suhu=suhu-0.2;
  }else if(suhu > 36.5){
    suhu=suhu-0.1;
  }else if (suhu < 34.6){
    suhu=suhu+1.1;
  }else if (suhu < 34.8){
    suhu=suhu+1;
  }else if (suhu < 35.0){
    suhu=suhu+0.9;
  }else if (suhu < 35.2){
    suhu=suhu+0.8;
  }else if (suhu < 35.4){
    suhu=suhu+0.5;
  }else if(suhu < 35.6){
    suhu=suhu+0.4;
  }else if(suhu < 35.8){
    suhu=suhu+0.2;
  }else if(suhu < 36.0){
    suhu=suhu+0.1;
  }else{
    //do nothing
  }
  
}
