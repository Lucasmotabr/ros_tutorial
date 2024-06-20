#include <ros/ros.h>
#include <std_msgs/String.h>
#include <web_speech_recognition/SpeechRecognition.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Odometry.h>

ros::Publisher cmd_vel_pub;
nav_msgs::Odometry last_odom;

void odomCallback(const nav_msgs::Odometry::ConstPtr& msg){
    last_odom = *msg;
}

void moveRobot(double distance){
    ros::Rate loop_rate(10);
    geometry_msgs::Twist twist;
    double initial_x = last_odom.pose.pose.position.x;
    double current_x = initial_x;

    while((current_x - initial_x) < distance){
        twist.linear.x = 0.5;
        cmd_vel_pub.publish(twist);
        ros::spinOnce();
        loop_rate.sleep();
        current_x = last_odom.pose.pose.position.x;
    }

    twist.linear.x = 0.0;
    cmd_vel_pub.publish(twist);
}

void processRecognitionResul(const std::vector<std::string>&words){
    //check command
    for(const std::string& word : words){
        //Check if the word is "go"
        if (word == "go"){
            moveRobot(1.0);

        }
    }
}


int main(int argc, char **argv){
    //Initialize ROS node
    ros ::init(argc, argv, "web_speech_recognition_client");
    ros::NodeHandle nh;

    //create a client for the SpeechRecognition Service
    ros::ServiceClient client = nh.serviceClient<web_speech_recognition::SpeechRecognition>("/speech_recognition");

    //Prepare request
    web_speech_recognition::SpeechRecognition srv;
    srv.request.timeout_sec = 10;

    //Subscriber for odometry
    ros::Subscriber odom_sub = nh.subscribe("/odom",10, odomCallback);

    //Publisher for robot movement
    cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10)



    //Call the service
    if (client.call(srv)){
        ROS_INFO("Recognition Result:");
        processRecognitionResult(srv.response.transcript);
        

    } else{
        ROS_ERROR("Failed to call web_speech_recognition");
        return 1;
    }
    return 0;
}