# Tutorial
This tutorial intend to setup an entire dds and ros2 communication environment from zero.
To archieve this, we need to deploy a `dds` world and a `ros2` world that will be connected thanks to `soss`.

## Prerrequisites
### Ros2
Any version of `crystal`, `dashing`, or `eloquent` is valid for the propose of this tutorial.

To install it, you can take a look to the
![official documentation](https://index.ros.org/doc/ros2/Installation/Eloquent/Linux-Install-Binary/).

In order to test it(supposing eloquent), please open two terminals and do the following:
- First terminal:
  ```
  . ~/ros2_eloquent/ros2-linux/setup.bash
  ros2 run demo_nodes_cpp talker
  ```
- Second terminal:
  ```
  . ~/ros2_eloquent/ros2-linux/setup.bash
  ros2 run demo_nodes_cpp listener
  ```
If the listener receive messages, congratulations, `ros2` has been installed succesuful.

### DDS
AS a dds middleware we will use the `fastrtps` implementation (the default `ros2` middleware implementation).
To install it, you can use the official documentation ![here](https://fast-rtps.docs.eprosima.com/en/latest/sources.html).

Also, we will need `fastrpts-gen` to generate our types.
To install it, clone the repo with `--recursive` option:
```
git clone https://github.com/eProsima/Fast-RTPS-Gen.git --recursive
cd Fast-RTPS-Gen
gradle assemble
```
Note that `Fast-RTPS-Gen` is a Java application. You need to have installed `Java 8` and `Gradle`.
Once you have `Fast-RTPS-Gen` installed you can execute it from the script found at `scripts/fastrptsgen`.
We strongly recommend to add this `scripts` folder to the path in order to execute the script easily from everywhere.

In order to test your dds installation you can follow the steps found
![here](https://fast-rtps.docs.eprosima.com/en/latest/introduction.html).

You need to be able to create and connect a Publisher and a Subscriber.

### SOSS
To use soss, you need to create a _colcon workspace_ with the packages you want to use.
In our purpose, we need to add to our workspace two repositories:
- `https://github.com/osrf/soss` that offers the soss core and ros2 system handle.
- `https://github.com/eProsima/SOSS-DDS` that offers the dds system handle.
Both repositories must be placed into the `src` folder of our _colcon workspace_.

Before compile it with colcon, we need to _source_ the ros2 environment as follows (supposing eloquent and linux):
```
. ~/ros2_eloquent/ros2-linux/setup.bash
```
This is needed to fetch all tools and dependencies from `ros2` in order to compile the _ros2 system handle_.
Then we can compile simply writing the following command from our colcon workspace:
```
colcon build
```

## DDS-ROS2 use case
In our use case we want communicate a ros2 world that is using the `std_msgs/String` type,
with a dds world that is using the `HelloWorld.idl` found in the _fastrtps getting started_.

Both types have a `string`, so they are compatibles.
The type name and the member name are differents but the it does not matter because soss will take care of it
(see the [xtypes QoS policies for consistency between types](https://github.com/eProsima/xtypes#type-consistency-qos-policies) for more information).

`soss` executable receives a [yaml file](https://github.com/osrf/soss/blob/master/doc/concept.md) as a configuration file for the communications.
You can see `soss` as a router that enables differents communications among several middlewares.

In this case that yaml we want is the following:
```
types:
    idls:
        - >
            struct HelloWorld
            {
                string msg;
            };
systems:
    dds: { type: dds }
    ros2: { type: ros2 }

routes:
    ros2_to_dds: { from: ros2, to: dds }
    dds_to_ros2: { from: dds, to: ros2 }

topics:
    hello_ros2: { type: "std_msgs/String", route: dds_to_ros2, remap: { dds: { type: HelloWorld } } }
    hello_dds: { type: "std_msgs/String", route: ros2_to_dds, remap: { dds: { type: HelloWorld } } }

```
