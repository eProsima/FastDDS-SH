# soss-dds
System handle to connect dds to *soss*.

## Installation
0. Prerequisites: fastrtps installed
1. Create a colcon workspace
2. Clone the soss project into the colcon workspace (tag: `eprosima_usage`)
3. Clone the soss-dds plugin into the colcon workspace (with `--recursive` option).
For example:
```
soss_wp
└── src
    ├── osrf
    │   └── soss
    │       └── ... (soss project subfolders)
    └── eprosima
        └── soss
            └── dds (repo)
                ├── dds (soss-dds colcon pkg)
                └── dds-test (soss-dds-test colcon pkg)
```
4. Execute colcon: `colcon build --packages-up-to soss-dds`
5. source soss environment: `source install/local_setup.bash`

## Run soss (with ros2)
0. Source the ros2 environment and compile with `--packages-up-to soss-ros2-test`
1. Run soss (with the sample configuration): `soss sample/udp/hello_dds_ros2.yaml`

- Also, you can have a look to the [internal design](dds/doc/design.md)

## Changelog
### v0.1.0
- DDS communication in both directions based on topic
- TCP tunnel support
- Integration tests
