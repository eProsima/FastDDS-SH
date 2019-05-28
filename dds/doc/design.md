# *soss-dds* system handle

## Class diagram
![](http://www.plantuml.com/plantuml/png/hPD1Yzim48Nl_XKYlJXjRN8B16C22s6F2_u1Hf5clIAs93Hom9JzxrNSiIh7Iq_roNxVqymRMJgZvywFGvzbpOGUXbUkPG-ExGvbgNViUr7Kx6syzWe_mFt3qunN54vTdqrK8JHHGbck_HCkovf9Rxd3i39BVAmgYgJp4qdxPUIYQYz6ty6XCZgBfWt9HCyHsJhBhuo5vmSJHh-hRdIGxva9AvoIGCZS-BHLqIhT6YtW2xH4s28kmxTNk4AVIxHaO0fMC2dnTB9_0Sa3jzMCvAJve4Jh9mjOKni7dK8Fharf7bvXlyphW7k8N7BFYSSqSOlNMQASYcaTqhsnGGWoB941aNSmPw4Kza-j70rYIpn9aFpBLGGa_RzaS210tI5FjsgR9Jb8hAfOtlpyThaSZuV3dZLATptu0EewdYSfuq1idiwRDcZhIScRQlF2u7reEHM9IImNjzb3boTq2-ePj7pSvjy0)

## Sequence diagram
TODO

## Proyect layout:
- `dds`: main system handler
    - `src` folder: contains the implementation of *soss-dds* library.
    - `doc` folder: contains the design and user documentation.
    - `sample` folder: contains a *.yaml* file example and utils for running *soss-dds* easily.
    - `utils` folder: contains scripts and utilities.
    - `dockerfiles` folder: contains dockerfiles with an already system configuration.
- `dds-test`: system handler for testing the `dds` system handler.
    - `src` folder: contains the test sources
    - `resources` folder: contains the *yaml* configurations for the tests.
    - `thirdparty` folder: contains the external code used and distributed with the project.
        - `catch2` folder: contains the *catch2* test framework
