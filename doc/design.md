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
            - `catch2` folder: contains the *fastrtps* as long as any fastrtps can be found.
    - `dds-test`: system handle for testing the `dds` system handle.
        - `src` folder: contains the test sources
        - `resources` folder: contains the *yaml* configurations for the tests.
        - `thirdparty` folder: contains the external code used and distributed with the project.
            - `catch2` folder: contains the *catch2* test framework

## Class diagram
![](http://www.plantuml.com/plantuml/png/dLHDYzim4BtxLmpRYzF68RU54Cp2BakFXVHknjAiLwmj6SquD9J-z_f32ALsUsWkaUUzUVDcnl8J2tePkZRBBZSMwhwYrIrvU3WU3lO1FXT52T-6kZNyJluVdCjGi_AcNf4M1VHYbEdHIfOb3t0kYaJ-3oGLpI8B3eSIdfszacVZR1P9AoHJBJB3lP-V6Oo_Bw2SVJFDu2dVWHiplD4K8FU1jtMKUsChPnLjGC721eHwV1R3Tz2lO2sTIp1Mm1kod4vzWYJdmEcmFxDuOvjzuB_SB7P6ZLCQv_7zrbC9udgZU2DZ-G-4Ibmb8p_uRKWghRbAQkQxW7bg30lvIY_5vhnyFD8Uwi6qcdoWWlA4Gf6eKN0cxf8oAvLTCfYuxVknn7WQ3Rs_e608i-ZJQMapT3dfXzhzef7_c0hpAKtVs55y2LTU8l3_PWe8z6bZq14pDXJN0V46z6ASVSQshL5zfB9NKkByUJxBkoVK9NXID2TKIuIhk3z5vahvOZzjEtQRN1yudo6_x3-zlZu-7eyxk8p-Mv8HACjuYrPIm8vvUgUqviq9XyGD2BIUCoU5ltXy8gldZzLl)

## Sequence diagram
![](http://www.plantuml.com/plantuml/png/hLN1YXin3BtxAmPwQG-1dfTcMIWiFHRQMod2iRMxWiGUR4rW-lMrQ2OQ3VMo3Sqbdh6UtlEpf7cex0jFfwFpWNCPChzWWVfoUGove1jR10KQVUBXsrmPJrzyYaUxyLNNzlrqE59jIMsQwL13eSFczUUSpbWgvUGmCV4yn5YTs-y7IiJaZ_JBSziTzdidqeRTlL5qrmId9tgU2ie1wEmPhUJrWw1Niky60A41UF_XzRY2pzIs2yGv-HC5u7d41beeZHchTJ-H9DYoVjhjR7HbprDr4V_A8tu8nXgo1MtzwDZeunaBKtsZZNhBniIEfEzvf715_AkqkfJE6wLfgLbj_zEkEbA2d49FqsraKlUSfGMatfVEtDWz_bckgq-jlQu4fCtLc7Ejm1bQxZLIGnGgkWssOO3b2bLu1xdxyUvZ-pq-_hmNLAF559NsUY9XkAAsjjWJYITF9PyML-HFQUnj5mn8PmI9tvvKuvmkroxfIK7CwhYaM6kJMjptoV8kPbo0boeVkKL2E6WA8rtSY5YvJE7riGWWOINFKlitD8x5Txk_4zYPjXfwo-oSqdPwTWPZjxqUC4NvyhdV)

