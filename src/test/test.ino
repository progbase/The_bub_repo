#include "gyro.h"





void setup()
{      
  int error;
  uint8_t c;


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

void loop()
{
    struct angles angles = update_gyro(); 
    Serial.print(angles.x, 2);
    Serial.print(F(","));
    Serial.print(angles.y, 2);
    Serial.print(F(","));
    Serial.print(angles.z, 2);
    Serial.println(F(""));
}
