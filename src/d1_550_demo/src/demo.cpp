#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>

using moveit::planning_interface::MoveGroupInterface;
using moveit::core::MoveItErrorCode;
using Joints = std::vector<double>;                    // Joint1..Joint6 in radians
using Waypoint = std::pair<std::string, Joints>;

static const char* HOME_POSE = "home";
static const char* GRIPPER_OPEN = "open";      // change if your SRDF names differ
static const char* GRIPPER_CLOSED = "closed";

struct Demo
{
  rclcpp::Logger log;
  MoveGroupInterface& arm;
  MoveGroupInterface& gripper;
  int failures = 0;

  Demo(rclcpp::Logger l, MoveGroupInterface& a, MoveGroupInterface& g)
  : log(l), arm(a), gripper(g) {}

  bool execute(MoveGroupInterface& group, const std::string& label)
  {
    RCLCPP_INFO(log, "Moving: %s", label.c_str());
    bool ok = (group.move() == MoveItErrorCode::SUCCESS);
    if (!ok) {
      ++failures;
      RCLCPP_WARN(log, "Skipped (planning or execution failed): %s", label.c_str());
    }
    return ok;
  }

  bool named(MoveGroupInterface& group, const char* name)
  {
    if (!group.setNamedTarget(name)) {
      ++failures;
      RCLCPP_WARN(log, "Unknown named target: %s", name);
      return false;
    }
    return execute(group, name);
  }

  bool joints(const std::string& label, const Joints& values)
  {
    if (!arm.setJointValueTarget(values)) {
      ++failures;
      RCLCPP_WARN(log, "Target outside joint limits: %s", label.c_str());
      return false;
    }
    return execute(arm, label);
  }

  void sequence(const std::string& title, const std::vector<Waypoint>& waypoints)
  {
    RCLCPP_INFO(log, "=== %s ===", title.c_str());
    for (const auto& wp : waypoints) {
      joints(wp.first, wp.second);
    }
    named(arm, HOME_POSE);
  }

  void pause(int ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("d1_pick_demo", options);

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spinner([&executor]() { executor.spin(); });

  MoveGroupInterface arm(node, "arm");
  MoveGroupInterface gripper(node, "gripper");
  arm.setMaxVelocityScalingFactor(0.5);        // raise toward 1.0 for faster motion
  arm.setMaxAccelerationScalingFactor(0.5);
  arm.setPlanningTime(5.0);

  Demo demo(node->get_logger(), arm, gripper);

  // Start from home
  demo.named(arm, HOME_POSE);

  // 1. Right-left sweep (Joint1, limit +-2.36)
  demo.sequence("Left-right sweep", {
    {"left",       {0.6, 0.0, 0.0, 0.0, 0.0, 0.0}},
    {"far left",   {1.3, 0.0, 0.0, 0.0, 0.0, 0.0}},
    {"right",      {-0.6, 0.0, 0.0, 0.0, 0.0, 0.0}},
    {"far right",  {-1.3, 0.0, 0.0, 0.0, 0.0, 0.0}},
  });

  // 2. Up-down (Joint2 and Joint3, limit +-1.57)
  demo.sequence("Up and down", {
    {"raise",      {0.0, -0.4, -0.5, 0.0, 0.0, 0.0}},
    {"lower",      {0.0, 0.5, 0.2, 0.0, 0.0, 0.0}},
    {"raise high", {0.0, -0.6, -0.9, 0.0, 0.0, 0.0}},
    {"reach out",  {0.0, 0.8, -0.2, 0.0, 0.0, 0.0}},
  });

  // 3. Wrist rotation back and forth (Joint4 and Joint6 +-2.36, Joint5 +-1.57)
  demo.sequence("Wrist rotation", {
    {"roll J4 +",  {0.0, 0.0, 0.0, 1.5, 0.0, 0.0}},
    {"roll J4 -",  {0.0, 0.0, 0.0, -1.5, 0.0, 0.0}},
    {"pitch J5 +", {0.0, 0.0, 0.0, 0.0, 0.8, 0.0}},
    {"pitch J5 -", {0.0, 0.0, 0.0, 0.0, -0.8, 0.0}},
    {"roll J6 +",  {0.0, 0.0, 0.0, 0.0, 0.0, 1.5}},
    {"roll J6 -",  {0.0, 0.0, 0.0, 0.0, 0.0, -1.5}},
  });

  // 4. Several combined poses
  demo.sequence("Combined poses", {
    {"pose A", {0.6, 0.3, -0.4, 0.8, 0.4, 0.8}},
    {"pose B", {-0.6, 0.3, -0.4, -0.8, -0.4, -0.8}},
    {"pose C", {0.0, -0.3, -0.6, 0.0, 0.5, 0.0}},
    {"pose D", {1.0, 0.5, -0.3, -1.0, 0.3, 1.0}},
  });

  // 5. Back at home: close and open the fingers twice
  RCLCPP_INFO(node->get_logger(), "=== Gripper ===");
  for (int i = 0; i < 2; ++i) {
    demo.named(gripper, GRIPPER_CLOSED);
    demo.pause(500);
    demo.named(gripper, GRIPPER_OPEN);
    demo.pause(500);
  }

  if (demo.failures == 0) {
    RCLCPP_INFO(node->get_logger(), "Demo finished with no skipped steps.");
  } else {
    RCLCPP_WARN(node->get_logger(), "Demo finished; %d step(s) were skipped.", demo.failures);
  }

  rclcpp::shutdown();
  spinner.join();
  return 0;
}