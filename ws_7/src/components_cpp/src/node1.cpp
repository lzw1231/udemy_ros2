#include "components_cpp/node1.hpp"

Node1::Node1() : Node("node1") {
    timer1_ = create_wall_timer(1000ms, [this]() { callback_timer1(); });
    timer2_ = create_wall_timer(1000ms, [this]() { callback_timer2(); });
    timer3_ = create_wall_timer(1000ms, [this]() { callback_timer3(); });
}

void Node1::callback_timer1() {
    std::this_thread::sleep_for(2000ms);
    RCLCPP_INFO(this->get_logger(), "cb 1");
}

void Node1::callback_timer2() {
    std::this_thread::sleep_for(2000ms);
    RCLCPP_INFO(this->get_logger(), "cb 2");
}

void Node1::callback_timer3() {
    std::this_thread::sleep_for(2000ms);
    RCLCPP_INFO(this->get_logger(), "cb 3");
}
