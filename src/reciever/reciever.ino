#include "gyro.h"
#include <SPI.h>
#include <Wire.h>
#include "nRF24L01.h"
#include "RF24.h"
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
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);

  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
  // инициализируем все пины для управления двигателями как outputs

  pinMode(enA, OUTPUT);
  
  pinMode(enB, OUTPUT);
  
  pinMode(in1, OUTPUT);
  
  pinMode(in2, OUTPUT);
  
  pinMode(in3, OUTPUT);
  
  pinMode(in4, OUTPUT);
}

void _move_stepper(int speedR, int speedL) {
  if (speedR == 0) speedR = 1;
  if (speedL == 0) speedL = 1;
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

/*
   Пример скетча "управление с телеметрией", то есть модуль ПЕРЕДАТЧИК
   шлёт на ПРИЁМНИК команды управления, ПРИЁМНИК при получении пакета данных
   отправляет ПЕРЕДАТЧИКУ пакет телеметрии (какие-то свои данные). ПЕРЕДАТЧИК
   эти данные принимает. Вот такие пироги. Также в этом примере реализован 
   расчёт RSSI (процент ошибок связи), на основании которого можно судить о 
   качестве связи между модулями.
*/

// ЭТО СКЕТЧ ПРИЁМНИКА!!!

//--------------------- НАСТРОЙКИ ----------------------
#define CH_NUM 0x60   // номер канала (должен совпадать с передатчиком)
//--------------------- НАСТРОЙКИ ----------------------

//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------
// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_MIN

// СКОРОСТЬ ОБМЕНА
// На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
// должна быть одинакова на приёмнике и передатчике!
// при самой низкой скорости имеем самую высокую чувствительность и дальность!!
// ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
#define SIG_SPEED RF24_1MBPS
//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------

//--------------------- БИБЛИОТЕКИ ----------------------

RF24 radio(9, 10);   // "создать" модуль на пинах 9 и 10 для НАНО/УНО
//RF24 radio(9, 53); // для МЕГИ
//--------------------- БИБЛИОТЕКИ ----------------------

//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte pipeNo;
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int recieved_data[3];         // массив принятых данных
int telemetry[2];             // массив данных телеметрии (то что шлём на передатчик)
//--------------------- ПЕРЕМЕННЫЕ ----------------------


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

int small_speed = 40;

void _move_stepper_l(){
  _move_stepper(small_speed, -small_speed);
}

void _move_stepper_r(){
  _move_stepper(-small_speed, small_speed);
}

void _move_stepper_f(){
  _move_stepper(small_speed, small_speed);
}

void _move_stepper_b(){
  _move_stepper(-small_speed, -small_speed);
}

// @todo description of this
int x = 0;
int y = 0;
#define free_mode 0
#define draw_mode 1
int mode = free_mode;
// end

void moveStepper(int x, int y) {
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
      speedL = 200 * n / 8;
    } else if (y < -10) {
      speedL = 200 * n;
      speedR = 200 * n / 8;
    } else {
     speedL = 200 * n;
     speedR = 200 * n;
    }
  } else if (x <= -20) {
    if (y > 10) {
      speedR = 200 * n / 8;
      speedL = 400 * n;
    } else if (y < -10) {
      speedL = 200 * n / 8;
      speedR = 400 * n;
    } else {
     speedL = 200 * n;
     speedR = 200 * n;
    }
  }

  _move_stepper(speedR, speedL);
}

void moveStepper_draw(int x, int y) {
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
  
  _move_stepper(speedR, speedL);
}

bool marker_down;

String str;
void loop() {
  delay(100);
  int flex = 400;
  while (radio.available(&pipeNo)) {                                 // слушаем эфир
    radio.read( &recieved_data, sizeof(recieved_data));              // чиатем входящий сигнал
    y = recieved_data[0];
    x = recieved_data[1];
    flex = recieved_data[2];
  }

  if(mode == draw_mode)
  {
    moveStepper_draw(x , y);
  }
  else
  {
    moveStepper(x, y);
  } 


  flex = flex - flex % 10;
  Serial.print("    ");
    Serial.println(flex);
  if (flex > 375) {
    servo.write(100);
  } else if (flex < 350){
    servo.write(170);
  }

  if (Serial.available() > 0)     // Read data only when you receive data:
   {
       int incomingByte = Serial.read();
 
        // отсылаем то, что получили
        Serial.print("I received: ");
        Serial.println(incomingByte, DEC);
   }
}
