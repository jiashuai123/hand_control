/* Copyright (C) 2015 CentraleSupélec
 * All rights reserved
 */
#include <ros/ros.h>
#include <ros/time.h>
#include <locale.h>
#include <limits>
#include <math.h>
#include <assert.h>

#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>

#include <hand_control/Plan.h>
#include <geometry_msgs/Twist.h>
#include <math.h>

#include <dynamic_reconfigure/server.h>
#include <hand_control/CommanderConfig.h>


class Run
{
  private:
    float xx, yy, zz, theta; // read coords

    // see README.md to know what are the parameters
    float x_vel, y_vel, z_vel, angle_vel, up_factor, neutral_z;  // parameters
    float max_curv; // not used yet
    float z_dev_min, x_dev_min, y_dev_min, th_dev_min; // parameters : thresholds
    uint64_t min_number; // parameter
    bool no_diag; // parameter

    ros::Publisher pub;

    void publish()
    // build and publish a message from the "xx", "yy", "zz" and "theta" informations
    {
      geometry_msgs::Twist::Ptr mvt(new geometry_msgs::Twist());

      if (fabs(zz) > z_dev_min)
      {
        // up_factor to balance out the gravity effect
        if (zz > 0)
          mvt->linear.z = zz * z_vel * up_factor ;
        else
          mvt->linear.z = zz * z_vel;
      }


      // no_diag true : the drone can only translate on the "x" axis
      // or the "y" axis but not on a linear combination of these axes.
      if (no_diag)
      {
        if (fabs(yy) > fabs(xx) && fabs(yy) > y_dev_min)
        {
          mvt->linear.y = yy * y_vel;
        }
        else if (fabs(xx) > x_dev_min)
        {
          mvt->linear.x = - xx * x_vel;
        }
      } else // no_diag false : the drone can translate on any possible direction
      {
        if (fabs(yy) > y_dev_min)
          mvt->linear.y = yy * y_vel;
        if (fabs(xx) > x_dev_min)
          mvt->linear.x = - xx * x_vel;
      }

      if (fabs(theta) > th_dev_min) {
        mvt->angular.z = theta * angle_vel;
      }

      pub.publish(mvt);
      ROS_INFO("cmd published");
    }

  public:
    Run(const ros::Publisher& cmd_publisher) :
      pub(cmd_publisher) {}

    void callback(const hand_control::Plan::ConstPtr& msg)
    // handle received messages
    {
      ROS_INFO("plan received");
      if (msg->curvature < max_curv && msg->number > min_number)
      // we ever have msg->curvature == 0 in fact (not implemented yet)
      {

        if(msg->normal.z > 0)
        {
          yy = msg->normal.x;
          xx = msg->normal.y;
        }
        else
        {
          yy = - msg->normal.x;
          xx = - msg->normal.y;
        }

        zz = msg->altitude - neutral_z;

        theta = msg->angle;
        // theta between -90 and 90

        ROS_INFO("coords updated");
      } else {
        xx = yy = zz = 0.;
      }
      publish();
    };

    void reconfigure(const hand_control::CommanderConfig& c, const uint32_t& level)
    // updates the parameters (received with dynamic_reconfigure)
    {
      max_curv = c.max_curvature;
      x_dev_min = c.x_minimal_deviation;
      y_dev_min = c.y_minimal_deviation;
      z_dev_min = c.z_minimal_deviation;
      th_dev_min = c.theta_minimal_deviation;
      neutral_z = c.neutral_alt;
      min_number = c.min_points_number;
      up_factor = c.up_fact;
      x_vel = c.x_vel;
      y_vel = c.y_vel;
      z_vel = c.z_vel;
      angle_vel = c.angle_vel;
      no_diag = c.no_diag;
    }  

    void run()
    // runs the callbacks and publications process
    {
      ros::spin();
    }

};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "commander");
  ros::NodeHandle node("commander");
  ros::Publisher cmd_pub = node.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
  Run run(cmd_pub);
  ros::Subscriber plan_sub = node.subscribe<hand_control::Plan>("input", 1, &Run::callback, &run);

  // setting up dynamic_reconfigure (for rqt_reconfigure)
  dynamic_reconfigure::Server<hand_control::CommanderConfig> server;
  dynamic_reconfigure::Server<hand_control::CommanderConfig>::CallbackType f;
  f = boost::bind(&Run::reconfigure, &run, _1, _2);
  server.setCallback(f);

  // starts working
  run.run();
  return 0;
}
