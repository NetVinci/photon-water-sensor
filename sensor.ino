// This #include statement was automatically added by the Particle IDE.
#include <elapsedMillis.h>

// This #include statement was automatically added by the Particle IDE.
#include "elapsedMillis/elapsedMillis.h"

// Version of the code that is being used.
String _version = "0.05";

//this reads the flood sensors every 2 seconds
#define FLOOD_READ_INTERVAL 2000

//this defines the frequency of the notifications sent to the user
#define FLOOD_FIRST_ALARM 10000   //10 seconds
#define FLOOD_SECOND_ALARM 30000  //.5 minute
#define FLOOD_THIRD_ALARM 60000   //1 minutes
#define FLOOD_FOURTH_ALARM 300000 //5 minutes - We will alert every 5 minutes until the sensor is dry.

#define FLOOD_NOTIF "FLOOD"

elapsedMillis flood_timer;
elapsedMillis flood_alarm_timer;

int flood_alarms_array[4] = {FLOOD_FIRST_ALARM, FLOOD_SECOND_ALARM, FLOOD_THIRD_ALARM, FLOOD_FOURTH_ALARM};
int flood_alarm_index = 0;
bool flood_detected = false;
unsigned long flood_next_alarm = 0;

// Define the sensors that will be used. More sensors can be added to a board as necessary.
// As of now there are only 3 sensors connected.
int FLOOD_SENSOR_1 = D0;
int FLOOD_SENSOR_2 = D1;
int FLOOD_SENSOR_3 = D2;

// LED pin. This can be connected to show a led light.
int LED = D7;

void setup()
{
    // Mode of the pins are PULLUP for the sensors.
    pinMode(FLOOD_SENSOR_1, INPUT_PULLUP);
    pinMode(FLOOD_SENSOR_2, INPUT_PULLUP);
    pinMode(FLOOD_SENSOR_3, INPUT_PULLUP);
    pinMode(LED, OUTPUT);

    // Publish event message
    Spark.publish("the device starting", "Firmware version: " + _version, 60, PRIVATE);
}

// Set the loop to do the flood check and read global to detect the flood and notify the users.
void loop()
{

    flood_check();

    if (flood_detected)
    {
        flood_notify_user();
    }
}

/*******************************************************************************
 * Function Name  : flood_check
 * Description    : check water leak sensor at FLOOD_READ_INTERVAL, turns on led on D7 and raises alarm if water is detected
 * Return         : 0
 *******************************************************************************/
int flood_check()
{
    if (flood_timer < FLOOD_READ_INTERVAL)
    {
        return 0;
    }

    //time is up, so reset timer
    flood_timer = 0;

    // Read the sensor pins for detection of the leak with the water.
    if (not digitalRead(FLOOD_SENSOR_1) && not digitalRead(FLOOD_SENSOR_2) && not digitalRead(FLOOD_SENSOR_3))
    {

        //if flood is already detected, no need to do anything, since an alarm will be fired
        if (flood_detected)
        {
            return 0;
        }

        flood_detected = true;

        //reset alarm timer
        flood_alarm_timer = 0;

        //set next alarm
        flood_alarm_index = 0;
        flood_next_alarm = flood_alarms_array[0];

        digitalWrite(LED, HIGH);
    }
    else
    {

        digitalWrite(LED, LOW);
        flood_detected = false;
    }
    return 0;
}

/*******************************************************************************
 * Function Name  : flood_notify_user
 * Description    : will fire notifications to user at scheduled intervals
 * Return         : 0
 *******************************************************************************/
int flood_notify_user()
{

    if (flood_alarm_timer < flood_next_alarm)
    {
        return 0;
    }

    //time is up, so reset timer
    flood_alarm_timer = 0;

    //set next alarm or just keep current one if there are no more alarms to set
    if (flood_alarm_index < arraySize(flood_alarms_array) - 1)
    {
        flood_alarm_index = flood_alarm_index + 1;
        flood_next_alarm = flood_alarms_array[flood_alarm_index];
    }

    // Make Integration Call to twilioCall which calls a list of phone numbers to notify them a leak has been detected.
    Particle.publish("twilioCall", "A Flood Detected!!!", 60, PRIVATE);

    // Make Integration Call to waterDetected which pushes a Pushover alert to the user.
    Particle.publish("waterDetected", "A Flood Detected!!!", 60, PRIVATE);

    //send an alarm to user (this one goes to the dashboard)
    Spark.publish(FLOOD_NOTIF, "Flood detected!", 60, PRIVATE);

    return 0;
}