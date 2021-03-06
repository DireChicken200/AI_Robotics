#include "DriveConfigureations.h"
using namespace vex;

extern brain Brain;

extern bancroft::XDrive mainDrive;

struct point
{
  float x, y;
};

extern point towerPosition[3][3];
extern float towerHeadings[3][3];

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 *
 * This should be called at the start of your int main function.
 */
void vexcodeInit(void);
