//c++
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <chrono>
//opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
//C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "json.hpp"

#define TRIGGER BLUE_GP0_PIN_3

using namespace std;
using namespace cv;
using json = nlohmann::json;

enum MOTORS	
{
    LEFT = 1,
    RIGHT = 2
};

enum MODES
{
	UserControl = 0,
	RecordPath = 1,
	ExecutePath = 2,
};

static void MotorController(int* data, int* mode);
static void UltrasonicSensor(float* distance, int* mode);
static void MotionDetector(bool* isMotion);
static void DetectMotion(Mat img1, Mat img2, bool* isMotion);
static void CreateMap(int* data, int* mode);
static void SocketServer(int* drivingCommand, float* ultrasonicData, int* mode, double* batteryVoltage);
static void AdjustMotorPower(float &RIGHT_DUTY, float &LEFT_DUTY);

extern "C" {
        #include <roboticscape.h>
        #include <rc_usefulincludes.h> 
        int rc_initialize();
        int rc_cleanup();
        int rc_set_cpu_freq(rc_cpu_freq_t freq); 
        void rc_usleep(unsigned int us);
        uint64_t rc_nanos_since_boot();
        float rc_battery_voltage();
        int rc_get_encoder_pos(int ch);
        int rc_set_encoder_pos(int ch, int value);
        int rc_enable_motors();
        int rc_disable_motors();
        int rc_set_motor(int motor, float FORWARD);
        int rc_gpio_export(unsigned int gpio);
        int rc_gpio_set_dir(int gpio, rc_pin_direction_t dir);
        int rc_gpio_set_value(unsigned int gpio, int value);
        int rc_gpio_get_value(unsigned int gpio);
    }
    
int main()
{
	//declare pointers and run initilizations
    rc_initialize();
    int* drivingCommand = new int;
    float* ultrasonicData = new float;
    int* mode = new int;
    double* batteryVoltage = new double;
    bool* isMotion = new bool;
    *drivingCommand = 0;
    *ultrasonicData = 0;
    *mode = UserControl;
    *batteryVoltage = 0;
    *isMotion = false;
    rc_set_cpu_freq(FREQ_1000MHZ);
    rc_gpio_export(TRIGGER);
    thread socketServer(SocketServer, drivingCommand, ultrasonicData, mode, batteryVoltage);
    //loop to select which mode the program is in
    while(rc_get_state() != EXITING)
    {
    	switch(*mode)
    	{
    		/* 
    		user is driving the robot from the mobile application
    		need to stream video to server and run the ultrasonic sensor and motor driver
    		*/
    		case UserControl:
    		{
    			thread motorThread(MotorController, drivingCommand, mode);
		    	//thread ultrasonicThread(UltrasonicSensor, ultrasonicData, mode);
		    	//stream video to server
		    	//ultrasonicThread.join();
		    	motorThread.join();
		    	break;
    		}
    		
    		/*
    		user moves when the robot says to, and stops when the robot says to stop
    		needs motion detect, text to speech converter to say commands, and ultrasonic sensor when kid is far enough away
    		*/
    		case RecordPath:
    		{
    			//thread ultrasonicThread(UltrasonicSensor, ultrasonicData, mode);
    			thread motorThread(MotorController, drivingCommand, mode);
    			thread mapThread(CreateMap, drivingCommand, mode);
    			//ultrasonicThread.join();
    			motorThread.join();
    			mapThread.join();
    			break;
    		}
    		
    		case ExecutePath:
    		{
    		    break;
    		}

		    default:
		    {
		    	break;
		    }
    	}
    	
    }
    socketServer.join();
    delete drivingCommand;
    delete ultrasonicData;
    delete mode;
    delete isMotion;
    rc_cleanup();
    //system("pkill mjpg_streamer");
    return 0;
}

static void MotionDetector(bool* isMotion)
{
	Mat cap1, cap2, cap3;
    VideoCapture capture;
    capture.set(CV_CAP_PROP_FRAME_WIDTH,640);   // width pixels
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);   // height pixels
    capture.open(0);
    //allow camera to warm up
    for(int i = 0; i < 60; i++)
    {
        capture.read(cap1);
    }
    //read background image
    capture.read(cap1);
    blur(cap1, cap1, Size(10,10));
    //take series of images and check for motion
    //how many times to check for motion
    for(int i = 0; i < 50; i++)
    {
        //cout << "Move" << endl;
        //frames in between each check
        for(int x = 0; x < 2; x++)
        {
           capture.read(cap3);
        }
        capture.read(cap2);
        blur(cap2, cap2, Size(10,10));
        DetectMotion(cap1, cap2, isMotion);
    }
    capture.release();
}

static void DetectMotion(Mat img1, Mat img2, bool* isMotion)
{
	//region of interest box
	Point old_tl = Point(0,0);
	Point old_br = Point(0,0);
    //subtract background to create mask
    Mat frame, fgMaskMOG, threshold_output;
    Scalar color = Scalar(100, 100, 100);
    Ptr<BackgroundSubtractorMOG2> pMOG = createBackgroundSubtractorMOG2();
    pMOG->setVarThreshold(75);
    pMOG->setDetectShadows(false);
    frame = img1;
    pMOG->apply(frame, fgMaskMOG);
    frame = img2;
    pMOG->apply(frame, fgMaskMOG);
    blur( fgMaskMOG, fgMaskMOG, Size(25,25) );
    //draw region of interest
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    threshold( fgMaskMOG, threshold_output, 100, 255, THRESH_BINARY );
    findContours( threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    
    //cout << contours.size() << endl;
    for( size_t i = 0; i< contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        Point topLeft = boundRect[i].tl();
        Point bottomRight = boundRect[i].br();
        int sum = ((bottomRight.x - topLeft.x) * (bottomRight.y - topLeft.y));
        //cout << sum << endl;
        if(sum > 1000)
        {
            //calculate if it moved more than 25 pixels
            int changeTop_x = abs(old_tl.x - topLeft.x);
            int changeTop_y = abs(old_tl.y - topLeft.y);
            int changeBot_x = abs(old_br.x - bottomRight.x);
            int changeBot_y = abs(old_br.y - bottomRight.y);
            if(changeTop_x > 50 || changeTop_y > 50 || changeBot_x > 50 || changeBot_y > 50)
            {
                cout << "Motion Detected" << endl;
                *isMotion = true;
            }
            old_tl.x = topLeft.x;
            old_tl.y = topLeft.y;
            old_br.x = bottomRight.x;
            old_br.y = bottomRight.y;
        }
    }
}

static void MotorController(int* data, int* mode)
{
    //cout << "Started motor controller thread" << endl;
    float RIGHT_DUTY = .7;
    float LEFT_DUTY = .8;
    rc_enable_motors();
    int startMode = *mode;
	while(rc_get_state()!=EXITING && (startMode == *mode))
	{
		switch(*data)
		{
		    //stop
			case 0:
			    rc_set_motor(RIGHT, 0);
			    rc_set_motor(LEFT, 0);
			    rc_usleep(100000);
			    break;
			
			//forwards
			case 1:
				rc_set_motor(RIGHT, RIGHT_DUTY);
				rc_set_motor(LEFT, LEFT_DUTY);
				rc_usleep(100000);
				//AdjustMotorPower(RIGHT_DUTY, LEFT_DUTY);
				break;
			
			//backwards	
			case 2:
				rc_set_motor(RIGHT, -RIGHT_DUTY);
				rc_set_motor(LEFT, -LEFT_DUTY);
				rc_usleep(100000);
				break;
			
			//left	
			case 3:
				rc_set_motor(LEFT, RIGHT_DUTY);
				rc_set_motor(RIGHT, -LEFT_DUTY);
				rc_usleep(100000);
				break;
			
			//right
			case 4:
				rc_set_motor(LEFT, -RIGHT_DUTY);
				rc_set_motor(RIGHT, LEFT_DUTY);
				rc_usleep(100000);
				break;
			
			//stop	
			case 5:
				rc_set_motor(LEFT, 0);
				rc_set_motor(RIGHT, 0);
				rc_usleep(100000);
				break;
				
			//tilt head forwards
			case 6:
				rc_usleep(100000);
				break;
				
			//tilt head backwards
			case 7:
				rc_usleep(100000);
				break;
				
			//in case some error occurs
			default:
			    rc_usleep(100000);
			    break;
		}
	}
    rc_disable_motors();
	rc_set_motor(LEFT, 0);
	rc_set_motor(RIGHT, 0);
    cout << "cleaning up from motor thread" << endl;
}

static void AdjustMotorPower(float &RIGHT_DUTY, float &LEFT_DUTY)
{
    int rightCount = 0;
    int leftCount = 0;
    int difference = 0;
    if(LEFT_DUTY < 1)
    {
        rightCount = rc_get_encoder_pos(RIGHT);
        leftCount = rc_get_encoder_pos(LEFT);
        difference = abs(rightCount - leftCount);
        if(difference > 10)
        {
            if(rightCount < leftCount)
            {
                LEFT_DUTY += .01;
            }
            if(leftCount < rightCount)
            {
                LEFT_DUTY -= .01;
            }
        }
        rc_set_encoder_pos(RIGHT, 0);
        rc_set_encoder_pos(LEFT, 0);
    }
}
 
static void UltrasonicSensor(float* distance, int* mode)
{
    //need to implement timeout
    //5 second timeout limit
    const unsigned long long timeout = 5000000000;
    int startMode = * mode;
    rc_gpio_export(TRIGGER);
    while(rc_get_state()!=EXITING && (*mode == startMode))
    {
    	rc_gpio_set_dir(TRIGGER, OUTPUT_PIN);
    	rc_gpio_set_value(TRIGGER, LOW);
    	//wait a second in between each reading
    	this_thread::sleep_for(chrono::seconds(1));
    	//send pulse to sensor
    	unsigned int timeoutStart = rc_nanos_since_epoch();
	    rc_gpio_set_value(TRIGGER, HIGH);
	    rc_usleep(500);
	    rc_gpio_set_value(TRIGGER, LOW);
	    rc_gpio_set_dir(TRIGGER, INPUT_PIN);
	    //wait for pulse to bounce off object and come back
	    //while((rc_gpio_get_value(TRIGGER) == LOW) && (timeoutStart - rc_nanos_since_epoch()) < timeout);
	    while(rc_gpio_get_value(TRIGGER) == LOW);
	    unsigned long startTime = rc_nanos_since_epoch();
	    //calculate how long the "high" pulse is
	    while(rc_gpio_get_value(TRIGGER) == HIGH);
	    unsigned long travelTime = rc_nanos_since_epoch() - startTime;
	    //divide by how for sound travels in a nanosecond * 2 (there and back)
	    *distance = travelTime / 149284.0;
	    rc_gpio_set_value(TRIGGER, LOW);
	    //cout << *distance << endl;
    } 
    rc_gpio_unexport(TRIGGER);
    return;
 }
 
static void CreateMap(int* data, int* mode)
{
    vector<int> commands = {};
    vector<int> leftData = {};
    vector<int> rightData = {};
    rc_set_encoder_pos(LEFT, 0);
    rc_set_encoder_pos(RIGHT, 0);
    int* previous = new int;
    int startMode = *mode;
    *previous = 0;
    while((rc_get_state() != EXITING) && (startMode == *mode))
    {
        if(*data != *previous)
        {
            if(*data != 5)
            {
                commands.push_back(*previous);
                leftData.push_back(rc_get_encoder_pos(LEFT));
                rightData.push_back(rc_get_encoder_pos(RIGHT));
                *previous = *data;
                rc_set_encoder_pos(LEFT, 0);
                rc_set_encoder_pos(RIGHT, 0);
            }
        }
    }
    for(auto i : commands)
    {
        cout << "Commands" << endl;
        cout << i << endl;
    }
    for(auto i : rightData)
    {
        cout << "Right Data" << endl;
        cout << i << endl;
    }
    for(auto i : leftData)
    {
        cout << "Left Data" << endl;
        cout << i << endl;
    }
    delete previous;
}

/*void ExecutePath()
{
	MotorDriver.data = 5;
	System.out.println("Exiting loop");
	Thread.sleep(5000);
	for(int f = 0; f < commands.size(); f++)
	{
		MotorDriver.leftEncoderValue = 0;
		MotorDriver.rightEncoderValue = 0;
		System.out.println("Executing action " + f);
		while(MotorDriver.leftEncoderValue < (leftData.get(f) - 40))
		{
			MotorDriver.data = commands.get(f);
		}
	MotorDriver.data = 5;
	Thread.sleep(1000);
	}
} */

static void SocketServer(int* drivingCommand, float* ultrasonicData, int* mode, double* batteryVoltage)
{
    //socket file descriptors and port number
    cout << "Started socket thread" << endl;
    int sockFd;
    int newsockFd;
    int portNumber = 8420;
    int response;
    //size of client address
    socklen_t clilen;
    //input buffer size
    char buffer[256];
    //server and client addresses
    struct sockaddr_in serv_addr, cli_addr;
    //declare socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    //if the socket fails to open
    if (sockFd < 0) 
    {
        cout<< "ERROR opening socket" << endl;
        rc_cleanup();
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    //set ip address to local host
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber);
    if (bind(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "ERROR on binding" << endl;
        rc_cleanup();
    }
    //listen for incoming connections
    listen(sockFd, 5);
    clilen = sizeof(cli_addr);
    //accept the incoming connection
    newsockFd = accept(sockFd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockFd < 0) 
    {
        cout << "ERROR on accept" << endl;
        rc_cleanup();
    }
    
    //clear out the buffer
    //bzero(buffer, 256);
        
    while(rc_get_state()!=EXITING)
    {
        //clear out the buffer
        bzero(buffer, 256);
        //read from the socket; blocking call
        response = read(newsockFd, buffer, 255);
        if (response < 0) 
        {
            cout << "ERROR reading from socket" << endl;
            break;
        }
        
        //parse the input data
        String respString = buffer;
        
        try
        {
            //create JSON object out of response string
        	json incomingJson = json::parse(respString);
            json outgoingJson;
            
            //parse incoming JSON data
        	*batteryVoltage = incomingJson["/batteryPercentage"_json_pointer];
        	*drivingCommand = incomingJson["/drivingCommand"_json_pointer];
        	*mode = incomingJson["/mode"_json_pointer];
        	*ultrasonicData = incomingJson["/distance"_json_pointer];
        	
        	//create outgoing JSON object
        	outgoingJson["batteryPercentage"] = *batteryVoltage;
        	outgoingJson["drivingCommand"] = *drivingCommand;
        	outgoingJson["distance"] = *ultrasonicData;
        	outgoingJson["mode"] = *mode;
    
            //write to the client
    		String output = outgoingJson.dump();
    		bzero(buffer, 256);
            response = write(newsockFd, output.c_str(), strlen(output.c_str()));
            if (response < 0) 
            {
                cout << "ERROR writing to socket" << endl;
            }
        }
        catch(...)
        {
            cout << "encountered error parsing or sending data" << endl;
            break;
        }
    }
        
    cout << "Cleaning up from socket thread" << endl;
    close(newsockFd);
    close(sockFd);
    //if socket fails, close the whole program
    rc_cleanup();
}


