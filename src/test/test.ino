#include "gyro.h"

// первый двигатель

int enA = 10;

int in1 = 9;

int in2 = 8;

// второй двигатель

int enB = 5;

int in3 = 7;

int in4 = 6;

void setup()
{      
  int error;
  uint8_t c;
  Serial.begin(38400);
  Wire.begin();
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
  // инициализируем все пины для управления двигателями как outputs

  pinMode(enA, OUTPUT);
  
  pinMode(enB, OUTPUT);
  
  pinMode(in1, OUTPUT);
  
  pinMode(in2, OUTPUT);
  
  pinMode(in3, OUTPUT);
  
  pinMode(in4, OUTPUT);
}

void _move_stepper(int speedL, int speedR) {
    if (speedR == 0) speedR = 1;
    if (speedL == 0) speedL = 1;
    //speedL = -speedL;
    if(speedL >= 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enA, speedL);
  } else {
    speedL = abs(speedL);
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enA, speedL);      
  }

  if (speedR >= 0) {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enB, speedR);
  } else {
    speedR = abs(speedR);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    analogWrite(enB, speedR);    
  }
}

void _move_stepper_l(){
  _move_stepper(100, -100);
}

void _move_stepper_r(){
  _move_stepper(-100, 100);
}

void _move_stepper_f(){
  _move_stepper(100, 100);
}

void _move_stepper_b(){
  _move_stepper(-100, -100);
}

void loop()
{
    struct angles angles = update_gyro(); 
    Serial.print(angles.x, 2);
    Serial.print(F(","));
    Serial.print(angles.y, 2);
    Serial.print(F(","));
    Serial.print(angles.z, 2);
    Serial.println(F(""));

  //_move_stepper(240, 240);
  //_move_stepper_f();
  _move_stepper_l();
    
}
