#include "Arduino.h"
#include "Arduino_BHY2Host.h"

#include <Wire.h>

#include "altitude.h"

#include "altitude_eqn.h"
#include "DCMotorServo.h" // from https://github.com/CameronBrooks11/DCMotorServo

SensorXYZ accel(SENSOR_ID_ACC);
SensorOrientation ori(SENSOR_ID_ORI);
Sensor baro(SENSOR_ID_BARO);
SensorXYZ linaccel(SENSOR_ID_LACC);
SensorXYZ gyro(SENSOR_ID_GYRO);


// Altitude estimator
static AltitudeEstimator altitude = AltitudeEstimator(0.0005, // sigma Accel
        0.0005, // sigma Gyro
        0.018,   // sigma Baro
        0.5, // ca
        0.1);// accelThreshold

static void imuRead(float gyro[3], float accel[3])
{
      gyro[0] = gyro.x()*PI/180.0/4096.0; // radians/second
      gyro[1] = gyro.y()*PI/180.0/4096.0;
      gyro[2] = gyro.z()*PI/180.0/4096.0;
      // and acceleration values
      accel[0] = linaccel.x()/4096.0; // g-unit
      accel[1] = linaccel.y()/4096.0;
      accel[2] = linaccel.z()/4096.0;
}

void setup() {
  // debug port
  Serial.begin(115200);
  while(!Serial);

 // NOTE: if Nicla is used as a Shield on top of a MKR board we must use:
  BHY2Host.begin(false, NICLA_AS_SHIELD);


  accel.begin();
  ori.begin();
  baro.begin();
  linaccel.begin();
  gyro.begin();
}


void loop()
{
  BHY2Host.update();

  currentTime = millis();
  if ((currentTime - pastTime) > 50)
  {
    // get all necessary data
    float pressure = baro.value();
    baro.update(pressure);
    // Assuming pressure is in Pa, converting to millibars by dividing by 100    

    float baroHeight = AltitudeEquation::pressureToAltitudeMeters(pressure / 100); 
    uint32_t timestamp = micros();
    float accelData[3];
    float gyroData[3];
    imuRead(gyroData, accelData);
    altitude.estimate(accelData, gyroData, baroHeight, timestamp);
    Serial.print(baroHeight);
    Serial.print(",");
    Serial.print(altitude.getAltitude());
    Serial.print(",");
    Serial.print(altitude.getVerticalVelocity());
    Serial.print(",");
    Serial.println(altitude.getVerticalAcceleration());
    pastTime = currentTime;
  }


/*
static auto printTime = millis();
  if (millis() - printTime >= 500) {
    printTime = millis();
    Serial.println(String("Acceleration values: ") + accel.toString());
    Serial.println(String("Orientation values: ") + ori.toString());
    Serial.println(String("Barometer value: ") + baro.toString());
    Serial.println(String("Linear Acceleration: ") + linaccel.toString());
    printVelocity();
  }
*/

}


void printVelocity()
{
    // Measure state:  
  vec3_t linaccel_data = { linaccel.x()/4096.0, linaccel.y()/4096.0, linaccel.z()/4096.0};    // g-unit
  vec3_t gyro_data = { gyro.x()*PI/180.0/4096.0, gyro.y()*PI/180.0/4096.0, gyro.z()*PI/180.0/4096.0};     // radians/second

  // Print linear acceleration data with units
  Serial.print("Linear Acceleration X: ");
  Serial.print(linaccel_data.x, 6); // Assuming 6 decimal places for precision
  Serial.print(" g, Y: ");
  Serial.print(linaccel_data.y, 6);
  Serial.print(" g, Z: ");
  Serial.print(linaccel_data.z, 6);
  Serial.println(" g");

  // Print gyro data with units
  Serial.print("Gyro Data X: ");
  Serial.print(gyro_data.x, 6); // Assuming 6 decimal places for precision
  Serial.print(" rad/s, Y: ");
  Serial.print(gyro_data.y, 6);
  Serial.print(" rad/s, Z: ");
  Serial.print(gyro_data.z, 6);
  Serial.println(" rad/s");

}
