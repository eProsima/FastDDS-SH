# *soss-dds* system handle

## Class diagram
![](http://www.plantuml.com/plantuml/png/hPD1Yzim48Nl_XKYlJXjRN8B16C22s6F2_u1Hf5clIAs93Hom9JzxrNSiIh7Iq_roNxVqymRMJgZvywFGvzbpOGUXbUkPG-ExGvbgNViUr7Kx6syzWe_mFt3qunN54vTdqrK8JHHGbck_HCkovf9Rxd3i39BVAmgYgJp4qdxPUIYQYz6ty6XCZgBfWt9HCyHsJhBhuo5vmSJHh-hRdIGxva9AvoIGCZS-BHLqIhT6YtW2xH4s28kmxTNk4AVIxHaO0fMC2dnTB9_0Sa3jzMCvAJve4Jh9mjOKni7dK8Fharf7bvXlyphW7k8N7BFYSSqSOlNMQASYcaTqhsnGGWoB941aNSmPw4Kza-j70rYIpn9aFpBLGGa_RzaS210tI5FjsgR9Jb8hAfOtlpyThaSZuV3dZLATptu0EewdYSfuq1idiwRDcZhIScRQlF2u7reEHM9IImNjzb3boTq2-ePj7pSvjy0)

## Sequence diagram
![](http://www.plantuml.com/plantuml/png/hLN1Yjmm3BtxAmozj8U1deVEicrXwR7GtaenZgtT4MJiO2k1zEihjvE9Od3O3jtBEibxqjEJfNdCP1EDbrvzYfq-FEYFFJhGHts6le_g4OBdZolAEef9Hpf3ClbyVNGnWHeOWGu76qX_cpF1vOiDljz--BgUj-_7hiVCGRTXnYwxX5tr-dCC4wICCMnUU-Adp5W8X3HhxxDIfvF6W8Isnr-McA9F9rNqbbgbp2DN4PxnPKnGgiB94cpha0_CbhKSjJ4bZJ5ltzMucwwZxgFr35Z2huetb0-7W-3uETWBEaFp05bvQ91RkXAy87zEHiQH6cH7nBaPQ0bGIzwYCraQLxbLdk9_uLGDg4Yr_ussMJy1XywlRDjwrMJJMfA-nm7TZV6lql9oVrydJTntgUt_sfK732O6rz9qPpAUCufbvCfyBUkXidluUvItRxNsTcAIc73E77Cs50t75am14TIYMtAPOwYqa8NlCmy_thpdlwUddm-5nMaX-3NjKb7984kIjjGL5K-UKxmihfH_CetiTW87E84f-AfIWLFhKRWaAZLiLiCboQrjOjQkJrNUnOmhyNhB1sAaSPqC8QGNDpnaIgFRNyF6_875kTpzkmP7ydUdNmBtjxk0th4xnr1lpyPYR8tNyZkir6y0)

## Proyect layout:
- `Dockerfile`: Dockerfile with a system already configured with ROS2
- `dds`: main system handler
    - `src` folder: contains the implementation of *soss-dds* library.
    - `doc` folder: contains the design and user documentation.
    - `sample` folder: contains a *.yaml* file example and utils for running *soss-dds* easily.
    - `utils` folder: contains scripts and utilities.
- `dds-test`: system handler for testing the `dds` system handler.
    - `src` folder: contains the test sources
    - `resources` folder: contains the *yaml* configurations for the tests.
    - `thirdparty` folder: contains the external code used and distributed with the project.
        - `catch2` folder: contains the *catch2* test framework
