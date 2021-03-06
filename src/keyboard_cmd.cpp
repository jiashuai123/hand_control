/* Copyright © 2015 CentraleSupélec
 * 
 * This file is part of Hand Control.
 * 
 * Hand Control is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Hand Control is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Hand Control.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <ros/ros.h>
#include <ros/time.h>
#include <locale.h>

#include <std_msgs/Empty.h>
#include <geometry_msgs/Twist.h>
#include <ardrone_autonomy/Navdata.h>

#include "./display.h"

class NavdataCallback {
 private:
    boost::shared_ptr<Curses> term;

 public:
    explicit NavdataCallback(const boost::shared_ptr<Curses>& terminal) :
        term(terminal) {}

    void operator()(const ardrone_autonomy::Navdata::ConstPtr& msg) {
        term->update_nav_data(msg->batteryPercent, msg->state, msg->tm);
    }
};

class CmdVelCallback {
 private:
    boost::shared_ptr<Curses> term;

 public:
    explicit CmdVelCallback(const boost::shared_ptr<Curses>& terminal) :
      term(terminal) {}

    void operator()(const geometry_msgs::Twist::ConstPtr& msg) {
      term->update_topic(msg);
    }
};

class Run {
 private:
    std_msgs::Empty empty;
    ros::NodeHandle n;
    ros::Rate loop_rate;
    ros::Publisher cmd, pub_takeoff, pub_land, pub_reset;
    ros::Subscriber data_subscriber, cmdvel_subscriber;
    void land() { pub_land.publish(empty); }
    void takeoff() { pub_takeoff.publish(empty); }
    void reset() { pub_reset.publish(empty); }
    float x_speed, y_speed, z_speed, turn;
    boost::shared_ptr<Curses> term;
    NavdataCallback data_callback;
    CmdVelCallback cmdvel_callback;

 public:
    explicit Run(const boost::shared_ptr<Curses>& terminal) :
        data_callback(terminal),
        cmdvel_callback(terminal),
        term(terminal),
        loop_rate(30),
        x_speed(0.1),
        y_speed(0.1),
        z_speed(0.1),
        turn(0.1) {
            cmd =  n.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
            pub_takeoff =  n.advertise<std_msgs::Empty>("/ardrone/takeoff", 1);
            pub_land = n.advertise<std_msgs::Empty>("/ardrone/land", 1);
            pub_reset = n.advertise<std_msgs::Empty>("/ardrone/reset", 1);

            data_subscriber =
                n.subscribe<ardrone_autonomy::Navdata>(
                    "/ardrone/navdata", 1, data_callback);

            cmdvel_subscriber =
                n.subscribe<geometry_msgs::Twist>(
                    "/cmd_vel", 1, cmdvel_callback);

            term->update_cmd_speed('x', x_speed);
            term->update_cmd_speed('y', y_speed);
            term->update_cmd_speed('z', z_speed);
            term->update_cmd_speed('t', turn);

            float a(0);
            int s(0);
            float time(0);
            term->update_nav_data(a, s, time);
    }

    void operator()() {
        while (ros::ok()) {
            ros::spinOnce();

            geometry_msgs::Twist::Ptr msg(new geometry_msgs::Twist());
            msg->linear.x = msg->linear.y = msg->linear.z =
            msg->angular.x = msg->angular.y = msg->angular.z = 0.;

            char c = term->getchar();

            switch (c) {
                case 'k' : {  // hover
                    cmd.publish(msg);
                    term->log_sent("hover !");
                    break;
                }
                case 'i' : {  // forward
                    msg->linear.x = x_speed;
                    cmd.publish(msg);
                    term->log_sent("forward !");
                    break;
                }
                case ';' : {  // backward
                    msg->linear.x = -x_speed;
                    cmd.publish(msg);
                    term->log_sent("backward !");
                    break;
                }
                case 'h' : {  // translate left
                    msg->linear.y = -y_speed;
                    cmd.publish(msg);
                    term->log_sent("translate left !");
                    break;
                }
                case 'm' : {  // translate right
                    msg->linear.y = y_speed;
                    cmd.publish(msg);
                    term->log_sent("translate right !");
                    break;
                }
                case 'j' : {  // rotate left
                    msg->angular.z = turn;
                    cmd.publish(msg);
                    term->log_sent("rotate left !");
                    break;
                }
                case 'l' : {  // rotate right
                    msg->angular.z = -turn;
                    cmd.publish(msg);
                    term->log_sent("rotate right !");
                    break;
                }
                case 'u' : {  // turn left
                    msg->angular.z = turn;
                    msg->linear.x = x_speed;
                    cmd.publish(msg);
                    term->log_sent("forward left !");
                    break;
                }
                case 'o' : {  // turn right
                    msg->angular.z = -turn;
                    msg->linear.x = x_speed;
                    cmd.publish(msg);
                    term->log_sent("forward right !");
                    break;
                }
                case ',' : {  // turn left backward
                    msg->angular.z = turn;
                    msg->linear.x = -x_speed;
                    cmd.publish(msg);
                    term->log_sent("backward left !");
                    break;
                }
                case ':' : {  // turn right backward
                    msg->angular.z = -turn;
                    msg->linear.x = -x_speed;
                    cmd.publish(msg);
                    term->log_sent("backward right !");
                    break;
                }
                case 'y' : {  // up
                    msg->linear.z = z_speed;
                    cmd.publish(msg);
                    term->log_sent("up !");
                    break;
                }
                case 'n' : {  // down
                    msg->linear.z = -z_speed;
                    cmd.publish(msg);
                    term->log_sent("down !");
                    break;
                }
                case 't' : {  // takeoff
                    takeoff();
                    term->log_sent("takeoff !");
                    break;
                }
                case 'b' : {  // land
                    land();
                    term->log_sent("land !");
                    break;
                }
                case 'g' : {  // reset
                    reset();
                    term->log_sent("reset !");
                    break;
                }
                case 'a' : {  // + x_speed
                    x_speed *= 1.1;
                    term->update_cmd_speed('x', x_speed);
                    break;
                }
                case 'w' : {  // - x_speed
                    x_speed *= 0.9;
                    term->update_cmd_speed('x', x_speed);
                    break;
                }
                case 'z' : {  // + y_speed
                    y_speed *= 1.1;
                    term->update_cmd_speed('y', y_speed);
                    break;
                }
                case 'x' : {  // - y_speed
                    y_speed *= 0.9;
                    term->update_cmd_speed('y', y_speed);
                    break;
                }
                case 'e' : {  // + z_speed
                    z_speed *= 1.1;
                    term->update_cmd_speed('z', z_speed);
                    break;
                }
                case 'c' : {  // - z_speed
                    z_speed *= 0.9;
                    term->update_cmd_speed('z', z_speed);
                    break;
                }
                case 'r' : {  // + turn speed
                    turn *= 1.1;
                    term->update_cmd_speed('t', turn);
                    break;
                }
                case 'v' : {  // - turn speed
                    turn *= 0.9;
                    term->update_cmd_speed('t', turn);
                    break;
                }
                default : {
                    cmd.publish(msg);
                    term->log_sent("hover !");
                }
            }  // switch(c)
            loop_rate.sleep();
        }  // while
    }  // void run()
};  // class Run

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    ros::init(argc, argv, "keyboard_cmd");
    boost::shared_ptr<Curses> term(new Curses());
    Run fun(term);
    fun();
    return 0;
}
