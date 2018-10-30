

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <iostream>
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
using namespace cv;
using namespace std;


int main(int, char**){
    
    //SETUP FIFO
    int fd;
    int buf;
    if((fd=open("fifo",O_WRONLY))<0){
    	perror("error opening fifo");
    	exit(EXIT_FAILURE);
    }
    //cara write:
    
    //write(fd,buf,sizeof(int));
    
    
    
    
    //////////////////////
	
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_WIDTH,360);

     while(1)
    {
        Mat bgr_img;
        cap >> bgr_img; // get a new frame from camera

        if(bgr_img.empty()){
            printf("kosong!\n"); fflush(stdout);
            //getchar();
        }

        Mat orig_img = bgr_img.clone();

        medianBlur(bgr_img, bgr_img, 3);

        Mat hsv_img;
        cvtColor(bgr_img, hsv_img, cv::COLOR_BGR2HSV);

        Mat orange_hue_range;
        Mat red_hue_image;
        //inRange(hsv_img,cv::Scalar(10,100,160),cv::Scalar(65,255,255),orange_hue_range);
        inRange(hsv_img,cv::Scalar(10,127,127),cv::Scalar(65,255,255),orange_hue_range);
        Mat ranged = orange_hue_range;

        red_hue_image=orange_hue_range;
        GaussianBlur(orange_hue_range, red_hue_image, cv::Size(9,9), 2, 2);
        //GaussianBlur(orange_hue_range, red_hue_image, cv::Size(21,21), 5, 5);

        vector<Vec3f> circles;
        HoughCircles(red_hue_image, circles, CV_HOUGH_GRADIENT, 1, CAMERA_WIDTH/2,150, 20, 0, CAMERA_HEIGHT);
	//HoughCircles(red_hue_image, circles, CV_HOUGH_GRADIENT, 1, CAMERA_WIDTH/2,150, 50, 0, CAMERA_HEIGHT);

        printf("%d\n",circles.size());fflush(stdout);
	if(circles.size()==0){
		write(fd,"0",sizeof(int));
	}
	else{
	
	        for(size_t current_circle = 0; current_circle < circles.size(); ++current_circle) {
	            Point center(circles[current_circle][0], circles[current_circle][1]);
	            int radius = circles[current_circle][2];
	
	            circle(orig_img, center, radius, cv::Scalar(0, 255, 0), 5);
		    if(radius>(CAMERA_HEIGHT/4)){
	            	write(fd,"4",sizeof(int));
	                printf("dekat\n"); fflush(stdout);
	       	    }
	            else if(circles[current_circle][0]>((CAMERA_WIDTH/2)+50)){
	                write(fd,"1",sizeof(int));
	                printf("kanan\n"); fflush(stdout);
	            }
	            else if(circles[current_circle][0]<((CAMERA_WIDTH/2)-50)){
	                write(fd,"2",sizeof(int));
	                printf("kiri\n"); fflush(stdout);
	            }
	            else if(circles[current_circle][0]<((CAMERA_WIDTH/2)+50) &&
	            	circles[current_circle][0]>((CAMERA_WIDTH/2)-50)){
	            	if(radius>(CAMERA_HEIGHT/4)){
		            	write(fd,"4",sizeof(int));
		                printf("dekat\n"); fflush(stdout);
	       	    	}
	       	    	else{
	            		write(fd,"3",sizeof(int));
	                	printf("tengah\n"); fflush(stdout);
	                }
	       	    }
	       	    
	     	}
        }
        //640 x 360
        //namedWindow("Threshold orange image", cv::WINDOW_AUTOSIZE);
        //imshow("Threshold orange image", bgr_img);
        //namedWindow("Combined threshold images", cv::WINDOW_AUTOSIZE);
        //imshow("Combined threshold images", red_hue_image);
        //namedWindow("Detected red circles on the input image", cv::WINDOW_AUTOSIZE);
        //imshow("Detected red circles on the input image", orig_img);


        if(waitKey(30) >= 0) break;
    }
    close(fd);
    return 0;
}



