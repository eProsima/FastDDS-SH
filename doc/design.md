# *soss-dds* system handle

## Proyect layout:
- `Dockerfile`: Dockerfile with a system already configured with ROS2
- `doc` folder: contains the design and user documentation.
- `examples` folder: contains a *.yaml* file example and utils for running *soss-dds* easily.
- `packages`: colcon packages for this middleware
    - `dds`: main system handle
        - `src` folder: contains the implementation of *soss-dds* library.
        - `test` folder: contains the internal unitary tests for this module.
        - `thirdparty` folder: contains the external code used and distributed with the project.
    - `dds-test`: system handle for testing the `dds` system handle.
        - `src` folder: contains the test sources
        - `resources` folder: contains the *yaml* configurations for the tests.
        - `thirdparty` folder: contains the external code used and distributed with the project.

## Class diagram
![](http://www.plantuml.com/plantuml/png/dLHDYzim4BtxLmpRYzF68RU54Cp2BakFXVHknjAiLwmj6SquD9J-z_f32ALsUsWkaUUzUVDcnl8J2tePkZRBBZSMwhwYrIrvU3WU3lO1FXT52T-6kZNyJluVdCjGi_AcNf4M1VHYbEdHIfOb3t0kYaJ-3oGLpI8B3eSIdfszacVZR1P9AoHJBJB3lP-V6Oo_Bw2SVJFDu2dVWHiplD4K8FU1jtMKUsChPnLjGC721eHwV1R3Tz2lO2sTIp1Mm1kod4vzWYJdmEcmFxDuOvjzuB_SB7P6ZLCQv_7zrbC9udgZU2DZ-G-4Ibmb8p_uRKWghRbAQkQxW7bg30lvIY_5vhnyFD8Uwi6qcdoWWlA4Gf6eKN0cxf8oAvLTCfYuxVknn7WQ3Rs_e608i-ZJQMapT3dfXzhzef7_c0hpAKtVs55y2LTU8l3_PWe8z6bZq14pDXJN0V46z6ASVSQshL5zfB9NKkByUJxBkoVK9NXID2TKIuIhk3z5vahvOZzjEtQRN1yudo6_x3-zlZu-7eyxk8p-Mv8HACjuYrPIm8vvUgUqviq9XyGD2BIUCoU5ltXy8gldZzLl)

## Sequence diagram
![](http://www.plantuml.com/plantuml/png/jLN1Yjim4BtxAmozj8T0UbsQPQ6mz5XeRwK8HT8c0x5af74Mz-ixGsEFnyGc93INIDQyUKzFCz9BGPknQqwLTPXodV0OiYd7oxIaKenBELGjXO63raQiV_G5G_FDHd_I2zzbhBzt-nCKogpJTFlYCknddx-cU0wvG8fLY0ZOrzwNgjhjQeY0O4_mnoAjrhjTDUghDyzArbFjKdo3GvS3RmNE5eFMFPqGUghzgJQ6kOpv-6azBeS3q78sle-s0MUmRmE11ahbo6Iyd8WJp5c_R3Owkg35hbG-NEKHKmYeNPaZvwQi6MhzEH19kT5GENHbvvgudwa5DyBk4bc6chfHfDIWLlb_B2mjH9EYMrBt22tr1oPk5UwBeOrdvz3-6quBLEmUHmoI9YHcLeN9KEufKa84oheLbZ06-LX9-3foy-l3bdwlhx-V6KMq8NgX7NV432DAspBV4NlobbCpkC9_GgCFFmSNu1mCnyztAd6L3CMbwKuDcRNaac359h5utIVDEvXn0LwAlaMAD6ulAPHqTkFz25JtkOiHue4iUQHO9vChw7SBQ3vljmsvyMXT-r0mT-w6IzyOZisfvow-umfG-FKRRJLGRcCTUiVBqCNPsx1mxIyx-1AYvnU--Wi0)
