#!/usr/bin/env/python3
import rclpy
import time
import threading
from examples_rclpy_minimal_action_server.server_not_composable import execute_callback
from fontTools.varLib.instancer import __main__
from rclpy.node import Node
from rclpy.action import ActionServer, GoalResponse, CancelResponse
from rclpy.action.server import ServerGoalHandle
from my_robot_interfaces.action import MoveRobot
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup


class MoveRobotServerNode(Node):
    def __init__(self):
        super().__init__("move_robot_server_node")
        self.goal_handle_: ServerGoalHandle = None
        self.goal_lock_ = threading.Lock()
        self.robot_position_ = 50
        self.move_robot_server = ActionServer(
            self,
            MoveRobot,
            'move_robot',
            goal_callback=self.goal_callback,
            cancel_callback=self.cancel_callback,
            execute_callback=self.execute_callback,
            callback_group=ReentrantCallbackGroup()
        )
        self.get_logger().info("Action server has been started")
        self.get_logger().info("Robot position: " + str(self.robot_position_))

    def goal_callback(self, goal_request: MoveRobot.Goal):
        self.get_logger().info("Received a new goal")

        if goal_request.position not in range(0, 100) or goal_request.velocity <= 0:
            self.get_logger().warn("Invalid position/velocity, reject goal")
            return GoalResponse.REJECT

        # New goal is valid, abort previous goal and accept new goal
        if self.goal_handle_ is not None and self.goal_handle_.is_active:
            self.goal_handle_.abort()

        self.get_logger().info("Accept goal")
        return GoalResponse.ACCEPT

    def cancel_callback(self, goal_handle: ServerGoalHandle):
        self.get_logger().info("Received a cancel request")
        return CancelResponse.ACCEPT

    def execute_callback(self, goal_handle: ServerGoalHandle):
        with self.goal_lock_:
            self.goal_handle_ = goal_handle

        goal_position = goal_handle.request.position
        goal_velocity = goal_handle.request.velocity

        result = MoveRobot.Result()
        feedback = MoveRobot.Feedback()

        self.get_logger().info("Execute goal")

        while rclpy.ok():
            if not goal_handle.is_active:
                result.position = self.robot_position_
                result.message = "Preempted by another goal"
                return result

            if goal_handle.is_cancel_requested:
                result.position = self.robot_position_
                result.message = "Canceled"
                goal_handle.canceled()
                return result

            diff = goal_position - self.robot_position_

            if diff == 0:
                result.position = self.robot_position_
                result.message = "SUCCESS"
                goal_handle.succeed()
                return result
            elif diff > 0:
                if diff >= goal_velocity:
                    self.robot_position_ += goal_velocity
                else:
                    self.robot_position_ += diff
            else:
                if abs(diff) >= goal_velocity:
                    self.robot_position_ -= goal_velocity
                else:
                    self.robot_position_ -= abs(diff)

            self.get_logger().info("Robot position: " + str(self.robot_position_))

            feedback.current_position = self.robot_position_
            goal_handle.publish_feedback(feedback)

            time.sleep(1.0)


def main(args=None):
    rclpy.init(args=args)
    node = MoveRobotServerNode()
    rclpy.spin(node, MultiThreadedExecutor())
    rclpy.shutdown()


if __main__ == "__main__":
    main()
