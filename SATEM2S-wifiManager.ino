#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

#include <WiFiManager.h>

#include <rdm6300.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <Tone32.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define BUZZER_PIN 13
#define BUZZER_CHANNEL 1
#define RDM6300_RX_PIN 16
#define TRIGGER_PIN 32 //pin wifimanager

#include <Ticker.h>
Ticker ticker;

int timeout = 120; //120

Rdm6300 rdm6300;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const char* serverName = "http://20.1.11.197/satem2s/post_data_satem2s.php";
String apiKeyValue = "5a1m1th3gr34t_4Lw4y5pr0t3ct3d8yth34Lm19hty";

//Maklumat Akaun Favoriot
const String myDevice = "satem2s@mohdsaimi";
char serverAdd [] = "apiv2.favoriot.com";
String apikey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6Im1vaGRzYWltaSIsInJlYWRfd3JpdGUiOnRydWUsImlhdCI6MTU4ODY2MzczMH0.OKEzyp3XUAmV3swaxnP4FiVMUHxqYFhCfLyxYXPfAiQ";
WiFiClient client;

float suhu,suhu_avg=0;
int LED4 = 12; //merah
int LED5 = 14; //hijau
int LED6 = 27; //biru
int LED1 = 26; //merah
int LED2 = 25; //hijau
int LED3 = 33; //biru
int no_rfid = 0;
int loopagain = 0;
int lokasi = 1001;
float jarak = 0.0;

const double T_offset=0.00; //adjustment parameter for sensor calibration. default 0.00
const double Obj_Emiss=0.97; // se more in https://www.thermoworks.com/emissivity-table

const int sensorPin[] = {34};
float distance[1];
float distance_v[1];
const int AVERAGE_OF = 50;
const float MCU_VOLTAGE = 3.3;//5.0

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  rdm6300.begin(RDM6300_RX_PIN);
  mlx.begin();
  //uncomment one o both lines for to change that parameters.
  //mlx.setOffsetTemp(T_offset);  //parametro de ajuste para calibración del sensor. por defecto 0.00

  //this line needs to be loaded the first time to adjust the emissivity value, then, you need to turn it off again
  //mlx.writeEmissivity(Obj_Emiss);  //parameter to define the emissivity of the sensed object

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
  Serial.println("\n Starting");
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

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
  bool res;

  res = wm.autoConnect("SATEM2S", "password"); // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected :)");
    ticker.detach();
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
  }
  if ( digitalRead(TRIGGER_PIN) == LOW) {
    change_wifi();
  }

}

void loop() {
mula:

  semua_off();
  no_rfid = 0;
  readDistance(0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
  while (distance[0] >= 14) {
    if ( digitalRead(TRIGGER_PIN) == LOW) {
      change_wifi();
      semua_off();
    }
    if (WiFi.status() != WL_CONNECTED) {
      ESP.restart();
    }

    hijau1_on();
    lcd.setCursor(0, 1);
    Serial.println(distance[0]);
    //baca_jarak();
    readDistance(0);
    lcd.print("Ready...");
  }
  lcd.clear();
  loopagain = 0;
  //while ((distance[0] > 3.5) || (distance[0] < 3.2)) { //2-3cm
  while ((distance[0] > 3.5) || (distance[0] < 3.3)) {
    hijau1_on();
    tone(BUZZER_PIN, NOTE_D4, 100, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    lcd.setCursor(0, 0);
    //baca_jarak();
    readDistance(0);
    lcd.print("Distance 3-3.5cm");
    lcd.setCursor(4, 1);
    lcd.print(distance[0]);
    lcd.print("cm");
    baca_suhu();
    Serial.print(distance[0]);
    Serial.print("        ");
    Serial.println(suhu);
    hijau1_off();
    loopagain++;
    if (loopagain > 20) {
      loopagain = 0;
      goto mula;
    }
  }

  lcd.clear();
  baca_suhu();
  suhu_avg=0;
  for (int x = 0; x < 20; x++) {
    baca_suhu();
    delay(20);
    suhu_avg=suhu_avg+suhu;
  }

  suhu=suhu_avg/20;
  
  //check_suhu();
  hijau1_on();
  lcd.setCursor(0, 0);
  lcd.print("Temp :");
  lcd.print(suhu);
  Serial.println(suhu);
  rdm6300.begin(RDM6300_RX_PIN);
  delay (500);

  loopagain = 0;
  while (no_rfid == 0) {
    if (rdm6300.update()) {
      no_rfid = rdm6300.get_tag_id();
      delay(10);
      break;
    }
    lcd.setCursor(0, 1);
    lcd.print("Scan RFID...");
    hijau2_on();
    tone(BUZZER_PIN, NOTE_E4, 300, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    hijau2_off();
    tone(BUZZER_PIN, NOTE_G4, 300, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    loopagain++;
    if (loopagain > 20) {
      loopagain = 0;
      goto mula;
    }

  }
  rdm6300.end();
  lcd.setCursor(0, 1);
  hijau2_on();
  lcd.print("RFID :");
  lcd.print(no_rfid);
  Serial.println(no_rfid);

  //send data to database
  Serial.println("Sending data to database....");
  send_data_database();
  //end send data to database
  
  //send data to favoriot
  Serial.println("Sending data to favoriot....");
  send_data_favoriot();
  //end send data to favoriot
  
  no_rfid = 0;

  while (1) {
    delay (5000);
    break;
  }
}
