#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <queue>
#include "my_robot_interfaces/action/count_until.hpp"

// Action类型别名
using CountUntil = my_robot_interfaces::action::CountUntil;
// Action目标句柄类型别名
using CountUntilGoalHandle = rclcpp_action::ServerGoalHandle<CountUntil>;

/**
 * @brief CountUntil Action服务端节点
 * 实现计数Action服务，接收目标数字并持续发送反馈
 */
class CountUntilServerNode : public rclcpp::Node {
public :
    /**
     * @brief 构造函数
     * 创建ROS2节点并初始化Action服务端
     */
    CountUntilServerNode() : Node("count_until_server") {
        goal_queue_thread_ = std::thread([this]() { run_goal_queue_thread(); });

        // 创建Action服务端
        count_until_server_ = rclcpp_action::create_server<CountUntil>(
            this, // 绑定当前节点
            "count_until", // Action话题名称
            // Goal Callback：目标接收回调
            [this](const rclcpp_action::GoalUUID& goal_uuid, std::shared_ptr<const CountUntil::Goal> goal) {
                return handle_goal_callback(goal_uuid, goal);
            },
            // Cancel Callback：取消请求回调
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) { return handle_cancel_callback(goal_handle); },
            // Accepted Callback：目标通过确认回调(ACCEPT_AND_EXECUTE模式)
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) { handle_accepted_callback(goal_handle); },
            rcl_action_server_get_default_options(),
            this->create_callback_group(rclcpp::CallbackGroupType::Reentrant)
        );
        RCLCPP_INFO(this->get_logger(), "Action server has been started");
    }

    ~CountUntilServerNode() {
        goal_queue_thread_.join();
    }

private:
    /**
     * @brief 目标接收回调 Goal Callback
     * @param goal_uuid 目标唯一UUID标识
     * @param goal 客户端发送的目标请求数据
     * @return GoalResponse 返回 REJECT / ACCEPT_AND_EXECUTE / ACCEPT_AND_DEFER
     * @details 客户端调用async_send_goal()后首先触发该回调
     *          可在此校验目标参数合法性,决定是否接纳本次目标任务
     */
    rclcpp_action::GoalResponse handle_goal_callback(const rclcpp_action::GoalUUID& goal_uuid,
                                                     std::shared_ptr<const CountUntil::Goal> goal) {
        (void)goal_uuid;
        RCLCPP_INFO(this->get_logger(), "Received a goal");


        if (goal->target_number <= 0) {
            RCLCPP_WARN(this->get_logger(), "Rejecting the goal");
            return rclcpp_action::GoalResponse::REJECT;
        }


        RCLCPP_INFO(this->get_logger(), "Accepting the goal");
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    /**
     * @brief 取消请求回调 Cancel Callback
     * @param goal_handle 当前运行任务的句柄
     * @return CancelResponse 返回 ACCEPT / REJECT
     * @details 客户端调用async_cancel_goal()触发
     *          响应外部任务取消指令
     */
    rclcpp_action::CancelResponse handle_cancel_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Received cancel request.");
        return rclcpp_action::CancelResponse::ACCEPT;
    };

    /**
     * @brief 目标准入回调 Accepted Callback
     * @param goal_handle 当前任务句柄
     * @details 当handle_goal_callback 返回 ACCEPT_AND_EXECUTE 后触发
     *          一般在此启动任务执行函数
     */
    void handle_accepted_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            goal_queue_.push(goal_handle);
            RCLCPP_INFO(this->get_logger(), "Add goal to the queue");
            RCLCPP_INFO(this->get_logger(), "Queue size: %lu", goal_queue_.size());
        }
    };

    void run_goal_queue_thread() {
        rclcpp::Rate loop_rate(1000.0);
        while (rclcpp::ok()) {
            std::shared_ptr<CountUntilGoalHandle> next_goal_handle;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!goal_queue_.empty()) {
                    next_goal_handle = goal_queue_.front();
                    goal_queue_.pop();
                }
            }

            if (next_goal_handle) {
                RCLCPP_INFO(this->get_logger(), "Execute next goal in queue");
                execute_goal(next_goal_handle);
            }

            loop_rate.sleep();
        }
    }

    /**
     * @brief 任务主执行函数
     * @param goal_handle 当前任务句柄
     * @details 实现具体业务逻辑，循环发送反馈
     */
    void execute_goal(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        //Get request from goal
        int target_number = goal_handle->get_goal()->target_number;
        double period = goal_handle->get_goal()->period;

        // Execute the action
        int counter = 0;
        auto result = std::make_shared<CountUntil::Result>();
        auto feedback = std::make_shared<CountUntil::Feedback>();
        rclcpp::Rate loop_rate(1.0 / period);

        for (int i = 0; i < target_number; i++) {
            if (goal_handle->is_canceling()) {
                result->reached_number = counter;
                goal_handle->canceled(result);
                return;
            }

            counter++;
            RCLCPP_INFO(this->get_logger(), "current number: %d", counter);
            feedback->current_number = counter;
            goal_handle->publish_feedback(feedback);
            loop_rate.sleep();
        }

        // Set final state and return result
        result->reached_number = counter;
        goal_handle->succeed(result);
    };

    //!< CountUntil Action服务端，处理目标接收、执行与取消
    rclcpp_action::Server<CountUntil>::SharedPtr count_until_server_;

    //!< 互斥锁，保护goal_handle_多线程并发访问
    std::mutex mutex_;

    std::queue<std::shared_ptr<CountUntilGoalHandle>> goal_queue_;
    std::thread goal_queue_thread_;
};

/**
 * @brief 程序入口
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 程序退出码
 * @details 初始化ROS2上下文，创建节点并进入自旋循环
 */
int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CountUntilServerNode>();
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}
