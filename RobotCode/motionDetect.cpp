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
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

void detectMotion(Mat img1, Mat img2);
Point old_tl = Point(0,0);
Point old_br = Point(0,0);

int main()
{
    Mat cap1, cap2, cap3;
    VideoCapture capture;
    capture.set(CV_CAP_PROP_FRAME_WIDTH,640);   // width pixels
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);   // height pixels
    capture.open(0);
    //allow camera to warm up
    for(int i = 0; i < 30; i++)
    {
        //capture background image
        capture.read(cap1);
    }
    //read background image and then remove noise
    capture.read(cap1);
    blur(cap1, cap1, Size(10,10));
    //take series of images and check for motion against background image
    //how many times to check for motion
    cout << "go" << endl;
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
        detectMotion(cap1, cap2);
    }
    capture.release();
    return 0;
}

//checks for motion from first image to second image
void detectMotion(Mat img1, Mat img2)
{
    //subtract background to create mask
    const int limit = 50;
    //mats to hold images
    Mat frame, fgMaskMOG, threshold_output;
    //color to draw the region of interest box
    Scalar color = Scalar(100, 100, 100);
    Ptr<BackgroundSubtractorMOG2> pMOG = createBackgroundSubtractorMOG2();
    pMOG->setVarThreshold(75);
    pMOG->setDetectShadows(false);
    //add base frame to background subtractor
    //frame = background image
    frame = img1;
    pMOG->apply(frame, fgMaskMOG);
    //frame = new current image
    frame = img2;
    pMOG->apply(frame, fgMaskMOG);
    //blug Mask to remove noise
    blur( fgMaskMOG, fgMaskMOG, Size(25,25) );
    //draw region of interest
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //remove noise
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
            //calculate if it moved more than 50 pixels
            int changeTop_x = abs(old_tl.x - topLeft.x);
            int changeTop_y = abs(old_tl.y - topLeft.y);
            int changeBot_x = abs(old_br.x - bottomRight.x);
            int changeBot_y = abs(old_br.y - bottomRight.y);
            if(changeTop_x > limit || changeTop_y > limit || changeBot_x > limit || changeBot_y > limit)
            {
                cout << "Motion Detected" << endl;
            }
            old_tl.x = topLeft.x;
            old_tl.y = topLeft.y;
            old_br.x = bottomRight.x;
            old_br.y = bottomRight.y;
        }
    }
}