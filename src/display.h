/* Copyright (C) 2015 CentraleSupélec
 * All rights reserved
 */
// library to be used by "keyboard_cmd.cpp"
#ifndef CURSES_DISPLAY
#define CURSES_DISPLAY

#include <ncurses.h>
#include <string>
#include <geometry_msgs/Twist.h>

class Curses
{
  private:
    static const int cmd_kbd_lines;
    static const int cmd_kbd_columns;
    WINDOW* cmd_kbd;
    void print_cmd_kbd();

    static const int cmd_speed_lines;
    static const int cmd_speed_columns;
    WINDOW* cmd_speed;
    void print_cmd_speed();

    static const int get_lines;
    static const int get_columns;
    WINDOW* get;

    static const int log_sent_w_lines;
    static const int log_sent_w_columns;
    int log_line_number;
    WINDOW* log_sent_w;
    WINDOW* log_sent_title;

    static const int nav_data_lines;
    static const int nav_data_columns;
    WINDOW* nav_data;
    void print_nav_data();
  
    static const int topic_lines;
    static const int topic_columns;
    WINDOW* topic;
    void print_topic();

  public:
    Curses();
    ~Curses();
    char getchar();

    // TODO
    void update_cmd_speed(const char& coord, const float& v);
    void update_nav_data(const float& batteryPercent,
                        const int& state,
                        const float& time);
    void log_sent(const std::string& str);
    void update_topic(const geometry_msgs::Twist::ConstPtr& twist);
};

#endif
