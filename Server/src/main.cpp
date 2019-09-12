#include <TaskScheduler.h>
#include <rom/rtc.h>
#include <vesc_comms.h>

// https://raw.githubusercontent.com/LilyGO/TTGO-TS/master/Image/TS%20V1.0.jpg
// https://www.aliexpress.com/item/Wemos-18650-Battery-shield-V3-RaspberryPi-Arduino-Lolin-ESP32-OLED-wemos-for-Arduino-ESP32S-WiFi-Modules/32807898232.html?spm=a2g0s.9042311.0.0.27424c4dy1K0TN
/*--------------------------------------------------------------------------------*/

const char compile_date[] = __DATE__ " " __TIME__;
const char file_name[] = __FILE__;

//--------------------------------------------------------------

#define MOTOR_POLE_PAIRS 7
#define WHEEL_DIAMETER_MM 97
#define MOTOR_PULLEY_TEETH 15
#define WHEEL_PULLEY_TEETH 36 // https://hobbyking.com/en_us/gear-set-with-belt.html

uint8_t vesc_packet[PACKET_MAX_LENGTH];

#define GET_FROM_VESC_INTERVAL 500

struct VESC_DATA
{
  float batteryVoltage;
  float motorCurrent;
  bool moving;
  float ampHours;
  float odometer;
};
VESC_DATA vescdata;

#define STATUS_BIT_POWER_DOWN_NORMAL 0
#define STATUS_BIT_CLEARED_TRIP      1

//--------------------------------------------------------------

void waitForFirstPacketFromVesc();

//--------------------------------------------------------------
#define VESC_UART_BAUDRATE 115200

#define POWER_DOWN_BATT_VOLTS_START   33.0
// #define STORE_NAMESPACE "data"
// #define STORE_TOTAL_AMP_HOURS "totalAmpHours"
// #define STORE_TOTAL_ODOMETER "total_odometer"
// #define STORE_POWERED_DOWN "poweredDown"
// #define STORE_LAST_VOLTAGE_READ "lastVolts"

// #include "nvmstorage.h"

//--------------------------------------------------------------------------------

// configure TX RX in vesc_comms.cpp
vesc_comms vesc;

bool handledFirstVescPacket = false;
float lastStableVoltsRead = 0.0;
long lastReport = 0;

//--------------------------------------------------------------
bool controllerOnline = true;
//--------------------------------------------------------------

#include "utils.h"

bool getVescValues()
{
  bool success = vesc.fetch_packet(vesc_packet) > 0;

  if ( success )
  {
    vescdata.batteryVoltage = vesc.get_voltage(vesc_packet);
    vescdata.moving = vesc.get_rpm(vesc_packet) > 50;
    vescdata.motorCurrent = vesc.get_motor_current(vesc_packet);
    vescdata.ampHours = vesc.get_amphours_discharged(vesc_packet);
    vescdata.odometer = getDistanceInMeters(/*tacho*/ vesc.get_tachometer(vesc_packet));
  }
  else
  {
    // vescdata.vescOnline = false;
    vescdata.batteryVoltage = 0.0;
    vescdata.moving = false;
    vescdata.motorCurrent = 0.0;
  }
  return success;
}

//--------------------------------------------------------------

Scheduler runner;

//------------------------------------------------------------------

void handleBoardNotMoving()
{
  lastStableVoltsRead = vescdata.batteryVoltage;
}

void handleBoardMoving()
{
}

void handlePoweringDown()
{
}

// gets called from ble_notify.h
void clearTripMeterAndOdometer() {
}

#include "ble_notify.h"

void vescOfflineCallback()
{
}

void vescOnlineCallback()
{
}

/**************************************************************/

float oldBattVoltage = 0.0;

void tGetFromVESC_callback();
Task tGetFromVESC(GET_FROM_VESC_INTERVAL, TASK_FOREVER, &tGetFromVESC_callback);
void tGetFromVESC_callback()
{
  bool vescOnline = getVescValues() == true;

  if (vescOnline == false)
  {
    Serial.printf("VESC not responding!\n");
    if (millis() - lastReport > 5000)
    {
      lastReport = millis();
    }
  }
  else
  {
    Serial.printf("batt volts: %.1f \n", vescdata.batteryVoltage);
    sendDataToClient();

    bool vescPoweringDown = vescdata.batteryVoltage < POWER_DOWN_BATT_VOLTS_START && vescdata.batteryVoltage > 10;
    if (vescPoweringDown)
    {
      handlePoweringDown();
    }
    else if (vescdata.moving == false)
    {
      handleBoardNotMoving();
    }
    else
    {
      handleBoardMoving();
    }
  }
}
//*************************************************************

void setup()
{
  Serial.begin(115200);

  vesc.init(VESC_UART_BAUDRATE);

  waitForFirstPacketFromVesc();

  runner.startNow();
  runner.addTask( tGetFromVESC );
  tGetFromVESC.enable();

  setupBLE();
}
//--------------------------------------------------------------------------------
void loop()
{
  runner.execute();
}
//*************************************************************
void waitForFirstPacketFromVesc() {
  
  while ( getVescValues() == false ) {
    delay(1);
    yield();
  }
  // just got first packet
  
  // storedTotalAmpHoursOffsetByInitial = recallFloat( STORE_TOTAL_AMP_HOURS ) - vescdata.ampHours;
  // storedTotalOdometerOffsetByInitial = recallFloat( STORE_TOTAL_ODOMETER ) - vescdata.odometer;
}
