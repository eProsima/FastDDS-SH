FROM ubuntu:bionic

# Dependencies
RUN apt-get install -f && apt-get update

RUN apt-get install -y lsb-release gnupg2

ENV APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn
ENV TZ=Europe/Stockholm
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654 && \ 
echo "deb http://packages.ros.org/ros2/ubuntu `lsb_release -sc` main" > /etc/apt/sources.list.d/ros2-latest.list && apt-get update


# Install ros2
RUN apt-get install -y git libyaml-cpp-dev libboost-dev libboost-program-options-dev python3 python3-colcon-common-extensions ros-dashing-desktop ros-dashing-test-msgs
RUN chmod +x ./opt/ros/dashing/setup.sh

# Prepare soss
RUN mkdir -p root/soss_wp/src
WORKDIR /root/soss_wp/src
RUN git clone --recursive --branch feature/xtypes-dds https://github.com/eProsima/soss_v2.git soss
RUN git clone --recursive --branch doc/examples https://github.com/eProsima/SOSS-DDS.git soss-dds 

WORKDIR /root/soss_wp

# Compile soss
RUN . /opt/ros/dashing/setup.sh && \
    colcon build --packages-up-to soss-ros2-test soss-dds-test --cmake-args -DCMAKE_BUILD_TYPE=RELEASE --install-base /opt/soss

# Check compilation
RUN . /opt/ros/dashing/setup.sh && \
    colcon test --packages-up-to soss-ros2-test soss-dds-test --install-base /opt/soss

# Prepare environment
WORKDIR /root
RUN cp soss_wp/src/dds/dds/sample/udp/dds_ros2.yaml .
RUN rm -rf soss_wp

ENTRYPOINT . /opt/soss/setup.sh && echo "[NOTE] Write: 'soss dds_ros2.yaml' to test soss with ros2 and DDS." && bash
