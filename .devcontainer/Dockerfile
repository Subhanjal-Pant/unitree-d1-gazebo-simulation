FROM osrf/ros:humble-desktop-full

# Set environment to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create user and install everything in ONE layer to keep the image small and stable
RUN if ! id -u $USER_UID >/dev/null 2>&1; then \
        groupadd --gid $USER_GID $USERNAME && \
        useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME; \
    fi \
    && apt-get update && apt-get install -y \
        sudo \
        nano \
        vim \
        git \
        xterm \
        x11-apps \
        bash-completion \
        ros-humble-gazebo-ros-pkgs \
        ros-humble-robot-state-publisher \
        ros-humble-joint-state-publisher \
        ros-humble-joint-state-publisher-gui \
        ros-humble-moveit \
        ros-humble-ros2-control \
        ros-humble-ros2-controllers \
        ros-humble-gazebo-ros2-control \
        ros-humble-joint-state-broadcaster \
        ros-humble-xacro \
        ros-humble-moveit-ros-visualization \
        ros-humble-gripper-controllers \
        ros-humble-controller-manager \
        ros-humble-joint-trajectory-controller \
        ros-humble-topic-tools \
        python3-argcomplete \
        python3-venv \
        python3-pip \
        ros-humble-rmw-cyclonedds-cpp \
    && echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && rm -rf /var/lib/apt/lists/*

# Switch to the user
USER $USERNAME
WORKDIR /home/$USERNAME

# Add user to video group
RUN sudo usermod --append --groups video $USERNAME

# Rosdep update and sourcing
RUN rosdep update \
    && echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# Render Gazebo and RViz on the NVIDIA GPU (the Intel graphics chip is much slower)
ENV __NV_PRIME_RENDER_OFFLOAD=1
ENV __GLX_VENDOR_LIBRARY_NAME=nvidia

ENV GAZEBO_MODEL_DATABASE_URI=""