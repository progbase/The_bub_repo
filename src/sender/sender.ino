#include "gyro.h"

#include "radios.h"
int flexSensorPin = A0; 


void setup() {
  transmit_data[0]=1;
  transmit_data[1]=1; 
  transmit_data[2]=400;
  int error;
  uint8_t c;

  radioSetup();
  for(int i=0; i<3; i++) {
    transmit_data[i]=0;
  }
  Serial.begin(38400);
  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();


  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);

  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  

  // Clear the 'sleep' bit to start the sensor.
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  
  //Initialize the angles
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
}

void loop() {
  struct angles angles = update_gyro(); 
  transmit_data[0] = -angles.x;
  transmit_data[1] = angles.y;
  transmit_data[2] = analogRead(flexSensorPin); //

  Serial.print(angles.x);//to test
  Serial.print("    ");
  Serial.println(angles.y);
  if (radio.write(&transmit_data, sizeof(transmit_data))) {    // отправка пакета transmit_data
    trnsmtd_pack++;
  } else {
    failed_pack++;
  }
  delay(10);
}
