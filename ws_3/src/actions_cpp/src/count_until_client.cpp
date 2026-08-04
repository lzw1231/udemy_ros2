#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "my_robot_interfaces/action/count_until.hpp"

using CountUntil = my_robot_interfaces::action::CountUntil;
using CountUntilGoalHandle = rclcpp_action::ClientGoalHandle<CountUntil>;

/**
 * @brief CountUntil Action 客户端节点
 */
class CountUntilClientNode : public rclcpp::Node {
public :
    /**
     * @brief 构造函数
     */
    CountUntilClientNode() : Node("count_until_client") {
        count_until_client_ = rclcpp_action::create_client<CountUntil>(this, "count_until");
    }

    /**
     * @brief 向服务端发送 Action 目标
     * @param target_number 目标计数值
     * @param period 计数间隔周期
     */
    void send_goal(int target_number, double period) {
        // 等待 Action Server 上线
        count_until_client_->wait_for_action_server();

        // 构造目标请求
        auto goal = CountUntil::Goal();
        goal.target_number = target_number;
        goal.period = period;

        // 配置SendGoalOptions，注册三类Action回调
        auto options = rclcpp_action::Client<CountUntil>::SendGoalOptions();
        // 任务结束结果回调 Result Callback
        options.result_callback = [this](const CountUntilGoalHandle::WrappedResult& result) {
            goal_result_callback(result);
        };
        // 目标应答回调 Goal Response Callback（接收/拒绝目标）
        options.goal_response_callback = [this](const CountUntilGoalHandle::SharedPtr& goal_handle) {
            goal_response_callback(goal_handle);
        };
        // 过程反馈回调 Feedback Callback（运行中持续接收反馈）
        options.feedback_callback = [this](const CountUntilGoalHandle::SharedPtr& goal_handle,
                                           const std::shared_ptr<const CountUntil::Feedback>& feedback) {
            goal_feedback_callback(goal_handle, feedback);
        };

        // 异步发送目标
        RCLCPP_INFO(this->get_logger(), "Sending goal with period %f", period);
        count_until_client_->async_send_goal(goal, options);

        // 2秒后发送取消请求  <取消发送>的测试代码
        // timer_ = this->create_wall_timer(std::chrono::seconds(2), [this]() {
        //     timer_callback();
        // });
    }

private:
    // Action Client 实例
    rclcpp_action::Client<CountUntil>::SharedPtr count_until_client_;
    CountUntilGoalHandle::SharedPtr goal_handle_;
    rclcpp::CallbackGroup::SharedPtr cb_group_;

    // 定时器
    rclcpp::TimerBase::SharedPtr timer_;


    void timer_callback() {
        RCLCPP_WARN(this->get_logger(), "Cancel the goal");
        count_until_client_->async_cancel_goal(this->goal_handle_);
        timer_->cancel();
    }

    /**
     * @brief 目标响应回调（Goal Response Callback）
     * @param goal_handle 本次目标对应的句柄
     * @details 服务端确认接收/拒绝目标时触发
     */
    void goal_response_callback(const CountUntilGoalHandle::SharedPtr& goal_handle) {
        if (!goal_handle) {
            RCLCPP_WARN(this->get_logger(), "Goal got rejected!");
        } else {
            this->goal_handle_ = goal_handle;
            RCLCPP_INFO(this->get_logger(), "Goal got accepted!");
        }
    }

    /**
     * @brief 反馈回调（Feedback Callback）
     * @param goal_handle 本次目标对应的句柄
     * @param feedback 服务端推送的实时反馈数据
     * @details 任务运行期间持续触发
     */
    void goal_feedback_callback(const CountUntilGoalHandle::SharedPtr& goal_handle,
                                const std::shared_ptr<const CountUntil::Feedback>& feedback) {
        (void)goal_handle;
        int number = feedback->current_number;
        RCLCPP_INFO(this->get_logger(), "Got feedback: %d", number);
    }

    /**
     * @brief 结果回调（Result Callback）
     * @param result 服务端返回的最终结果封装对象
     * @details 任务结束后触发：SUCCEEDED / ABORTED / CANCELED
     */
    void goal_result_callback(const CountUntilGoalHandle::WrappedResult& result) {
        auto status = result.code;
        if (status == rclcpp_action::ResultCode::SUCCEEDED) {
            RCLCPP_INFO(this->get_logger(), "Succeeded");
        } else if (status == rclcpp_action::ResultCode::ABORTED) {
            RCLCPP_ERROR(this->get_logger(), "Aborted");
        } else if (status == rclcpp_action::ResultCode::CANCELED) {
            RCLCPP_WARN(this->get_logger(), "Canceled");
        }


        int reached_number = result.result->reached_number;
        RCLCPP_INFO(this->get_logger(), "Reached number %d", reached_number);
    }
};

/**
 * @brief 程序入口
 */
int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CountUntilClientNode>();
    node->send_goal(6, 1.0);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
