#include <khepera/khepera.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <math.h>
#include <ifaddrs.h>



static knet_dev_t * dsPic;
static int quitReq = 0; // quit variable for loop
int feedback_frequency = 10;


/*--------------------------------------------------------------------*/
/* Make sure the program terminate properly on a ctrl-c */
static void ctrlc_handler( int sig ) 
{
  quitReq = 1;
}

static void pkill_handler( int sig ) 
{
  quitReq = 1;
}

/*------------------- Time Value Difference -----------*/
/* Compute time difference

 * \param difference difference between the two times, in structure timeval type
 * \param end_time end time
 * \param start_time start time
 *
 * \return difference between the two times in [us] */
long long timeval_diff(struct timeval *difference, struct timeval *end_time, struct timeval *start_time)
{
	// timeval is a time structure that is commonly used in low level c
	struct timeval temp_diff;

	if(difference == NULL) {
		difference =& temp_diff;
	}

	difference -> tv_sec  = end_time -> tv_sec  - start_time -> tv_sec ;
	difference -> tv_usec = end_time -> tv_usec - start_time -> tv_usec;

	/* Using while instead of if below makes the code slightly more robust. */

	while(difference -> tv_usec < 0) {
		difference -> tv_usec += 1000000;
    	difference -> tv_sec  -= 1;
	}

	return 1000000LL * difference -> tv_sec + difference -> tv_usec;
}



/*--------velocity to pulse-------*/
int v2p(double v) {
	return (int)v / 0.678181;
}

int getSign(double x) {

	return x<0 ? -1 : 1;
}




void display_battery_status(knet_dev_t *hDev){
    char bat_buffer[100];
    kh4_battery_status(bat_buffer,dsPic);
    int battery_charge = bat_buffer[3];
    if(battery_charge > 75){
        // Green
        kh4_SetRGBLeds(
            0x00, 0x08, 0x00,
            0x00, 0x08, 0x00,
            0x00, 0x08, 0x00, hDev);
    }else if(battery_charge > 50){
        // Yellow
        kh4_SetRGBLeds(
            0x08, 0x08, 0x00,
            0x08, 0x08, 0x00,
            0x08, 0x08, 0x00, hDev);
    }else if(battery_charge > 25){
        // Orange
        kh4_SetRGBLeds(
            0x14, 0x04, 0x00,
            0x14, 0x04, 0x00,
            0x14, 0x04, 0x00, hDev);
    }else{
        // Red
        kh4_SetRGBLeds(
            0x20, 0x00, 0x00,
            0x20, 0x00, 0x00,
            0x20, 0x00, 0x00, hDev);
    }
}




/*----------------Main Program-----------------*/
#define FOR_SPD 1000
#define SPIN_SPD 150
#define FOR_DEV_SPD 850

int main(int argc, char *argv[]) {
	int i;
	long int main_loop_delay = 100000;

	/* Initial Template Setup by LinKhepera */
	int rc;

	/* Set the libkhepera debug level - Highly recommended for development. */
	kb_set_debug_level(2);

    /* Init the khepera library */
	if((rc = kb_init( argc , argv )) < 0 )
		return 1;

	/* Main Code */
  
  	// dsPIC is the microcontroller of khepera
  	// It handles all the inputs and outputs
  	dsPic  = knet_open( "Khepera4:dsPic" , KNET_BUS_I2C , 0 , NULL );

    // Blue LED for booting
    kh4_SetRGBLeds(
        0x00, 0x00, 0x08,
        0x00, 0x00, 0x08,
        0x00, 0x00, 0x08, dsPic);

  	// This is for the ctrl-C handler
  	signal( SIGINT , ctrlc_handler );
    // To handle PKILL
    signal( SIGTERM , pkill_handler );

  	// Setting the term mode to 1 will return the pressed key immediately!
  	kb_change_term_mode(1);

    // Variables for time stamps
    struct timeval cur_time, old_time;
    long long elapsed_time_us;

    // Get the starting time stamp
    gettimeofday(&cur_time,0x0);
    old_time = cur_time;

    // For blinking LED
    char led_cnt = 0;

    while(quitReq == 0) {
        
		// Update time
		gettimeofday(&cur_time,0x0);
		elapsed_time_us = timeval_diff(NULL, &cur_time, &old_time);


		if(elapsed_time_us > main_loop_delay){
            led_cnt++;
            if(led_cnt > feedback_frequency){
                led_cnt = 0;
                // Turn LED off to cause blinking
                kh4_SetRGBLeds(
                    0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, dsPic);
            }
            old_time = cur_time;
            
            //----------------- All sensor readings ------------------//
    		// Receive accelerometer readings
    		// getAcc(acc_Buffer, &acc_X, &acc_Y, &acc_Z);

    		// Receive ultrasonic sensor readings
    		// getUS(us_Buffer, usValues);
    		
    		// Receive infrared sensor readings
    		// getIR(ir_Buffer, irValues);
    		
    		// Receive gyroscope readings
    		// getGyro(gyro_Buffer, &gyro_X, &gyro_Y, &gyro_Z);
    		
    		// Receive encoder readings
    		// getEC(&posL, &posR);
    		
    		// Receive encoder speed readings
    		// getSPD(&spdL, &spdR);

            // Receive LRF readings if available
            // if(!(LRF_DeviceHandle < 0))
            //     getLRF(LRF_DeviceHandle, LRF_Buffer);
            // else
            //     memset(LRF_Buffer, 0, sizeof(long)*LRF_DATA_NB);

    		//TCPsendSensor(new_socket, T, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z, posL, posR, spdL, spdR, usValues, irValues);
    		//UDPsendSensor(UDP_sockfd, servaddr, 0, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z, posL, posR, spdL, spdR, usValues, irValues, LRF_Buffer);
    		//printf("Sleeping...\n");

            // Display battery status
            display_battery_status(dsPic);

		}
  	}	

    // Red when not doing anything
    kh4_SetRGBLeds(
        0x08, 0x00, 0x00,
        0x08, 0x00, 0x00,
        0x08, 0x00, 0x00, dsPic);

  	// switch to normal key input mode
  	// This is important, if we don't switch the term mode back to zero
  	// It will still return the pressed key immediately
  	// even at the root@r1:~/tests#
  	// resulting in no characters showing up even if you press any keys on keyboard
  	kb_change_term_mode(0);

  	// set to regular idle mode!
  	kh4_SetMode(kh4RegIdle, dsPic);



 	return 0;  
}