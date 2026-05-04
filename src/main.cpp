#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- RTC ----------
RTC_DS1307 rtc;

// ---------- DHT ----------
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------- Buttons ----------
#define BTN1 4
#define BTN2 3

// ---------- Days ----------
const char* daysOfWeek[] = {
  "Domingo", "Segunda", "Terca",
  "Quarta", "Quinta", "Sexta", "Sabado"
};

// ---------- Adjust Modes ----------
enum AdjustMode {
  MODE_NORMAL,
  MODE_HOUR,
  MODE_MINUTE,
  MODE_DAY,
  MODE_MONTH,
  MODE_YEAR
};

AdjustMode mode = MODE_NORMAL;

// Editable values
int editHour, editMinute, editDay, editMonth, editYear;

// Button states
bool lastBtn1 = HIGH;
bool lastBtn2 = HIGH;

void drawScreen(DateTime now, float temp, float hum);

void setup() {
  Serial.begin(9600);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  dht.begin();

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  DateTime now = rtc.now();

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    temp = 0;
    hum = 0;
  }

  // ---------- BUTTON LOGIC ----------
  bool btn1 = digitalRead(BTN1);
  bool btn2 = digitalRead(BTN2);

  // BTN1 → change mode
  if (lastBtn1 == HIGH && btn1 == LOW) {
    mode = (AdjustMode)((mode + 1) % 6);

    // Load current values when entering edit
    editHour   = now.hour();
    editMinute = now.minute();
    editDay    = now.day();
    editMonth  = now.month();
    editYear   = now.year();
  }

  // BTN2 → increment
  if (lastBtn2 == HIGH && btn2 == LOW) {
    switch (mode) {
      case MODE_HOUR:   editHour = (editHour + 1) % 24; break;
      case MODE_MINUTE: editMinute = (editMinute + 1) % 60; break;
      case MODE_DAY:    editDay = (editDay % 31) + 1; break;
      case MODE_MONTH:  editMonth = (editMonth % 12) + 1; break;
      case MODE_YEAR:   editYear++; break;
      default: break;
    }
  }

  // Save when exiting edit mode
  if (mode == MODE_NORMAL && lastBtn1 == LOW && btn1 == HIGH) {
    rtc.adjust(DateTime(editYear, editMonth, editDay, editHour, editMinute, 0));
  }

  lastBtn1 = btn1;
  lastBtn2 = btn2;

  drawScreen(now, temp, hum);

  delay(150);
}

void drawScreen(DateTime now, float temp, float hum) {
  display.clearDisplay();

  int h  = (mode == MODE_NORMAL) ? now.hour()   : editHour;
  int m  = (mode == MODE_NORMAL) ? now.minute() : editMinute;
  int d  = (mode == MODE_NORMAL) ? now.day()    : editDay;
  int mo = (mode == MODE_NORMAL) ? now.month()  : editMonth;
  int y  = (mode == MODE_NORMAL) ? now.year()   : editYear;

  // TIME
  display.setTextSize(2);
  display.setCursor(0, 0);
  char buf[10];
  sprintf(buf, "%02d:%02d:%02d", h, m, now.second());
  display.println(buf);

  // DAY
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println(daysOfWeek[now.dayOfTheWeek()]);

  // DATE
  display.setCursor(0, 30);
  sprintf(buf, "%02d/%02d/%04d", d, mo, y);
  display.println(buf);

  // MODE INDICATOR
  display.setCursor(0, 40);
  switch (mode) {
    case MODE_HOUR:   display.println("Ajuste: Hora"); break;
    case MODE_MINUTE: display.println("Ajuste: Minuto"); break;
    case MODE_DAY:    display.println("Ajuste: Dia"); break;
    case MODE_MONTH:  display.println("Ajuste: Mes"); break;
    case MODE_YEAR:   display.println("Ajuste: Ano"); break;
    default:          display.println("Normal"); break;
  }

  // TEMP + HUM
  display.setCursor(0, 52);
  display.print("T:");
  display.print(temp);
  display.print("C ");

  display.print("H:");
  display.print(hum);
  display.print("%");

  display.display();
}