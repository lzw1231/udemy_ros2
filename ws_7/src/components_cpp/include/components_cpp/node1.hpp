#pragma once

#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

class Node1 : public rclcpp::Node {
public:
    Node1();

private:
    void callback_timer1();
    void callback_timer2();
    void callback_timer3();

    rclcpp::TimerBase::SharedPtr timer1_;
    rclcpp::TimerBase::SharedPtr timer2_;
    rclcpp::TimerBase::SharedPtr timer3_;
};
