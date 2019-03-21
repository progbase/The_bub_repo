#include "gyro.h"
#include <SPI.h>
#include <Wire.h>
#include "nRF24L01.h"
#include "RF24.h"

// первый двигатель

int enA = 3;

int in1 = 2;

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

  radioSetup();
  Wire.begin();
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  //calibrate_sensors();  
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
    int tmp = speedR;
    speedR = speedL;
    speedL = tmp;
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


#define CH_NUM 0x60   // номер канала (должен совпадать с передатчиком)

#define SIG_SPEED RF24_1MBPS



RF24 radio(9, 10);   // "создать" модуль на пинах 9 и 10 для НАНО/УНО
//RF24 radio(9, 53); // для МЕГИ



byte pipeNo;
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int recieved_data[6];         // массив принятых данных
int telemetry[2];             // массив данных телеметрии (то что шлём на передатчик)



void radioSetup() {         // настройка радио
  radio.begin();               // активировать модуль
  radio.setAutoAck(1);         // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);     // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);    // размер пакета, байт
  radio.openReadingPipe(1, address[0]);   // хотим слушать трубу 0
  radio.setChannel(CH_NUM);               // выбираем канал (в котором нет шумов!)
  radio.setPALevel(SIG_POWER);            // уровень мощности передатчика
  radio.setDataRate(SIG_SPEED);           // скорость обмена
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();         // начать работу
  radio.startListening();  // начинаем слушать эфир, мы приёмный модуль
}

//int small_speed = 40;
//
//void _move_stepper_l(){
//  _move_stepper(small_speed, -small_speed);
//}
//
//void _move_stepper_r(){
//  _move_stepper(-small_speed, small_speed);
//}
//
//void _move_stepper_f(){
//  _move_stepper(small_speed, small_speed);
//}
//
//void _move_stepper_b(){
//  _move_stepper(-small_speed, -small_speed);
//}

    

void moveStepper_free(int x, int y)
{
  int speedL;
  int speedR;
  float x_fl = x;
  float y_fl = y;
  float n = x_fl / 100;
  float k = y_fl / 25;
  if (x > -20 && x < 20) {
    if (y > 10) {
      speedR = 100;
      speedL = -100;
    } else if (y < -10) {
      speedL = 100;
      speedR = -100;
    } else {
       speedL = 0;
       speedR = 0;
    }
  } else if (x >= 20) {
      if (y > 10) {
        speedR = 400 * n;
        speedL = 200 * n / 4;
      } else if (y < -10) {
        speedL = 200 * n;
        speedR = 200 * n / 4;
      } else {
       speedL = 200 * n;
       speedR = 200 * n;
      }
  } else if (x <= -20) {
      if (y > 10) {
        speedR = 200 * n / 4;
        speedL = 400 * n;
      } else if (y < -10) {
        speedL = 200 * n / 4;
        speedR = 400 * n;
      } else {
       speedL = 200 * n;
       speedR = 200 * n;
      }
  }
//  speedL = map(x, -55, 115, -255, 255);
//  speedR = map(x, -55, 115, -254, 254);
//  float k = 0;
//  if (x >= 40 || x <= -40) {
//    float k = map(y, -30, 50, -99, 99);
//  } else {
//    float k = map(y, -80, 80, -99, 99);
//  }
//  float n = k;
//  if(k>0)
//  k=100-k;
//  else
//  k=-100-k;
//  k=k/100.0;
//  if(k>=0)
//  {
//    speedL = k*speedL;
//  }
//  else
//  {
//    k=-k;
//    speedR = k*speedR;
//  }
//  if(speedL>255)
//  speedL=254;
//  if(speedR>255)
//  speedR=254;
//  if (n > 99) {
//    speedR =  - 120;
//    speedL = 120;
//  } else if (n < -99) {
//    speedL =  - 120;
//    speedR = 120;
//  }

  
  _move_stepper(speedR, speedL);
}

void moveStepper_draw(int x, int y)
{
  int speedL;
  int speedR;
  if (x > -20 && x < 20) {
    if (y > 10) {
      speedR = 100;
      speedL = -100;
    } else if (y < -10) {
      speedL = 100;
      speedR = -100;
    } else {
       speedL = 0;
       speedR = 0;
    }
  } else if (x >= 20) {
      
       speedL = 100;
       speedR = 100;
  } else if (x <= -20) {
      
       speedL = -100;
       speedR = -100;
  }
//  speedL = map(x, -55, 115, -255, 255);
//  speedR = map(x, -55, 115, -254, 254);
//  float k = 0;
//  if (x >= 40 || x <= -40) {
//    float k = map(y, -30, 50, -99, 99);
//  } else {
//    float k = map(y, -80, 80, -99, 99);
//  }
//  float n = k;
//  if(k>0)
//  k=100-k;
//  else
//  k=-100-k;
//  k=k/100.0;
//  if(k>=0)
//  {
//    speedL = k*speedL;
//  }
//  else
//  {
//    k=-k;
//    speedR = k*speedR;
//  }
//  if(speedL>255)
//  speedL=254;
//  if(speedR>255)
//  speedR=254;
//  if (n > 99) {
//    speedR =  - 120;
//    speedL = 120;
//  } else if (n < -99) {
//    speedL =  - 120;
//    speedR = 120;
//  }
  
//  if(speedL >= 0) {
//    digitalWrite(in1, HIGH);
//    digitalWrite(in2, LOW);
//    analogWrite(enA, speedL);
//  } else {
//    speedL=255-speedL;
//      digitalWrite(in1, LOW);
//  digitalWrite(in2, HIGH);
//  analogWrite(enA, speedL);      
//  }
//
//  if (speedR >= 0) {
//    digitalWrite(in3, HIGH);
//    digitalWrite(in4, LOW);
//    analogWrite(enB, speedR);
//  } else {
//    speedR=255-speedR;
//      digitalWrite(in3, LOW);
//  digitalWrite(in4, HIGH);
//  analogWrite(enB, speedR);    
//  }

  
  _move_stepper(speedR, speedL);
}
    int x = 0;
    int y = 0;
    #define free_mode 0
    #define draw_mode 1
    int mode = draw_mode;
void loop()
{
    
    while (radio.available(&pipeNo)) {                                 // слушаем эфир
    radio.read( &recieved_data, sizeof(recieved_data));              // чиатем входящий сигнал
//    Serial.print(recieved_data[0]);
      y = recieved_data[0];
      x = recieved_data[1];
//        Serial.print("    ");
//    Serial.println(recieved_data[1]);

    }

    if (mode == free_mode){
          moveStepper_free(x, y);
    } else {
      moveStepper_draw(x, y);
    }
//        int forw_back = map(x, -90, 90, -255, 255);
//        int left_right = map(y, -90, 90, -255, 255);
//
//        Serial.println(l);
//        Serial.println(r);
//        _move_stepper(l, r);
////        Serial.print(left_speed); Serial.println("  ");
////        Serial.println(right_speed); Serial.println("  ");
    
    
  
    
}
