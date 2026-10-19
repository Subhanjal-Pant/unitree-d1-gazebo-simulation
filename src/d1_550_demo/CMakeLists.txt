cmake_minimum_required(VERSION 3.8)
project(d1_550_demo)

set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

add_executable(pick_demo src/demo.cpp)
ament_target_dependencies(pick_demo rclcpp moveit_ros_planning_interface)

install(TARGETS pick_demo DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}/)

ament_package()