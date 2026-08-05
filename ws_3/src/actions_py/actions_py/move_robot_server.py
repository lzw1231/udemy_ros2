#!/usr/bin/env/python3
import rclpy
import time
from examples_rclpy_minimal_action_server.server_not_composable import execute_callback
from fontTools.varLib.instancer import __main__
from rclpy.node import Node
from rclpy.action import ActionServer
from rclpy.action.server import ServerGoalHandle
from my_robot_interfaces.action import MoveRobot


class MoveRobotServerNode(Node):
    def __init__(self):
        super().__init__("move_robot_server_node")
        self.robot_position_ = 50
        self.move_robot_server = ActionServer(
            self,
            MoveRobot,
            'move_robot',
            execute_callback=self.execute_callback
        )
        self.get_logger().info("Action server has been started")
        self.get_logger().info("Robot position: " + str(self.robot_position_))

    def execute_callback(self, goal_handle: ServerGoalHandle):
        goal_position = goal_handle.request.position
        goal_velocity = goal_handle.request.velocity

        result = MoveRobot.Result()
        feedback = MoveRobot.Feedback()

        self.get_logger().info("Execute goal")

        while rclpy.ok():
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
    rclpy.spin(node)
    rclpy.shutdown()


if __main__ == "__main__":
    main()
