#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "my_robot_interfaces/action/count_until.hpp"

// Action消息接口别名
using CountUntil = my_robot_interfaces::action::CountUntil;
// Action服务端Goal句柄类型别名
using CountUntilGoalHandle = rclcpp_action::ServerGoalHandle<CountUntil>;

/**
 * @brief CountUntil Action服务端节点类
 * 提供计数动作服务，接收客户端目标请求，周期性反馈进度，任务完成返回最终结果
 */
class CountUntilServerNode : public rclcpp::Node {
public :
    /**
     * @brief 构造函数
     * 创建ROS2节点，实例化Action服务端，注册三组核心回调函数
     */
    CountUntilServerNode() : Node("count_until_server") {
        // 实例化Action服务端
        count_until_server_ = rclcpp_action::create_server<CountUntil>(
            this, // 绑定所属节点
            "count_until", // Action服务名称，客户端通过该名称建立通信
            // 目标请求回调：收到客户端发起新目标时触发
            [this](const rclcpp_action::GoalUUID& goal_uuid, std::shared_ptr<const CountUntil::Goal> goal) {
                return handle_goal_callback(goal_uuid, goal);
            },
            // 取消目标回调：客户端发起取消请求时触发
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
                return handle_cancel_callback(goal_handle);
            },
            // 目标接受回调：服务端同意执行目标(ACCEPT_AND_EXECUTE)之后触发
            [this](const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
                handle_accepted_callback(goal_handle);
            }
        );
        RCLCPP_INFO(this->get_logger(), "Action server has been started");
    }

private:
    /// CountUntil动作服务端共享指针
    rclcpp_action::Server<CountUntil>::SharedPtr count_until_server_;

    /**
     * @brief 目标请求回调函数 Goal Callback
     * @param goal_uuid 本次目标唯一UUID标识
     * @param goal 客户端发送的目标请求数据
     * @return GoalResponse 目标处理策略：REJECT / ACCEPT_AND_EXECUTE / ACCEPT_AND_DEFER
     * @details 触发时机：客户端发送async_send_goal()发起新任务时自动调用；
     *          职责：校验目标合法性，决定拒绝或者接受目标；
     *          注意：此回调运行在主线程，禁止长时间阻塞。
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
     * @brief 目标取消请求回调 Cancel Callback
     * @param goal_handle 待取消任务对应的目标句柄
     * @return CancelResponse 取消请求处理策略：ACCEPT / REJECT
     * @details 触发时机：客户端调用async_cancel_goal()发起任务取消时触发；
     *          职责：决定是否同意客户端取消正在运行的目标；
     */
    rclcpp_action::CancelResponse handle_cancel_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        (void)goal_handle;
        return rclcpp_action::CancelResponse::ACCEPT;
    };

    /**
     * @brief 目标接受完成回调 Accepted Callback
     * @param goal_handle 已被接受的目标任务句柄
     * @details 触发时机：handle_goal_callback 返回 ACCEPT_AND_EXECUTE 之后执行；
     *          职责：启动目标任务的执行逻辑；
     */
    void handle_accepted_callback(const std::shared_ptr<CountUntilGoalHandle> goal_handle) {
        execute_goal(goal_handle);
    };

    /**
     * @brief 目标任务执行主函数
     * @param goal_handle 当前执行任务的目标句柄
     * @details 业务逻辑载体，负责循环执行任务、周期性发布反馈、任务结束生成结果；
     *          可在此内部增加goal_handle->is_canceling()检测，响应客户端取消指令；
     */
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

/**
 * @brief 程序入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 程序退出码
 * @details 初始化ROS2上下文，创建节点实例，进入自旋循环处理消息与回调，程序退出时释放资源
 */
int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CountUntilServerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
