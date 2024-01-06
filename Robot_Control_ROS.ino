/*
  Currently supported topics:
  cmd_vel
  msg_compass
  msg_irarray (using rosserial_arduino Adc message for 5*uint16)
*/


#include <ArduinoRobotLite.h>

#define USE_USBCON  //serial via USB
#include <ros.h>
#include <rosserial_arduino/Adc.h>
#include <std_msgs/Int16.h>
#include <std_msgs/Float32.h>
#include <geometry_msgs/Twist.h>


#define LOOP_TIME 50000  // loop time in us. 50ms = 20Hz
const double L_BASE = 0.15; //distance between the wheels in m

unsigned long loopTimer;
int cmdVelLeft = 0;
int cmdVelRight = 0;

ros::NodeHandle nh; //instantiate the node handle for ROS

void cmdVelCallBack(const geometry_msgs::Twist& twist) {
  cmdVelLeft = round( (double)twist.linear.x ); //testing
  //cmdVelLeft = round( (double)twist.linear.x - (double)twist.angular.z * L_BASE );
  cmdVelRight = round( (double)twist.linear.x ); //testing
  //cmdVelRight = round( (double)twist.linear.x + (double)twist.angular.z * L_BASE );

  if(cmdVelLeft > 255) { cmdVelLeft = 255; }
  else if (cmdVelLeft < -255) { cmdVelLeft = -255; }

  if(cmdVelRight > 255) { cmdVelRight = 255; }
  else if (cmdVelRight <-255) { cmdVelRight = -255; }
}

ros::Subscriber<geometry_msgs::Twist> subCmdVel("cmd_vel", cmdVelCallBack);

std_msgs::Int16 heading_msg;
ros::Publisher pubCompassHeading("msg_compass", &heading_msg);

rosserial_arduino::Adc IRarray_msg;
ros::Publisher pubIRarray("msg_irarray", &IRarray_msg);




void setup() {
  nh.initNode();
  nh.subscribe(subCmdVel);
  nh.advertise(pubCompassHeading);
  nh.advertise(pubIRarray);

  Robot.begin();
  Robot.setMode(MODE_SIMPLE);

  loopTimer = micros() + LOOP_TIME; //Set the loopTimer variable for the next loop-end time.
}


void loop() {
  //Robot.motorsWrite(0, 0);      // slow stop
  //Robot.motorsStop();           // fast stop
  Robot.motorsWrite(cmdVelLeft, cmdVelRight);
  
  heading_msg.data = Robot.compassRead();
  pubCompassHeading.publish( &heading_msg );

  Robot.updateIR();
  IRarray_msg.adc0 = Robot.IRarray[0];
  IRarray_msg.adc1 = Robot.IRarray[1];
  IRarray_msg.adc2 = Robot.IRarray[2];
  IRarray_msg.adc3 = Robot.IRarray[3];
  IRarray_msg.adc4 = Robot.IRarray[4];
  IRarray_msg.adc5 = Robot.IRarray[5]; 
  pubIRarray.publish(&IRarray_msg);

  nh.spinOnce();
  // Wait for the remaining time in the loop and set the new loop-end time.
  while(loopTimer > micros()){;}
    loopTimer += LOOP_TIME;
}