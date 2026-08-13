#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

class Node1 : public rclcpp::Node {
public:
    Node1() : Node("node1") {
        this->timer1_ = this->create_wall_timer(1000ms, [this]() { callback_timer1_(); });
        this->timer2_ = this->create_wall_timer(1000ms, [this]() { callback_timer2_(); });
        this->timer3_ = this->create_wall_timer(1000ms, [this]() { callback_timer3_(); });
    }

private:
    rclcpp::TimerBase::SharedPtr timer1_;
    rclcpp::TimerBase::SharedPtr timer2_;
    rclcpp::TimerBase::SharedPtr timer3_;

    void callback_timer1_() {
        std::this_thread::sleep_for(2000ms);
        RCLCPP_INFO(this->get_logger(), "cb 1");
    }

    void callback_timer2_() {
        std::this_thread::sleep_for(2000ms);
        RCLCPP_INFO(this->get_logger(), "cb 2");
    }

    void callback_timer3_() {
        std::this_thread::sleep_for(2000ms);
        RCLCPP_INFO(this->get_logger(), "cb 3");
    }
};

class Node2 : public rclcpp::Node {
public:
    Node2() : Node("node2") {
        this->timer4_ = this->create_wall_timer(1000ms, [this]() { callback_timer4_(); });
        this->timer5_ = this->create_wall_timer(1000ms, [this]() { callback_timer5_(); });
    }

private:
    rclcpp::TimerBase::SharedPtr timer4_;
    rclcpp::TimerBase::SharedPtr timer5_;


    void callback_timer4_() {
        std::this_thread::sleep_for(2000ms);
        RCLCPP_INFO(this->get_logger(), "cb 4");
    }

    void callback_timer5_() {
        std::this_thread::sleep_for(2000ms);
        RCLCPP_INFO(this->get_logger(), "cb 5");
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);

    auto node1 = std::make_shared<Node1>();
    auto node2 = std::make_shared<Node2>();

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node1);
    executor.add_node(node2);
    executor.spin();
    rclcpp::shutdown();

    return 0;
}
