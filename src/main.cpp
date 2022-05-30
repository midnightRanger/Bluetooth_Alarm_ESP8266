#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Button.h"

#define latchPin 4
#define clockPin 7
#define dataPin 8
#define display_delay 16

#define BUZZER_PIN 3

//коды для цифр от 0 до 10 под соответствующими индексами
int digit_array[] = {~B11111100, ~B01100000, ~B11011010, ~B11110010, ~B01100110, ~B10110110, ~B10111110, ~B11100000, ~B11111110, ~B11110110, ~B00000000};
//коды для каждого разряда
int class_array[] = {0xF1, 0xF2, 0xF4, 0xF8};
//массив с цифрами числа, которое ввел пользователь
int decade_array[4];

String val;
long countdown_millis;
bool is_timer_running;

SoftwareSerial bluetooth(1, 0);

int seconds;
int current_seconds = 60;

button btn(A1);
long displaying_millis;

void setup()
{
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 255);

  bluetooth.begin(9600);
  bluetooth.flush();
  while (!bluetooth.available())
    ;
  bluetooth.readString();
  bluetooth.println("Hi, I am Arduino Ostapenko");
  current_seconds = 60;
  bluetooth.println("Текущее значение: " + (String)current_seconds);
  seconds = 60;
  int mins = current_seconds / 60;
  int secs = current_seconds % 60;
  decade_array[0] = mins / 10;
  decade_array[1] = mins % 10;
  decade_array[2] = secs / 10;
  decade_array[3] = secs % 10;
}

bool is_correct_num(int num)
{
  return (num >= 0 && num < 60);
}

void redraw_display()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, digit_array[decade_array[i]]);
    shiftOut(dataPin, clockPin, MSBFIRST, class_array[i]);
    digitalWrite(latchPin, HIGH);
  }
}

void update_timer()
{
  if (millis() - countdown_millis > 1000 && is_timer_running)
  {
    bluetooth.println(current_seconds);
    current_seconds--;
    int mins = current_seconds / 60;
    int secs = current_seconds % 60;
    decade_array[0] = mins / 10;
    decade_array[1] = mins % 10;
    decade_array[2] = secs / 10;
    decade_array[3] = secs % 10;

    countdown_millis = millis();
    if (current_seconds <= 0)
    {
      is_timer_running = false;
      digitalWrite(BUZZER_PIN, 0);
    }
  }
}

void command_handle(String command)
{
  //command.trim();
  if (command.startsWith("SLEEP"))
  {

    if (command[7] == ':')
    {
      int min = command.substring(5, 7).toInt();
      int sec = command.substring(8, 10).toInt();
      if (is_correct_num(min) && is_correct_num(sec))
      {
        seconds = min * 60 + sec;
        current_seconds = min * 60 + sec;
        is_timer_running = true;
        countdown_millis = millis();
        bluetooth.println("Timer is set on :" + (String)current_seconds);
      }
      else
      {
        bluetooth.println("Invalid seconds/minutes -  must be beetwen 1 and 59");
      }
    }
    else
    {
      bluetooth.println("Should use the divider : ! ");
    }
  }

  else if (command.startsWith("START"))
  {
    is_timer_running = true;
  }

  else if (command.startsWith("PAUSE"))
  {
    is_timer_running = false;
  }

  else if (command.startsWith("STOP"))
  {
    bluetooth.println("Введена команда - STOP");
    current_seconds = seconds;
    is_timer_running = false;
  }

  else
  {
    bluetooth.println("Неизвестная команда");
  }
}

void loop()
{
  update_timer();
  redraw_display();

  if (btn.click())
  {
    digitalWrite(BUZZER_PIN, 255);
    Serial.println("Будильник отключен");
  }

  if (bluetooth.available())
  {
    val = bluetooth.readString();
    bluetooth.println((String)"INPUT: " +val);

    command_handle(val);
  }
}