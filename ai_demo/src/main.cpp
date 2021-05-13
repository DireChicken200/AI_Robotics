/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       james                                                     */
/*    Created:      Mon Aug 31 2020                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;

// A global instance of competition
competition Competition;

// create instance of jetson class to receive location and other
// data from the Jetson nano
//
ai::jetson  jetson_comms;

/*----------------------------------------------------------------------------*/
// Create a robot_link on PORT1 using the unique name robot_32456_1
// The unique name should probably incorporate the team number
// and be at least 12 characters so as to generate a good hash
//
// The Demo is symetrical, we send the same data and display the same status on both
// manager and worker robots
// Comment out the following definition to build for the worker robot
#define  MANAGER_ROBOT    1

#if defined(MANAGER_ROBOT)
#pragma message("building for the manager")
ai::robot_link       link( PORT11, "robot_32456_1", linkType::manager );
#else
#pragma message("building for the worker")
ai::robot_link       link( PORT11, "robot_32456_1", linkType::worker );
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                          Auto_Isolation Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous isolation  */
/*  phase of a VEX AI Competition.                                           */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void auto_Isolation(void) {
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                        Auto_Interaction Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous interaction*/
/*  phase of a VEX AI Competition.                                           */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/


void auto_Interaction(void) {
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                          AutonomousMain Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

bool firstAutoFlag = true;

void autonomousMain(void) {
  // ..........................................................................
  // The first time we enter this function we will launch our Isolation routine
  // When the field goes disabled after the isolation period this task will die
  // When the field goes enabled for the second time this task will start again
  // and we will enter the interaction period. 
  // ..........................................................................

  if(firstAutoFlag)
    auto_Isolation();
  else 
    auto_Interaction();

  firstAutoFlag = false;
}


/*----------------------------------------------------------------------------*/
int IntakeX = 184, IntakeY = 221;
int8_t targetID = 1;
int32_t deadzone = 20;
int32_t deadzoneY = 80;

void useIntake(MAP_RECORD& lm)
{
  for(int i = 0; i < lm.boxnum; i++)
  {
    if(lm.boxobj[i].x < IntakeX + deadzone/2.0 && lm.boxobj[i].x > IntakeX - deadzone/2.0)
    {
      mainDrive.driveUntil(100, 20);
      if(lm.boxobj[i].y < IntakeY + deadzone/2.0 && lm.boxobj[i].y > IntakeY - deadzoneY)
      {
        if(lm.boxobj[i].classID == targetID)
        {
          mainDrive.getModule("Intake")->runUntil(100, 5.0);
        } else 
        {
          mainDrive.getModule("Intake")->runUntil(-100, 5.0);
        }
      } 
      mainDrive.waitUntilComplete(); 
    }
  }
}

void center(int x, int y)
{
  mainDrive.getModule("Intake")->runForwardAt(50);
  mainDrive.getModule("Intake")->startAllMotors();
   if(x > IntakeX)
    {
      mainDrive.turnRightBy(50 * fabs((float)(IntakeX - x))/(float)(IntakeX));
    } 
    if(x < IntakeX)
    {
      mainDrive.turnLeftBy(50 * fabs((float)(IntakeX - x))/(float)(IntakeX)) ;
    } 

    if(y > IntakeY) 
    {
      mainDrive.strafeForwardBy(50); // this might need to be removed
    }

    mainDrive.startAllMotors(true);
}
void idle( void )
{
  mainDrive.getModule("Intake")->stopAllMotors();

  mainDrive.turnRightAt(50);
  mainDrive.startAllMotors(false);
}

int main() {
    // Initializing Robot Configuration. DO NOT REMOVE!
    vexcodeInit();

    // local storage for latest data from the Jetson Nano
    static MAP_RECORD       local_map;

    // RUn at about 15Hz
    int32_t loop_time = 66;
    float turnAngle;
    float rVal;
    bool needAngle = true;
    bool atAngle = false;

    // start the status update display
    thread t1(dashboardTask);

    // Set up callbacks for autonomous and driver control periods.
    Competition.autonomous(autonomousMain);

    // print through the controller to the terminal (vexos 1.0.12 is needed)
    // As USB is tied up with Jetson communications we cannot use
    // printf for debug.  If the controller is connected
    // then this can be used as a direct connection to USB on the controller
    // when using VEXcode.
    //
    //FILE *fp = fopen("/dev/serial2","wb");

    while(1) {
        // get last map data
        jetson_comms.get_data( &local_map );

        // set our location to be sent to partner robot
        link.set_remote_location( local_map.pos.x, local_map.pos.y, local_map.pos.az );

        //fprintf(fp, "%.2f %.2f %.2f\n", local_map.pos.x, local_map.pos.y, local_map.pos.az  );
        useIntake(local_map);
        // Rework this into using box objects: take object closest to the center of the camera and turn until it is in the center of the camera
        // Then move forward until it is inside the intakes
        // Then 'store' the ball
        int previousDifference = 1000;
        bool found = false;
        fifo_object_box tempTracking;
        for(int i = 0; i < local_map.boxnum; i++)
        {
          if(local_map.boxobj[i].classID == targetID)
          {
            if(abs(IntakeX - local_map.boxobj[i].x) < previousDifference) 
            {
              previousDifference = abs(IntakeX - local_map.boxobj[i].x);
              tempTracking = local_map.boxobj[i];
              found = true;
            }
          }
        }

        if(found)
        {
          center(tempTracking.x, tempTracking.y);
        } else 
        {
          idle();
        }



        // request new data    
        // NOTE: This request should only happen in a single task.    
        jetson_comms.request_map();

        // Allow other tasks to run
        this_thread::sleep_for(loop_time);
    }
}