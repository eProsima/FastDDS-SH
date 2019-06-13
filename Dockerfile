FROM ubuntu:bionic

# Dependencies
RUN apt-get install -f
RUN apt-get update

RUN apt-get install -y lsb-release
RUN apt-get install -y gnupg2

ENV APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
RUN echo "deb http://packages.ros.org/ros2/ubuntu `lsb_release -sc` main" > /etc/apt/sources.list.d/ros2-latest.list
RUN apt-get update

RUN apt-get install -y libyaml-cpp-dev
RUN apt-get install -y libboost-program-options-dev
RUN apt-get install -y python3
RUN apt-get install -y python3-colcon-common-extensions

# Install ros2
RUN apt-get install -y ros-crystal-desktop
RUN apt-get install -y ros-crystal-test-msgs
RUN chmod +x ./opt/ros/crystal/setup.sh

# Prepare soss
RUN apt-get install -y git #Required for config
RUN mkdir -p root/soss_wp/src
WORKDIR /root/soss_wp/src
RUN git clone https://github.com/osrf/soss_v2.git
RUN git clone https://github.com/eProsima/SOSS-DDS.git dds
WORKDIR /root/soss_wp

# Compile soss
RUN . /opt/ros/crystal/setup.sh && \
    colcon build --packages-up-to soss-ros2-test soss-dds-test --cmake-args -DCMAKE_BUILD_TYPE=RELEASE --install-base /opt/soss

# Check compilation
RUN . /opt/ros/crystal/setup.sh && \
    colcon test --packages-up-to soss-ros2-test soss-dds-test --install-base /opt/soss

# Prepare environment
WORKDIR /root
RUN cp soss_wp/src/dds/dds/sample/udp/dds_ros2.yaml .
RUN rm -rf soss_wp

ENTRYPOINT . /opt/soss/setup.sh && echo "[NOTE] Write: 'soss dds_ros2.yaml' to test soss with ros2 and DDS." && bash
