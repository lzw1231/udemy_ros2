#include "components_cpp/node2.hpp"

Node2::Node2() : Node("node2") {
    timer4_ = create_wall_timer(1000ms, [this]() { callback_timer4(); });
    timer5_ = create_wall_timer(1000ms, [this]() { callback_timer5(); });
}

void Node2::callback_timer4() {
    std::this_thread::sleep_for(2000ms);
    RCLCPP_INFO(this->get_logger(), "cb 4");
}

void Node2::callback_timer5() {
    std::this_thread::sleep_for(2000ms);
    RCLCPP_INFO(this->get_logger(), "cb 5");
}

