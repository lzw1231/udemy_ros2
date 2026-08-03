#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "my_robot_interfaces/action/count_until.hpp"

using CountUntil = my_robot_interfaces::action::CountUntil;
using CountUntilGoalHandle = rclcpp_action::ServerGoalHandle<CountUntil>;

class CountUntilServerNode : public rclcpp::Node {
public :
    CountUntilServerNode() : Node("count_until_server") {
        count_until_server_ = rclcpp_action::create_server<CountUntil>(
            this,
            "count_until",
            [this](const rclcpp_action::GoalUUID &goal_uuid, std::shared_ptr<const CountUntil::Goal> goal) {
                return handle_goal_callback(goal_uuid, goal);
            },
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
                return handle_cancel_callback(goal_handle);
            },
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
                handle_accepted_callback(goal_handle);
            }
        );
        RCLCPP_INFO(this->get_logger(), "Action server has been started");
    }

private:
    rclcpp_action::Server<CountUntil>::SharedPtr count_until_server_;

    rclcpp_action::GoalResponse handle_goal_callback(const rclcpp_action::GoalUUID &goal_uuid, std::shared_ptr<const CountUntil::Goal> goal) {
        (void) goal_uuid;

        RCLCPP_INFO(this->get_logger(), "Received a goal");

        if (goal->target_number <= 0) {
            RCLCPP_WARN(this->get_logger(), "Rejecting the goal");
            return rclcpp_action::GoalResponse::REJECT;
        }
        RCLCPP_INFO(this->get_logger(), "Accepting the goal");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    rclcpp_action::CancelResponse handle_cancel_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        (void) goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    };

    void handle_accepted_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        execute_goal(goal_handle);
    };

    void execute_goal(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        //Get request from goal
        int target_number = goal_handle->get_goal()->target_number;
        double period = goal_handle->get_goal()->period;

        // Execute the action
        int counter = 0;
        auto feedback = std::make_shared<CountUntil::Feedback>();
        rclcpp::Rate loop_rate(1.0 / period);

        for (int i = 0; i < target_number; i++) {
            counter++;
            RCLCPP_INFO(this->get_logger(), "current number: %d", counter);
            feedback->current_number = counter;
            goal_handle->publish_feedback(feedback);
            loop_rate.sleep();
        }

        // Set final state and return result
        auto result = std::make_shared<CountUntil::Result>();
        result->reached_number = counter;
        goal_handle->succeed(result);
    };
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CountUntilServerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}