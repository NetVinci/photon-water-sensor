// This #include statement was automatically added by the Particle IDE.
#include <elapsedMillis.h>

// This #include statement was automatically added by the Particle IDE.
#include "elapsedMillis/elapsedMillis.h"

String _version = "0.04";

//this reads the flood sensor every 2 seconds
#define FLOOD_READ_INTERVAL 2000

//this defines the frequency of the notifications sent to the user
#define FLOOD_FIRST_ALARM 10000 //10 seconds
#define FLOOD_SECOND_ALARM 60000 //1 minute
#define FLOOD_THIRD_ALARM 300000 //5 minutes
#define FLOOD_FOURTH_ALARM 900000 //15 minutes
#define FLOOD_FIFTH_ALARM 3600000 //1 hour
#define FLOOD_SIXTH_ALARM 14400000 //4 hours - and every 4 hours ever after, until the situation is rectified (ie no more water is detected)

#define FLOOD_NOTIF "FLOOD"

elapsedMillis flood_timer;
elapsedMillis flood_alarm_timer;

int flood_alarms_array[6]={FLOOD_FIRST_ALARM, FLOOD_SECOND_ALARM, FLOOD_THIRD_ALARM, FLOOD_FOURTH_ALARM, FLOOD_FIFTH_ALARM, FLOOD_SIXTH_ALARM};
int flood_alarm_index = 0;
bool flood_detected = false;
unsigned long flood_next_alarm = 0;

int FLOOD_SENSOR = D0;
int LED = D7;

void setup() {
 pinMode(FLOOD_SENSOR, INPUT_PULLUP);
 pinMode(LED, OUTPUT);
 Spark.publish("device starting", "Firmware version: " + _version, 60, PRIVATE);
}

void loop() {

    flood_check();
    
    if ( flood_detected ) {
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
    if (flood_timer < FLOOD_READ_INTERVAL) {
        return 0;
    }
    
    //time is up, so reset timer
    flood_timer = 0;

    if (not digitalRead(FLOOD_SENSOR)) {
        
        //if flood is already detected, no need to do anything, since an alarm will be fired
        if (flood_detected){
            return 0;
        }
        
        flood_detected = true;
    
        //reset alarm timer
        flood_alarm_timer = 0;

        //set next alarm
        flood_alarm_index = 0;
        flood_next_alarm = flood_alarms_array[0];
        
        digitalWrite(LED,HIGH);
    } else {
        
        digitalWrite(LED,LOW);
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

    if (flood_alarm_timer < flood_next_alarm) {
        return 0;
    }

    
    //time is up, so reset timer
    flood_alarm_timer = 0;
    
    //set next alarm or just keep current one if there are no more alarms to set
    if (flood_alarm_index < arraySize(flood_alarms_array)-1) {
        flood_alarm_index = flood_alarm_index + 1;
        flood_next_alarm = flood_alarms_array[flood_alarm_index];
    }
    Particle.publish("twilioCall", "A Flood Detected!!!", 60, PRIVATE);
    Particle.publish("waterDetected", "A Flood Detected!!!", 60, PRIVATE);

    //send an alarm to user (this one goes to the dashboard)
    Spark.publish(FLOOD_NOTIF, "Flood detected!", 60, PRIVATE);
 // twilioCall
    //send an alarm to user (this one goes to pushbullet servers)
    Spark.publish("waterDetected", "Flood detected!", 60, PRIVATE);
    
   
   return 0; 
}
