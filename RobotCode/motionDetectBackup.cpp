//opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
//C
#include <stdio.h>
#include <unistd.h>
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

int main()
{
    Mat fgMaskMOG, frame, cap1, cap2, edge1, edge2;
    VideoCapture capture;
    capture.set(CV_CAP_PROP_FRAME_WIDTH,640);   // width pixels
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);   // height pixels
    capture.open(0);
    //allow camera to warm up
    for(int i = 0; i < 60; i++)
    {
        capture.read(cap1);
    }
    capture.read(cap1);
    cout << "Move" << endl;
    //capture.set(CV_CAP_PROP_FRAME_WIDTH,640);   // width pixels
    //capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);   // height pixels
    for(int i = 0; i < 60; i++)
    {
        capture.read(cap2);
    }
    capture.read(cap2);
    capture.release();
    cout << "Processing - Performing Image Processing" << endl;
    blur(cap1, cap1, Size(10,10));
    blur(cap2, cap2, Size(10,10));
    //Canny(cap1, edge1, 195, 200, 3);
    //Canny(cap2, edge2, 195, 200, 3);
    //subtract background to create mask
    Ptr<BackgroundSubtractorMOG2> pMOG = createBackgroundSubtractorMOG2();
    //cout << pMOG->getVarThreshold() << endl;
    pMOG->setVarThreshold(75);
    pMOG->setDetectShadows(false);
    frame = cap1;
    pMOG->apply(frame, fgMaskMOG);
    frame = cap2;
    pMOG->apply(frame, fgMaskMOG);
    blur( fgMaskMOG, fgMaskMOG, Size(25,25) );
    //fgMaskMOG = edge2 - edge1;
    imwrite("Mog.png", fgMaskMOG);
    imwrite("Cap1.png", cap1);
    imwrite("Cap2.png", cap2);
    //imwrite("Edge1.png", edge1);
    //imwrite("Edge2.png", edge2);
    //draw region of interest
    Mat threshold_output, src_gray;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //read image and convert to Grayscale
    //Mat src = imread("/var/lib/cloud9/Mog.png", IMREAD_COLOR );
    //cvtColor( fgMaskMOG, src_gray, COLOR_BGR2GRAY );
    threshold( fgMaskMOG, threshold_output, 100, 255, THRESH_BINARY );
    findContours( threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
    }
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar(100, 100, 100);
        //drawContours( drawing, contours_poly, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
    }
    imwrite("Drawing.png", drawing);
    return 0;
}