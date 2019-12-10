[![soss-commit](https://img.shields.io/badge/soss--commit-660078e-blue.svg)](https://github.com/osrf/soss_v2/tree/660078e9fee9fca1b3094d62dc6a22d4a1e80a38)
# soss-dds

System handle to connect [*SOSS*][soss] to *eProsima*'s open-source implementation of the
[DDS protocol][dds], [Fast-RTPS][fast].

## Installation

To install this package into a workspace already containing SOSS,
just clone this repository into the sources directory and build it:
```
$ cd <soss workspace folder>
$ git clone https://github.com/eProsima/SOSS-DDS.git src/soss-dds
$ colcon build --packages-up-to soss-dds
```

## Example - Connecting with ROS2

1. Create a [colcon workspace](https://index.ros.org/doc/ros2/Tutorials/Colcon-Tutorial/#create-a-workspace).
    ```
    $ mkdir -p soss_wp/src && cd soss_wp
    ```

2. Clone the soss project into the source subfolder.
    ```
    $ git clone https://github.com/osrf/soss_v2.git src/soss
    ```

3. Clone this project into the subfolder.
    ```
    $ git clone https://github.com/eProsima/SOSS-DDS.git src/soss-dds
    ```

    The workspace layout should look like this:
    ```
        soss_wp
        └── src
            ├── soss (repo)
            │   └── ... (other soss project subfolders)
            │   └── packages
            │       └── soss-ros2 (ROS2 system handle)
            └── soss-dds (repo)
                    ├── dds (soss-dds colcon pkg)
                    └── dds-test (soss-dds-test colcon pkg)
    ```

5. Source a colcon environment in which ROS2 has been built (soss-ros2 uses rclcpp package).
    ```
    $ source path/to/ros2/ws/install/local_setup.bash
    ```

6. In the workspace folder, execute colcon:
    ```
    $ colcon build --packages-up-to soss-dds soss-ros2
    ```

7. Source the current environment
    ```
    $ source install/local_setup.bash
    ```

## Usage

This system handle is mainly used to connect any system with the DDS protocol.
There are two communication modes of *SOSS DDS plugin`:
- Connection through *UDP* to a DDS cloud.
- Connection through *TCP* to others DDS participants.
This could be used as a tunnel between two SOSS.
See [TCP tunnel](#tcp-tunnel) for more information.

### Configuration

SOSS must be configured with a YAML file, which tells the program everything it needs to know
in order to establish the connection between two or more systems that the user wants.
An example of a YAML configuration file to connect ROS2 to DDS could be the following:

```YAML
types:
    - idl: >
        struct std_msgs__String // This name can't have any '/', as explained later on this section.
        {
            string data;
        };

systems:
    dds:
      type: dds
      participant: # Optional
        file_path: "path/to/dds_config_file.xml"
        profile_name: "participant profile name"

    ros2:
      type: ros2

routes:
    ros2_to_dds: { from: ros2, to: dds }
    dds_to_ros2: { from: dds, to: ros2 }

topics:
    hello_ros2: { type: "std_msgs/String", route: dds_to_ros2 }
    hello_dds: { type: "std_msgs/String", route: ros2_to_dds }
```

To see how general SOSS systems, users and topics are configured, please refer to [SOSS' documentation][soss].

For the DDS system handle the user must add two extra YAML maps to the configuration file as seen above,
which are `dynamic types` and `participant`:

* The `dynamic types` map tells the DDS system handle how a certain type is mapped.
This is necessary to convert the type from a *xtypes*, which is the type used inside soss,
to a *dynamic type*, which is the type internally used in DDS.
This conversion is done dynamically at runtime.
To have a guide on how dynamic types are defined in YAML files,
see the [YAML dynamic types](#yaml-dynamic-types) section.

  **The dynamic types standard does not allow certain characters in its names**.
For this reason, if a type defined in the topics section of the configuration file has in its name a `/`,
the dds system handle will map that character into two underscores.
That's why the type inside the dynamic types map is `std_msgs__String`,
while the type inside the topics section is `std_msgs/String`.
This is **something important to notice when connecting to ROS2**,
because in ROS2 most of the types have a `/` in their names.
Also, notice that **in the DDS system, the message will be published with a type name with two
underscores instead of a slash**.

* The `participant` map *(optional)* tells to the dds system handle where it can
find the configuration file for the DDS profle,
and what profile must be used from the many that can be defined in that XML.
This profile is used to set the DDS quality of services' parameters.
A guide on how this XML files are configured can be found in
[Fast-RTPS' documentation](https://fast-rtps.docs.eprosima.com/en/v1.7.2/xmlprofiles.html).
An example of an XML configuration file can be found [in this repository](dds/sample/tcp/config.xml).
Notice that this example file has two participant profiles defined in it,
one to be used in the client side and other for the server side,
so the YAML file used to configure SOSS in the server computer must change the `profile_name` in the example above
from `soss_profile_client` to `soss_profile_server`.

  The `participant` map is optional. If it is not given, the dds system handle will create a default UDP profile.

### YAML dynamic types


The IDL content is provided in the YAML file as follows:

```YAML
types:
    - idl: >
        struct name
        {
            idl_type1 member_1_name;
            idl_type2 member_2_name;
        };
```

The main 'type' for the IDL must be a struct, as xtypes are defined as structures.

The name for each type can be whatever the user wants, with the two following rules:

1. The name can not have spaces in it.
1. The name must be formed only by letters, numbers and underscores.
Remember that the system handle will map each `/` for `__`, as mentioned in the configuration section,
to allow an easy connection with ROS2 types.

Currently, the `xtypes::idl::Parser` (or xtypes) doesn't support the following IDL 4.2 characteristics:

- Any
- Interfaces
- Value Types
- CORBA related features
- Components related features
- CCM related features
- Template Modules
- Extended Data-Types
- Anonymous Types
- Annotations

For more details about IDL definition, please refer to [IDL documentation](https://www.omg.org/spec/IDL/4.2/PDF).

The following is an example of a full configuration file that uses the ROS2 nested type
[std_msgs/Header](http://docs.ros.org/melodic/api/std_msgs/html/msg/Header.html):

```YAML
types:
    - idl: >
        struct stamp
        {
            int32 sec;
            uint32 nanosec;
        };

        struct std_msgs__Header
        {
            string frame_id;
            stamp stamp;
        };

systems:
    dds:
      type: dds

    ros2:
      type: ros2

routes:
    ros2_to_dds: { from: ros2, to: dds }
    dds_to_ros2: { from: dds, to: ros2 }

topics:
    hello_dds:
      type: "std_msgs/Header"
      route: ros2_to_dds
    hello_ros2:
      type: "std_msgs/Header"
      route: dds_to_ros2
```

### TCP tunnel

Besides connecting any system to DDS, this system handle can also be used to create a TCP tunnel connecting
two SOSS instances.
That way, a user can connect two ROS2 systems through TCP,
or connect any system supported by soss with other system that is not in its LAN.

For the TCP tunnel, two instances of SOSS are going to be used, one in each of the computers that
are going to be communicated.
Each of those instances will have a system handle for the system they want to communicate in the WAN network,
and other to communicate with Fast-RTPS' DDS implementation.

You can see the YAML's configuration files related to TCP tunnel configuration in the [sample folder](dds/sample/tcp).

If we take as an example the communication between ROS2 and FIWARE, the communication scheme will look like this:

![](dds/doc/images/ROS2_TCP_tunnel.png)

### More information

- You can have a look at the [internal design](dds/doc/design.md)
- For a fast usage, you can use the [`Dockerfile`](Dockerfile)

## Changelog

### v0.1.0

- DDS communication in both directions based on topic
- TCP tunnel support
- Integration tests

 [fast]: https://github.com/eProsima/Fast-RTPS
 [soss]: https://github.com/osrf/soss_v2
 [dds]: https://en.wikipedia.org/wiki/Data_Distribution_Service

---

<!--
    ROSIN acknowledgement from the ROSIN press kit
    @ https://github.com/rosin-project/press_kit
-->

<a href="http://rosin-project.eu">
  <img src="http://rosin-project.eu/wp-content/uploads/rosin_ack_logo_wide.png"
       alt="rosin_logo" height="60" >
</a>

Supported by ROSIN - ROS-Industrial Quality-Assured Robot Software Components.
More information: <a href="http://rosin-project.eu">rosin-project.eu</a>

<img src="http://rosin-project.eu/wp-content/uploads/rosin_eu_flag.jpg"
     alt="eu_flag" height="45" align="left" >

This project has received funding from the European Union’s Horizon 2020
research and innovation programme under grant agreement no. 732287.
