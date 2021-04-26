# Integration Service System Handle for Fast DDS

## Introduction

*System Handle* that connects any protocol supported by the [*the eProsima Integration Service*][integrationservice]
to *eProsima*'s open-source implementation of the [DDS protocol][dds], [Fast-DDS][fast].

This *System Handle* can be used for three main purposes:

* Connection between a *DDS* application and an application running over a different middleware implementation.
  This is the classic usage approach for the *Integration Service*.

* Connecting two *DDS* applications running under different Domain IDs.

* Creating a *TCP tunnel*, by means of running an *Integration Service* instance on each of the
  machines we want to establish a communication between. You can find an example about this
  in the [examples](#Examples) section.

## Build and installation process

### System Handle

To install this package, just clone this repository into a workspace already containing the *Integration Service* and build it:

```bash
$ cd <IS workspace folder>
$ git clone https://github.com/eProsima/FastDDS-SH.git src/fastdds-sh

$ colcon build --packages-up-to is-fastdds

```

If your working environment already provides with an installed version of *Fast DDS*, this one will be used.
Notice that you need a version above **2.0.0** for the *eProsima Integration Service Fast DDS System Handle* to work.

If that is not the case, *Fast DDS* will be automatically downloaded and compiled for you.
A specific version has been proved to be compatible and should be updated periodically once in a while,
when newer *Fast DDS* versions become available; right now, it is **v2.3.0**.

### Tests

If you want to build unitary and integration tests, then compile de project using `BUILD_TESTING` CMake flag:

```bash
$ colcon build --packages-up-to is-fastdds --cmake-args -DBUILD_TESTING=ON
```

## Changelog

### v1.0.0
- Updated to work with xTypes support.
- Support several Fast-DDS versions, used in Crystal, Dashing and Eloquent.

### v0.1.0

- DDS communication in both directions based on topic
- TCP tunnel support
- Integration tests

 [fast]: https://github.com/eProsima/Fast-DDS
 [integrationservice]: https://github.com/eProsima/Integration-Service
 [dds]: https://en.wikipedia.org/wiki/Data_Distribution_Service

---

<!--
    ROSIN acknowledgement from the ROSIN press kit
    @ https://github.com/rosin-project/press_kit
-->

<a href="http://rosin-project.eu">
  <img src="http://rosin-project.eu/wp-content/uploads/rosin_ack_logo_wide.png"
       alt="rosin_logo" height="45" align="left" >
</a>

Supported by ROSIN - ROS-Industrial Quality-Assured Robot Software Components.
More information: <a href="http://rosin-project.eu">rosin-project.eu</a>

<img src="http://rosin-project.eu/wp-content/uploads/rosin_eu_flag.jpg"
     alt="eu_flag" height="45" align="left" >

This project has received funding from the European Union’s Horizon 2020
research and innovation programme under grant agreement no. 732287.
