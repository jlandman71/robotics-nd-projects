#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <string>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested velocities
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    int left_border = (int)round(img.width * 0.3);
    int right_border = (int)round(img.width * 0.7);
    int detect_position;
    bool ball_detected = false;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    // loop through all pixels, evaluating all 3 RGB values per pixel 
    for (int i = 0; i < (img.height * img.width); i++) {

        // detect white pixel when all 3 RGB values have white_pixel value
        if (img.data[3*i] == white_pixel && img.data[(3*i)+1] == white_pixel && img.data[(3*i)+2] == white_pixel) {
                             
            
            // calulate detected x position as (pixel index % img width)
            detect_position = i % img.width; 

            std::string msg = "ProcessImage - White ball detected - position:" + std::to_string(i);
            msg = msg + " , detect_position:" + std::to_string(detect_position);
            msg = msg + " , left_border:" + std::to_string(left_border);
            msg = msg + " , right_border:" + std::to_string(right_border);
            msg = msg + " , step:" + std::to_string(img.step);
            msg = msg + " , height:" + std::to_string(img.height);
            msg = msg + " , width:" + std::to_string(img.width);
            ROS_INFO_STREAM(msg);

            if (detect_position < left_border){
              // ball detected at left side
              // turn robot to left
              ball_detected = true;
              ROS_INFO_STREAM("turn robot to left");
              drive_robot(0.0, 0.5);
              break;
            } 
            else if (detect_position < right_border){
              // ball detected at mid side
              // drive robot straight forward
              ball_detected = true;
              ROS_INFO_STREAM("drive robot straight forward");
              drive_robot(0.5, 0.0);
              break;
            }
            else {
              // ball detected at right side
              // turn robot to right
              ball_detected = true;
              ROS_INFO_STREAM("turn robot to right");
              drive_robot(0.0, -0.5);
              break;
            }   
        }
    }
    if (!ball_detected){
      // no ball detected
      // stop robot
      drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
