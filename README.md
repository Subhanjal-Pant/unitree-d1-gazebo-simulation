# Unitree D1-550 arm: Gazebo + MoveIt 2 (ROS 2 Humble)

Simulation of the Unitree D1-550 6-DOF robot arm with its 2-finger gripper in Gazebo Classic,
planned and controlled with MoveIt 2 on ROS 2 Humble. Includes a scripted demo.

## Packages (`src/`)

| Package | What it does |
|---|---|
| `d1_550_description` | Arm URDF and meshes (see Credits) |
| `d1_550_bringup` | Gazebo launch, `ros2_control` setup, controller config |
| `d1_550_moveit_config` | MoveIt 2 configuration, plus `full_sim.launch.py` which starts everything |
| `d1_550_demo` | C++ demo: sweeps, up/down, wrist rotation, combined poses, gripper open/close |

## Quick start (Linux + Docker + VS Code)

Requirements: Docker, VS Code with the **Dev Containers** extension.
An NVIDIA GPU is optional (see below).

1. On your host, allow the container to open windows (once per login):
```
   xhost +local:
```
2. Clone this repo and open the folder in VS Code.
3. Run **Dev Containers: Reopen in Container**. Choose the default config, or the
   **NVIDIA GPU** config if you have an NVIDIA GPU and the NVIDIA container toolkit.
4. Wait for the first build to finish (this can take some time for the first build).
5. In a terminal inside the container:
```
   source install/setup.bash
   ros2 launch d1_550_moveit_config full_sim.launch.py
```
   Gazebo opens first. RViz with MoveIt opens some time later.
6. When `ros2 control list_controllers` shows three active controllers, run the demo
   in a second terminal:
```
   source install/setup.bash
   ros2 launch d1_550_demo demo.launch.py
```

You can also use the **MotionPlanning** panel in RViz: choose group `arm`, drag the
marker, then press **Plan & Execute**.

## What the demo does

Left-right sweep, up and down, wrist rotations, several combined poses, returning
`home` after each stage, then closes and opens the gripper twice.

## Troubleshooting

- **No windows appear:** run `xhost +local:` on the host, then restart the launch.
- **Arm missing or flat dark grey in Gazebo:** stop everything and relaunch:
```
  pkill -9 -f "gzserver|gzclient"
```
- In Gazebo, `ground_plane` and `d1_550` should be listed under Models, and Lights should contain `sun`.

- **Arm flickers, or you see `TF_OLD_DATA` warnings:** another ROS 2 system on the same network can interfere. The container sets `ROS_LOCALHOST_ONLY=1` to prevent this.
  
- In a native (non-Docker) install, run `export ROS_LOCALHOST_ONLY=1` before launching.
  
- **Slow first start:** Gazebo can take a minute or two on the first launch.

## Known limitations

- Link masses, inertia, etc in the URDF are assummed to be correct and not verified
- Tested on Ubuntu with Docker and VS Code, including an NVIDIA GPU laptop.
  Other platforms (Windows/WSL2, macOS) have not been tested.

## Credits

The arm URDF and meshes in `src/d1_550_description` come from
https://github.com/JeewanthaSadaruwan/unitree-D1-550-Robot-ARM, which refers to
Unitree Robotics' D1-550 files. 
Please check the original repository and Unitree for their license terms.

Author: Subhanjal Pant