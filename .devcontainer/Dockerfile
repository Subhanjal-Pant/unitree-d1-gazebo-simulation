FROM osrf/ros:humble-desktop-full

# Avoid interactive prompts during apt installs
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
        python3-colcon-common-extensions \
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

# Rosdep update and shell setup
RUN rosdep update \
    && echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc \
    && echo "[ -f /usr/share/gazebo/setup.sh ] && source /usr/share/gazebo/setup.sh" >> ~/.bashrc

# Stops Gazebo from trying to reach its online model database at startup
ENV GAZEBO_MODEL_DATABASE_URI=""