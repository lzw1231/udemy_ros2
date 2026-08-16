#include <example_interfaces/msg/int64.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

using namespace std::chrono_literals;
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class NumberPublisherNode : public rclcpp_lifecycle::LifecycleNode {
public:
    NumberPublisherNode() : LifecycleNode("number_publisher") {
        RCLCPP_INFO(this->get_logger(), "IN on_constructor...");

        number_ = 1;
        publish_frequency_ = 1.0;
    }

    CallbackReturn on_configure(const rclcpp_lifecycle::State& previous_state) {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "IN on_configure...");

        number_publisher_ = this->create_publisher<example_interfaces::msg::Int64>("number", 10);
        number_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(static_cast<int>(1000.0 / publish_frequency_)),
            [this]() { this->publishNumber(); });
        number_timer_->cancel();
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State& previous_state) {
        RCLCPP_INFO(this->get_logger(), "IN on_activate...");
        number_timer_->reset();
        rclcpp_lifecycle::LifecycleNode::on_activate(previous_state);
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& previous_state) {
        RCLCPP_INFO(this->get_logger(), "IN on_deactivate...");
        number_timer_->cancel();
        rclcpp_lifecycle::LifecycleNode::on_deactivate(previous_state);
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& previous_state) {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "IN on_cleanup...");
        number_publisher_.reset();
        number_timer_.reset();
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& previous_state) {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "IN on_shutdown...");
        number_publisher_.reset();
        number_timer_.reset();
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_error(const rclcpp_lifecycle::State& previous_state) {
        (void)previous_state;
        RCLCPP_INFO(this->get_logger(), "IN on_error...");
        number_publisher_.reset();
        number_timer_.reset();
        return CallbackReturn::FAILURE;
    }

private:
    void publishNumber() {
        auto msg = example_interfaces::msg::Int64();
        msg.data = number_;
        number_publisher_->publish(msg);
        number_++;
    }

    rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr number_publisher_;
    rclcpp::TimerBase::SharedPtr number_timer_;
    int64_t number_;
    double publish_frequency_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NumberPublisherNode>();
    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
    return 0;
}
