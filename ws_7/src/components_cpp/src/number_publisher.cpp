//========= 1、所有#include全部放文件最顶部 =========
#include "components_cpp/number_publisher.hpp"
#include <rclcpp_components/register_node_macro.hpp>

using namespace std::chrono_literals;

//========= 2、namespace 包裹类实现 =========
namespace my_namespace{
    NumberPublisher::NumberPublisher(const rclcpp::NodeOptions& options)
        : Node("number_publisher", options) {
        number_ = 2;
        this->get_node_base_interface();
        number_publisher_ = this->create_publisher<example_interfaces::msg::Int64>("number", 10);
        number_timer_ = this->create_wall_timer(1000ms, [this]() { publishNumber(); });
        RCLCPP_INFO(this->get_logger(), "Number publisher has been started.");
    }

    void NumberPublisher::publishNumber() {
        auto msg = example_interfaces::msg::Int64();
        msg.data = number_;
        number_publisher_->publish(msg);
    }
} // namespace my_namespace  <==== namespace在这里结束闭合


//=========3、注册宏！必须写在namespace大括号外面！全局域！=========
RCLCPP_COMPONENTS_REGISTER_NODE(my_namespace::NumberPublisher);
