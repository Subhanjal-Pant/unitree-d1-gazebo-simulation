#include <memory>
#include <thread>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>

using moveit::planning_interface::MoveGroupInterface;
using moveit::core::MoveItErrorCode;

static const char* HOME_POSE = "home";
static const char* GRIPPER_CLOSED = "closed";  // change if your SRDF differs

static bool run(MoveGroupInterface& group, const rclcpp::Logger& log, const char* what)
{
  RCLCPP_INFO(log, "Moving: %s", what);
  bool ok = (group.move() == MoveItErrorCode::SUCCESS);
  if (!ok) {
    RCLCPP_ERROR(log, "Failed: %s", what);
  }
  return ok;
}

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("d1_pick_demo", options);
  auto log = node->get_logger();

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinner([&executor]() { executor.spin(); });

  MoveGroupInterface arm(node, "arm");
  MoveGroupInterface gripper(node, "gripper");
  arm.setMaxVelocityScalingFactor(0.5);
  arm.setMaxAccelerationScalingFactor(0.5);

  bool ok = true;

  // 1. home
  arm.setNamedTarget(HOME_POSE);
  ok = ok && run(arm, log, "home");

  // 2. joint target, then record the pose we reached
  geometry_msgs::msg::PoseStamped target;
  if (ok) {
    arm.setJointValueTarget(std::vector<double>{0.5, 0.3, -0.3, 0.0, 0.0, 0.0});
    ok = run(arm, log, "joint target");
    if (ok) {
      target = arm.getCurrentPose();
      RCLCPP_INFO(log, "Recorded target pose: x=%.3f y=%.3f z=%.3f",
                  target.pose.position.x, target.pose.position.y, target.pose.position.z);
    }
  }

  // 3. home again
  if (ok) {
    arm.setNamedTarget(HOME_POSE);
    ok = run(arm, log, "home (again)");
  }

  // 4. go to the recorded pose with inverse kinematics
  if (ok) {
    arm.setPoseTarget(target);
    ok = run(arm, log, "pose target");
  }

  // 5. close gripper
  if (ok) {
    gripper.setNamedTarget(GRIPPER_CLOSED);
    ok = run(gripper, log, "close gripper");
  }

  // 6. home
  if (ok) {
    arm.setNamedTarget(HOME_POSE);
    ok = run(arm, log, "home (final)");
  }

  RCLCPP_INFO(log, ok ? "Demo finished." : "Demo stopped after a failure.");

  rclcpp::shutdown();
  spinner.join();
  return ok ? 0 : 1;
}