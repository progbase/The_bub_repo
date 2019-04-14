#include <SPI.h> 
#include <Wire.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "radios.h"
#include <Servo.h>

int servoPin = 4;

// первый двигатель

int enA = 3;

int in1 = 2;

int in2 = 8;

// второй двигатель

int enB = 5;

int in3 = 7;

int in4 = 6;

Servo servo;

void setup()
{
  int error;
  uint8_t c;
  Serial.begin(38400);
  servo.attach(servoPin);

  radioSetup();
  Wire.begin();
  
  // инициализируем все пины для управления двигателями как outputs

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

void _move_stepper(int speedR, int speedL)
{
  if (speedR == 0)
    speedR = 1;
  if (speedL == 0)
    speedL = 1;
  if (speedL >= 0)
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, speedL);
  }
  else
  {
    speedL = abs(speedL);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, speedL);
  }

  if (speedR >= 0)
  {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, speedR);
  }
  else
  {
    speedR = abs(speedR);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, speedR);
  }
}



int x = 0;
int y = 0;
#define free_mode 0
#define draw_mode 1
int mode = free_mode; // default mode without bluetooth connection

void moveStepper_free(int x, int y)
{
  int speedL;
  int speedR;
  float x_fl = x;
  float y_fl = y;
  float n = x_fl / 100;
  float k = y_fl / 25;
  if (x > -20 && x < 20)
  {
    if (y > 10)
    {
      speedR = 100;
      speedL = -100;
    }
    else if (y < -10)
    {
      speedL = 100;
      speedR = -100;
    }
    else
    {
      speedL = 0;
      speedR = 0;
    }
  }
  else if (x >= 20)
  {
    if (y > 10)
    {
      speedR = 400 * n;
      speedL = 200 * n / 8;
    }
    else if (y < -10)
    {
      speedL = 200 * n;
      speedR = 200 * n / 8;
    }
    else
    {
      speedL = 200 * n;
      speedR = 200 * n;
    }
  }
  else if (x <= -20)
  {
    if (y > 10)
    {
      speedR = 200 * n / 8;
      speedL = 400 * n;
    }
    else if (y < -10)
    {
      speedL = 200 * n / 8;
      speedR = 400 * n;
    }
    else
    {
      speedL = 200 * n;
      speedR = 200 * n;
    }
  }

  _move_stepper(speedR, speedL);
}

void moveStepper_draw(int x, int y)
{
  int speedL;
  int speedR;
  int draw_speed = 80;
  if (x > -20 && x < 20)
  {
    if (y > 10)
    {
      speedR = draw_speed;
      speedL = -draw_speed;
    }
    else if (y < -10)
    {
      speedL = draw_speed;
      speedR = -draw_speed;
    }
    else
    {
      speedL = 0;
      speedR = 0;
    }
  }
  else if (x >= 20)
  {
    speedL = draw_speed;
    speedR = draw_speed;
  }
  else if (x <= -20)
  {
    speedL = -draw_speed;
    speedR = -draw_speed;
  }

  _move_stepper(speedR, speedL);
}

void loop()
{
  delay(100);
  int flex = 400;
  while (radio.available(&pipeNo))
  {                                                    // слушаем эфир
    radio.read(&recieved_data, sizeof(recieved_data)); // чиатем входящий сигнал
    y = recieved_data[0];
    x = recieved_data[1];
    flex = recieved_data[2];
  }

  if (mode == draw_mode)
  {
    moveStepper_draw(x, y);
  }
  else
  {
    moveStepper_free(x, y);
  }

  if (mode == draw_mode)
  {
    flex = flex - flex % 10;
    Serial.print("    ");
    Serial.println(flex);
    if (flex > 375)
    {
      servo.write(100);
    }
    else if (flex < 350)
    {
      servo.write(170);
    }
  }
  else
  {
    servo.write(100);
  }

  if (Serial.available() > 0) // Read data only when you receive data:
  {
    int incomingByte = Serial.read();

    // Serial.print("I received: ");
    // Serial.println(incomingByte, DEC); //to test
    if (incomingByte == 0)
    {
      mode = free_mode;
    }
    else
    {
      mode = draw_mode;
    }
    incomingByte = Serial.read();
  }
}
