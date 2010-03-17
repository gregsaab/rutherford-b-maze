#pragma config(Sensor, S1,     touchSensor,         sensorTouch)
#pragma config(Sensor, S4,     sonarCensor,         sensorSONAR)
#pragma config(Motor,  motorA,          rightMotor,    tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorB,          turretMotor,   tmotorNormal, PIDControl, encoder)
#pragma config(Motor,  motorC,          leftMotor,     tmotorNormal, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/**********************************************************************************
* File:         main.c
* Authors:      Greg Nehus, Matt Odille
* Description:  This program is written in ROBOTC for the Lego Mindstorms NXT
*               platform.  It's an autonomous maze-solving robot that works in a
*               grid-style maze with any degree turns.  Given only the start and
*               destination coordinates as (X,Y) values, the robot will find a path
*               to the destination cell, then travel back and forth between the
*               destination and starting point indefinitely.  The robot should be
*               facing "north" originally and modifications may be required to
*               certain macros based on the size of cells and the build of the robot.
* Last modified: Mar 11, 2010
*********************************************************************************/

/**********************************************************************************
* Preprocessor Definitions
*********************************************************************************/
// Maze information (units=cells)
#define MAZE_CELL_TO_CELL 16
#define MIN_DIST 32
#define ROOM_X_CM  675
#define ROOM_Y_CM  620
#define MAZE_X_CM   480
#define MAZE_Y_CM   400
#define MAZE_HEIGHT MAZE_Y_CM/MAZE_CELL_TO_CELL
#define MAZE_WIDTH  MAZE_X_CM/MAZE_CELL_TO_CELL
#define MAZE_ORIGIN_X 0
#define MAZE_ORIGIN_Y 0

#define WALL_DISTANCE_THRESHOLD MAZE_CELL_TO_CELL

// Maze information (units=centimeters)
#define dMAZE_GOAL_X 110
#define dMAZE_GOAL_Y 100

// LCD Information
#define PIXELS_X 100
#define PIXELS_Y 64
#define CELL_WALL_PIXEL_WIDTH 1
#define CELL_PIXEL_WIDTH PIXELS_X  / MAZE_WIDTH
#define CELL_PIXEL_HEIGHT PIXELS_Y / MAZE_HEIGHT

// Robot Information
#define WHEEL_DIAMETER 5.5
#define DISTANCE_FROM_SONAR_TO_CENTER 10

// Defines for motor movement timing
#define DURATION_TURN_90 720
#define DURATION_LOOK_90 400
#define DURATION_DASH_CELL 3500


#include "main.h"

/**********************************************************************************
* Global Variables
*********************************************************************************/
walls direction_of_travel;                      // This keeps track of the direction that the bot is travelling

directions turret_angle = dSouth;               // Variable to hold the direction that the sonar turret is facing
                                                // Initialized to south so it's forced far to left at startup to properly align against alignment setup

directions base_angle = dNorth;                 // Variable to store the direction that the robot base is facing

directions dir_lookup[] =                       // Array serves as easy way to get direction value of wall, for angle references
      {0,dWest,dNorth,0,dEast,0,0,0,dSouth};

walls wall_lookup[] =                           // Array serves as way to have access to all enumerated wall types
    {0, west, north, 0, east, 0, 0, 0, south};

walls wall_lookup_west[] =                           // Array serves as way to have access to all enumerated wall types
    {0, south, west, 0, north, 0, 0, 0, east};

walls wall_lookup_north[] =                           // Array serves as way to have access to all enumerated wall types
    {0, west, north, 0, east, 0, 0, 0, south};

walls wall_lookup_east[] =                           // Array serves as way to have access to all enumerated wall types
    {0, north, east, 0, south, 0, 0, 0, west};

walls wall_lookup_south[] =                           // Array serves as way to have access to all enumerated wall types
    {0, east, south, 0, west, 0, 0, 0, north};


walls opp_wall_lookup[]=                        // Array gives easy way to get value of opposing wall direction
      {0,east, south, 0,west, 0, 0, 0, north};

cell maze[MAZE_WIDTH][MAZE_HEIGHT];             // Matrix of cells representing the maze

bool hasBumped = false;                         // Boolean variable to set if a wall has been ran into

int MAZE_GOAL_X;
int MAZE_GOAL_Y;

bool useVisited = false;
bool justWon;
bool hasWon;

// convert end goal coords in CM to coords in cells
int sMAZE_GOAL_X = (int)( dMAZE_GOAL_X / MAZE_CELL_TO_CELL );
int sMAZE_GOAL_Y = (int)( dMAZE_GOAL_Y / MAZE_CELL_TO_CELL );


/**********************************************************************************
* Main task
*********************************************************************************/
task main()
{

	  // Setup the motor configuration
	  bFloatDuringInactiveMotorPWM = false;
	  nMotorEncoder[rightMotor] = 0;
	  nMotorEncoder[leftMotor] = 0;

    // Set the start coordinates and align the turret
    curr_position.x = MAZE_ORIGIN_X;
    curr_position.y = MAZE_ORIGIN_Y;
    halt();
    align_turret();
    halt();




	  // Initialize the borders of the maze and draw the map on the LCD screen
    eraseDisplay();
    initialize_maze();
    //display_map();
    draw_destination(sMAZE_GOAL_X, sMAZE_GOAL_Y);
    nVolume = 4;




    // This loops through travelling back and forth between start and destination coordinates
    while(true){
			navigate_to_cell(sMAZE_GOAL_X, sMAZE_GOAL_Y);
			useVisited = true;
			navigate_to_cell(MAZE_ORIGIN_X, MAZE_ORIGIN_Y);
      //break;
    }
}//end main


/********************************************************************************
 * Function: navigate_to_cell
 * Parameters: X,Y coordinates of destination
 * Return: None
 * Description: Has the robot navigate to a given location, given coordinates
 */
void navigate_to_cell(int dest_x, int dest_y){
    MAZE_GOAL_X = dest_x;
    MAZE_GOAL_Y = dest_y;
	// This loops until the bot solves the maze
    while(true){

        nxtSetPixel(curr_position.x * 2 - 1, curr_position.y *2);
	      scan_cell();                                                          // Scan for walls in the current cell

	      maze[curr_position.x][curr_position.y].visited = true;                // Mark current cell as visited
	      draw_cell(curr_position.x,curr_position.y);                           // Draw the cell on the LCD
	      nxtClearPixel(curr_position.x * 2 - 1, curr_position.y *2);
	      direction_of_travel = choose_best_cell();                             // Choose which direction to travel

	      set_base_angle(dir_lookup[direction_of_travel]);                      // Turn base to proper direction


	      dash();                                                               // Dash into neighbor cell

        set_turret_angle(dNorth);
        adjust(SensorValue(sonarCensor) - (MAZE_CELL_TO_CELL / 2 + 8);




	      //nxtDisplayCenteredTextLine(2, "Coord: %d,%d", curr_position.x, curr_position.y);

	      if(curr_position.x == dest_x && curr_position.y == dest_y || hasWon){ // Check for destination
	          //PlaySound(soundUpwardTones);
	          //wait1Msec(250);
    	      StartTask(we_are_the_champions);
    	      hasWon = false;
            //set_base_angle(dir_lookup[opp_wall_lookup[direction_of_travel]]);
            justWon = true;
			  break;


	      }
    }//end while


}


/********************************************************************************
 * Function: initialize_maze
 * Parameters: None
 * Return: None
 * Description: Automatically sets the border of the maze
 */
void initialize_maze(){
    int x, y;
    for (x = 0; x< MAZE_WIDTH; x++){
        for (y = 0; y< MAZE_HEIGHT; y++){
            walls c = 0;  // 0000 = no walls

            if (x == 0) c |= west;                // West border
            if (x == MAZE_WIDTH - 1) c |= east;   // East border
            if (y == 0) c |= south;               // South border
            if (y == MAZE_HEIGHT -1) c |= north;  // North border

            maze[x][y].cell_walls = c;
        }
    }
}

/********************************************************************************
 * Function: display_map
 * Parameters: None
 * Return: None
 * Description: Displays the map on the NXTs LCD
 */
void display_map(){
  int x, y;
  for (x = 0; x< MAZE_WIDTH; x++)
    for (y = 0; y< MAZE_HEIGHT; y++)
      draw_cell(x,y);
}

/********************************************************************************
 * Function: draw_cell
 * Parameters: Takes an (x,y) coordinate representing the cell to draw
 * Return: None
 * Description: Displays the cell on the NXTs LCD by drawing all of its walls as lines
 */
void draw_cell(int x, int y){
    coord origin;
    get_cell_pixel_origin(x, y, origin);
    //if (maze[x][y].cell_walls & west) draw_cell_wall(origin.x,origin.y,west);
    //if (maze[x][y].cell_walls & north) draw_cell_wall(origin.x,origin.y,north);
    //if (maze[x][y].cell_walls & east) draw_cell_wall(origin.x,origin.y,east);
    //if (maze[x][y].cell_walls & south) draw_cell_wall(origin.x,origin.y,south);
    //nxtDisplayStringAt(70,20, "%d,%d", curr_position.x, curr_position.y);
    //wait10Msec(1000);
    //get_cell_pixel_center(x,y,origin);
    if (maze[x][y].visited) nxtSetPixel(x*2, y*2);
}

void draw_destination(int x, int y){
  int originx = x - 2;
  int originy = y - 2;
  x = x  * 2;
  y = y  * 2;

  int w, z;
  for (w = x - 2; w <= x + 2; w++){
    if( w == x-2 || w == x+2 ){
      for( z=y-2; z<=y+2; z++)
        nxtSetPixel(w,z);
    } else {
      nxtSetPixel(w,y-2);
      nxtSetPixel(w,y+2);
    }
  }

}

/********************************************************************************
 * Function: draw_cell_wall
 * Parameters: Takes an (x,y) coordinate and walls object representing the cell to draw
 * Return: None
 * Description: Prints the pixels for a cell's wall on the NXTs LCD
 */
void draw_cell_wall(int x, int y, walls dir){
    int width, height;
    int sX, sY;
    sX = x;
    sY = y;

    if (dir == north || dir == south){
        width = CELL_PIXEL_WIDTH;
        height = CELL_WALL_PIXEL_WIDTH;
        if (dir == north) sY = sY + CELL_PIXEL_HEIGHT;
    }else{
        width = CELL_WALL_PIXEL_WIDTH;
        height = CELL_PIXEL_HEIGHT;
        if (dir == west) sX = sX - CELL_PIXEL_WIDTH + CELL_WALL_PIXEL_WIDTH;
    }

    int xD, yD;
    for (xD = sX; xD > sX - width; xD--){
        for (yD = sY; yD < sY + height; yD++)
            nxtSetPixel(xD, yD);
    }
}

/********************************************************************************
 * Function: get_cell_pixel_origin
 * Parameters: Takes a coordinate object to pass back coordinate
 * Return: None
 * Description: Determins the pixel origin of a given cell
 */
void get_cell_pixel_origin(int x, int y, coord *n){
    n->x = CELL_PIXEL_WIDTH  * (x + 1);
    n->y = CELL_PIXEL_HEIGHT * (y);
}

/********************************************************************************
 * Function: get_cell_pixel_center
 * Parameters: Takes a coordinate object to pass back coordinate
 * Return: None
 * Description: Determins the pixel center of a given cell
 */
void get_cell_pixel_center(int x, int y, coord *n){
    n->x = CELL_PIXEL_WIDTH  * (x + 1) - CELL_PIXEL_WIDTH /2;
    n->y = CELL_PIXEL_HEIGHT * y + CELL_PIXEL_HEIGHT /2;
}

/********************************************************************************
 * Function: void halt()
 * Parameters: None
 * Return: None
 * Description: This function stops all motors.
 */
void halt()
{
    motor[rightMotor] = 0;
    motor[leftMotor] = 0;
    motor[turretMotor] = 0;
    wait1Msec(250);
    return;
}

/********************************************************************************
 * Function: set_base_angle
 * Parameters: Takes an enumerated direction name
 * Return: None
 * Description: This function sets the direction that the robot should face
 */
void set_base_angle(directions angle){
    directions r_angle;

    if (angle == base_angle) return;                        // If the angle that is requested is the current angle, quit

    r_angle = (directions) ((int)angle - (int)base_angle);    // Adjust the angle relative to where the base is currently aimed
    if (r_angle == -270) r_angle = 90;
    if (r_angle == 270) r_angle = -90;
    turn_base(abs(r_angle), r_angle/(abs(r_angle)));              // Turn the base
    base_angle = angle;   // Update the angle at which the base is aimed
}

/********************************************************************************
 * Function: set_turret_angle
 * Parameters: Takes an enumerated direction name
 * Return: None
 * Description: This function figures out the angle that the turret needs to turn and direction
 */
void set_turret_angle(directions angle){
    directions r_angle;

    if (angle == turret_angle) return;                          // If requested angle is already set, just leave

    r_angle = (directions) ((int)angle - (int)turret_angle);    // Adjust the angle relative to where the turret is currently aimed
    turn_turret(abs(r_angle), r_angle/(abs(r_angle)), angle);          // Turn the turret
    turret_angle = angle;  // Update the angle at which the base is aimed
}

/********************************************************************************
 * Function: turn_turret
 * Parameters: Takes an absolute (as opposed to relative) angle and direction (negative or positive)
 * Return: None
 * Description: This function turns the turret to a particular direction
 */
void turn_turret(int angle, int direction, directions iAngle)
{
    int fineTune = 0;                                       // Variable used to push motor a little further when that angle has physical stoppers

    halt();                                                 // Stop all motors

    if (iAngle == dWest || iAngle == dSouth) fineTune = 100;         // There are stoppers at 0 & 270, so lets go a little further for both of these angles
    motor[turretMotor] = 25 * direction;                    // Set turret motor speed
    wait1Msec(DURATION_LOOK_90 * (angle / 90) + fineTune);  // Timer delay

    halt();                                                 // Stop all motors
}

/********************************************************************************
 * Function: turn_base
 * Parameters: Takes an absolute (as opposed to relative) angle and direction (negative or positive)
 * Return: None
 * Description: This function turns the base of the robot
 */
void turn_base(int angle, int direction)
{

    halt();                                                 // Stop all motors

    motor[rightMotor] = 30 * (direction * -1);              // Set right motor speed
    motor[leftMotor] = 30 * direction;                      // synced to rightMotor master
    wait1Msec(DURATION_TURN_90 * (angle / 90));             // Set delay

    halt();                                                 // Stop all motors
}


/********************************************************************************
 * Function: dash
 * Parameters: None
 * Return: None
 * Description: This function makes the robot dash forward
 */
void dash()
{
    halt();                                   // Stop all motors

    nMotorEncoder[rightMotor] = 0;            // Reset motor encoder value
		motor[rightMotor] = 60;                   // Set motor speeds
		motor[leftMotor] = 60;

		// Loop until the motor encoder value indicates that the robot has travelled distance of MAZE_CELL_TO_CELL
    while(nMotorEncoder[rightMotor] < MAZE_CELL_TO_CELL / (PI * WHEEL_DIAMETER) * 360)
    {
				if (SensorValue[touchSensor] == 1){ // Monitor the bumper sensor while the bot is moving
						halt();                         // If bumper sensor is engaged, stop all motors
						hasBumped = true;               // Set the hasBumped flag
						break;                          // Exit the while loop
				}
    }

    halt();                                 // Stop all motors

    int hasTravelled = abs(nMotorEncoder[rightMotor]);
    if (hasBumped){                         // If a bump occurred...
        nMotorEncoder[rightMotor] = 0;
        motor[rightMotor] = -25;            // Set the motors to reverse
        motor[leftMotor] = -25;
        coord n;


        if (useVisited) hasTravelled = 0;
        // Loop until the motor encoder value indicates that the robot has travelled 1/2 the distance of MAZE_CELL_TO_CELL
        while(abs(nMotorEncoder[rightMotor]) < hasTravelled);
        get_neighbor_coordinate(curr_position.x, curr_position.y, opp_wall_lookup[direction_of_travel], n);
        if (curr_position.x == sMAZE_GOAL_X && curr_position.y == sMAZE_GOAL_Y) hasWon = true;

        if (!useVisited){
          curr_position.x = n.x;
          curr_position.y = n.y;
          maze[curr_position.x][curr_position.y].cell_walls |= direction_of_travel;
      }else{
        adjust(SensorValue(sonarCensor) - (MAZE_CELL_TO_CELL / 2 + 8);
      }

        halt();                             // Stop all motors
        hasBumped = false;                  // Reset hasBumped flag
    }
}

void dash_wallfollow()
{
    int sonarValue;
    halt();                                   // Stop all motors

    nMotorEncoder[rightMotor] = 0;            // Reset motor encoder value
		motor[rightMotor] = 60;                   // Set motor speeds
		motor[leftMotor] = 60;

		// Loop until the motor encoder value indicates that the robot has travelled distance of MAZE_CELL_TO_CELL
    while(nMotorEncoder[rightMotor] < MAZE_CELL_TO_CELL / (PI * WHEEL_DIAMETER) * 360)
    {
        sonarValue = SensorValue[sonarCensor];
        motor[rightMotor] = 60;
        motor[leftMotor] = 60;
				if (SensorValue[touchSensor] == 1){ // Monitor the bumper sensor while the bot is moving
						halt();                         // If bumper sensor is engaged, stop all motors
						hasBumped = true;               // Set the hasBumped flag
						break;                          // Exit the while loop
				}
    }

    halt();                                 // Stop all motors

    if (hasBumped){                         // If a bump occurred...
        nMotorEncoder[rightMotor] = 0;
        motor[rightMotor] = -50;            // Set the motors to reverse
        motor[leftMotor] = -50;

        // Loop until the motor encoder value indicates that the robot has travelled 1/2 the distance of MAZE_CELL_TO_CELL
        while(nMotorEncoder[rightMotor] > -(MAZE_CELL_TO_CELL / (PI * WHEEL_DIAMETER)) / 2* 360){
        }

        halt();                             // Stop all motors
        hasBumped = false;                  // Reset hasBumped flag
    }
}


void adjust(float distance){


    halt();                                               // Stop all motors

    if (abs(distance) == 0 || abs(distance) > MAZE_CELL_TO_CELL / 1.5 ) return;
    nMotorEncoder[rightMotor] = 0;                        // Reset motor encoder value
		motor[rightMotor] = 10 * (distance / abs(distance));                   // Set motor speeds
		motor[leftMotor] = 10* (distance / abs(distance));

		// Loop until the motor encoder value indicates that the robot has travelled distance of MAZE_CELL_TO_CELL
		int shouldTravel = distance / (PI * WHEEL_DIAMETER) * 360;
    while(true){
      if (distance < 0 && nMotorEncoder[rightMotor] < shouldTravel ) break;
      if (distance > 0 && nMotorEncoder[rightMotor] > shouldTravel) break;
    };

    halt();                                 // Stop all motors
}
/********************************************************************************
 * Function: align_turret
 * Parameters: None
 * Return: None
 * Description: Function turns the turret all the way to the left to make sure it is aligned
 */
void align_turret(){
    set_turret_angle(dWest);
}

/********************************************************************************
 * Function: scan_walls
 * Parameters: Enumerated wall enum
 * Return: None
 * Description: Function scans a wall in a particular direction
 */
void scan_wall(walls w, walls c)
{
    coord n, h;    //Variable to hold adjacent cell if wall is found

    get_neighbor_coordinate(curr_position.x, curr_position.y, w, h);

    if (maze[curr_position.x][curr_position.y].cell_walls & w || maze[h.x][h.y].visited) return;  // If there is already a wall here, don't scan it

    set_turret_angle(dir_lookup[(int) c]); //Turn to the particular wall

    // If there is an object that has a distance less than the WALL_DISTANCE_THRESHOLD value
    if (SensorValue(sonarCensor) < WALL_DISTANCE_THRESHOLD){
            maze[curr_position.x][curr_position.y].cell_walls |= w;             // Mark this as a wall

            // If there is a neighboring cell
            if (get_neighbor_coordinate(curr_position.x, curr_position.y, w, n))
                maze[n.x][n.y].cell_walls |= (walls) opp_wall_lookup[(int) w];  //Mark the opposing wall as a wall



     }
}

/********************************************************************************
 * Function: scan_cell
 * Parameters: None
 * Return: None
 * Description: Function a cell in all wall directions
 */
void scan_cell()
{
    int x;
    if (maze[curr_position.x][curr_position.y].visited == true) return;
    for (x = 1; x < 9; x = x * 2){                  // This for loop will iterate through wall_lookup array (1, 2, 4, 8)

			if (base_angle == dNorth) scan_wall(wall_lookup_north[x], wall_lookup[x]);
			if (base_angle == dEast) scan_wall(wall_lookup_east[x], wall_lookup[x]);
			if (base_angle == dSouth) scan_wall(wall_lookup_south[x], wall_lookup[x]);
			if (base_angle == dWest) scan_wall(wall_lookup_west[x], wall_lookup[x]);
		}

}

/********************************************************************************
 * Function: choose_best_cell
 * Parameters: None
 * Return: None
 * Description: Function determines which direction that the robot should move in. If there is a best
 *              route that facilitates an unvisited adjacent cell, the robot will travel there, however,
 *              if there is no open cell that is unvisited, the robot will go back to the cell that it had come
 *              from.
 */
walls choose_best_cell()
{
    int x;                                                                      // For loop variable
    int bX, bY;                                                                 // Best coordinates for non-visited route
    int bvX, bvY;                                                               // Best coordinates for visited route

    float shortestUnvisitedDistance = 65535;                                    // Variable to hold best univisted distance
    float shortestVisitedDistance = 65535;                                      // Variable to hold best visited distance

    walls dir, vdir;                                                            // Hold directions of direction of best visited and unvisted cells

    unsigned char mask = 1;                                                     // Bit mask to scan through walls enum

    for (x = 0; x<4; x++)
    {
        if (mask & ~(maze[curr_position.x][curr_position.y].cell_walls))        // If there is no wall at given direction
        {
            coord n,h;                                                            // Variable to hold proposed next cell


            if (!get_neighbor_coordinate(curr_position.x, curr_position.y, (walls)mask, n)) continue;   // If neighbor coordinate does not exist, skip
            memcpy(h,n,sizeof(h));
            // (get_neighbor_coordinate(n.x, n.y, (walls)mask, h) &&  mask & maze[h.x][h.x].cell_walls) continue;

            if (!useVisited){
	            // If proposed next cell is the closest proposed unvisted cell yet, remember it
	            if (get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y) < shortestUnvisitedDistance && !maze[n.x][n.y].visited){
	                shortestUnvisitedDistance = get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y);   // Save cartesian distance
	                bX = n.x;                                                                               // Save best unvisted x-coord
	                bY = n.y;                                                                               // Save best unvisted y-coord
	                dir = (walls)mask;                                                                      // Save relative unvistited next cell direction

	            // Else, if the proposed next cell is the closest cell that has already been visited, remember it
	            }else if (get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y) < shortestVisitedDistance && maze[n.x][n.y].visited){
	                shortestVisitedDistance = get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y);      // Save teh cartesian distance
	                bvX = n.x;                                                                               // Save the best visited x-coord
	                bvY = n.y;                                                                               // Save teh best visited y-coord
	                vdir = (walls)mask;                                                                      // Save relative visited next cell direction
              }
          }else{
              if (get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y) < shortestVisitedDistance && maze[n.x][n.y].visited && (opp_wall_lookup[(int)direction_of_travel] != (walls)mask ^ justWon)){
                shortestVisitedDistance = get_distance(n.x, n.y, MAZE_GOAL_X, MAZE_GOAL_Y);      // Save teh cartesian distance
                bvX = n.x;                                                                               // Save the best visited x-coord
                bvY = n.y;                                                                               // Save teh best visited y-coord
                vdir = (walls)mask;
                justWon = false;
            }
          }
        }
        mask = mask << 1;                                                                                 // Shift mask to left
    }

    if (shortestUnvisitedDistance == 65535 || useVisited){                                    // If there is no new cell that has not been visited
        PlaySound(soundBlip);                                                   // Sound playing (for debug purposes)
        wait1Msec(250);
        PlaySound(soundBlip);

        curr_position.x = bvX;                                                  // Set the current position to the best visited cell x-coord
        curr_position.y = bvY;                                                  // Set the current position to the best visited cell y-coord

        if (!useVisited) maze[curr_position.x][curr_position.y].cell_walls |= (walls) opp_wall_lookup[(int) vdir];
                        // Close off the dead end by putting a virtual wall

        return vdir;                                                            // Return the visited direction
    }else{                                                                      // Else (there is a new cell to be travelled to)
        PlaySound(soundBlip);                                                   // Play sound for debug purposes

        curr_position.x = bX;                                                   // Set the current position to the best unvisited cell x-coord
        curr_position.y = bY;                                                   // Set the current position to the best unvisited cell y-coord

        return dir;                                                             // Return the new cell direction
    }
}

/********************************************************************************
 * Function: get_distance
 * Parameters: Takes integer values of (x1,y1) and (x2, y2)
 * Return: Distance as a float
 * Description: Function that determines the cartesian distance between two points
 */
float get_distance(int x1, int y1, int x2, int y2){
    return sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/********************************************************************************
 * Function: get_neighbor_coordinate
 * Parameters: Takes the x,y corrdinate of current position, the proposed direction and a coord object to save the coordinate into
 * Return: True or false depending on whether proposed cell is valid or not
 * Description: Function determines which direction that the robot should move in. If there is a best
 *              route that facilitates an unvisited adjacent cell, the robot will travel there, however,
 *              if there is no open cell that is unvisited, the robot will go back to the cell that it had come
 *              from.
 */
bool get_neighbor_coordinate(int x, int y, walls dir, coord *n){

    // Figure out the coordinate based on the current position and the desired direction
		if (dir == west){
				n->x = x - 1;
				n->y = y;
		}else if(dir == north){
				n->x = x;
				n->y = y + 1;
		}else if(dir == east){
				n->x = x + 1;
				n->y = y;
		}else if(dir == south){
				n->x = x;
				n->y = y -1;
		}

		if (n->x < 0 || n->y < 0 || n->x >= MAZE_WIDTH  || n->y >= MAZE_HEIGHT) return false;   // if the cell is not valid, return false
		return true;                                                                            // else return true as it is valid
}

task we_are_the_champions()
{
  //        100 = Tempo
  //          6 = Default octave
  //    Quarter = Default note length
  //        10% = Break between notes
  //
  PlayTone( 1047,  108); wait1Msec(1200);  // Note(F, Duration(Half))
  PlayTone(  988,   27); wait1Msec( 300);  // Note(E, Duration(Eighth))
  StopTask(we_are_the_champions);
  PlayTone( 1047,   27); wait1Msec( 300);  // Note(F, Duration(Eighth))
  PlayTone(  988,   54); wait1Msec( 600);  // Note(E)
  PlayTone(  784,   54); wait1Msec( 600);  // Note(C)
  PlayTone(    0,   27); wait1Msec( 300);  // Note(Rest, Duration(Eighth))
  PlayTone(  880,   27); wait1Msec( 300);  // Note(A5, Duration(Eighth))
  PlayTone(  880,   54); wait1Msec( 600);  // Note(D)
  PlayTone(  880,   81); wait1Msec( 900);  // Note(A5, Duration(Quarter .))
  PlayTone(    0,  108); wait1Msec(1200);  // Note(Rest, Duration(Half))
  PlayTone(  784,   27); wait1Msec( 300);  // Note(C, Duration(Eighth))
  PlayTone( 1047,  108); wait1Msec(1200);  // Note(F, Duration(Half))
  PlayTone( 1175,   27); wait1Msec( 300);  // Note(G, Duration(Eighth))
  PlayTone( 1320,   27); wait1Msec( 300);  // Note(A, Duration(Eighth))
  PlayTone( 1046,   54); wait1Msec( 600);  // Note(C7)
  PlayTone( 1320,   81); wait1Msec( 900);  // Note(A, Duration(Quarter .))
  PlayTone(  880,   27); wait1Msec( 300);  // Note(D, Duration(Eighth))
  PlayTone(  988,   27); wait1Msec( 300);  // Note(E, Duration(Eighth))
  PlayTone(  880,  108); wait1Msec(1200);  // Note(D, Duration(Half))
  PlayTone(    0,  108); wait1Msec(1200);  // Note(Rest, Duration(Half))
  PlayTone(  880,   81); wait1Msec( 900);  // Note(D, Duration(Quarter .))
  PlayTone(  784,   54); wait1Msec( 600);  // Note(C)
  PlayTone(  880,   27); wait1Msec( 300);  // Note(D, Duration(Eighth))
  PlayTone(  784,   81); wait1Msec( 900);  // Note(C, Duration(Quarter .))
  PlayTone(  932,   54); wait1Msec( 600);  // Note(A#5)
  PlayTone(    0,   27); wait1Msec( 300);  // Note(Rest, Duration(Eighth))
  PlayTone( 1398,   81); wait1Msec( 900);  // Note(A#, Duration(Quarter .))
  PlayTone( 1320,   54); wait1Msec( 600);  // Note(A)
  PlayTone( 1398,   27); wait1Msec( 300);  // Note(A#, Duration(Eighth))
  PlayTone( 1320,   81); wait1Msec( 900);  // Note(A, Duration(Quarter .))
  PlayTone( 1175,   54); wait1Msec( 600);  // Note(G)
  PlayTone(    0,   27); wait1Msec( 300);  // Note(Rest, Duration(Eighth))
  PlayTone( 1320,   81); wait1Msec( 900);  // Note(A, Duration(Quarter .))
  PlayTone( 1047,   54); wait1Msec( 600);  // Note(F)
  PlayTone( 1398,   27); wait1Msec( 300);  // Note(A#, Duration(Eighth))
  PlayTone( 1320,   81); wait1Msec( 900);  // Note(A, Duration(Quarter .))
  PlayTone( 1047,   27); wait1Msec( 300);  // Note(F, Duration(Eighth))
  PlayTone(    0,   27); wait1Msec( 300);  // Note(Rest, Duration(Eighth))
  PlayTone( 1398,   27); wait1Msec( 300);  // Note(A#, Duration(Eighth))
  PlayTone( 1245,   81); wait1Msec( 900);  // Note(G#, Duration(Quarter .))
  PlayTone( 1047,   54); wait1Msec( 600);  // Note(F)
  PlayTone( 1398,   27); wait1Msec( 300);  // Note(A#, Duration(Eighth))
  PlayTone( 1245,   81); wait1Msec( 900);  // Note(G#, Duration(Quarter .))
  PlayTone( 1047,   54); wait1Msec( 600);  // Note(F)
}


/********************************************************************************
 * Function: get_sonar
 * Parameters: None
 * Return: Integer value of sonar reading
 * Description: Function reads the sonar and returns the reading
 */
/*!*!* UNUSED
int get_sonar()
{
    int sonarValue = SensorValue[S4]; // Store Sonar Sensor values in 'sonarValue' variable.

    //nxtDisplayCenteredTextLine(0, "Sonar Reading");
    //nxtDisplayCenteredBigTextLine(2, "%d", sonar);

    return sonarValue;
}
UNUSED !*!*/
