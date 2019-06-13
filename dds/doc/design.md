# *soss-dds* system handle

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

## Class diagram
![](http://www.plantuml.com/plantuml/png/hPF1QhGm58NtFaMOVwD_HvdTm20oK1YwBFW04fDR6qQJa1i7fFJTcwRHnh5qLLTwpicztut96IqrTXpw9Aadj30yKSbxC6HtA0gv8__pl2BFOziBV0NpIwOUMsH6jFUXKC4r4KneAksTNHGLuLPJWsvb4kL8cUTHvpidxfU4LAZsgEGD30ebaoMqSHBMKqIoPdbBY7iU25FoHNIZWJGZogqmWaEmr9LEMnMra8sI37wmPeIjHRdxjW9NrlkC5WqSm7B2EPvE-ji8ya1rwIq-IJe8rjX90rQXZO5Ee0NJn6xgNc5Fsb7q74ms3sw0MbY2d5hgWrc4DU_r6cgHOp6k4tIhj1ES33Cx8D8EF5IWq5-rKbI8JV0ZWkYdhX0mw7_2O821k44QxzcM9JfHf2n9Mk__NIx7u-6GaLh8hWVhZAfQ3XRdoq34JsTLuxJrf24RQswXptvgQLJ4asZAhZuJRf7hvjGpIEwlzGS0)

## Sequence diagram
![](http://www.plantuml.com/plantuml/png/hLN1Yjmm3BtxAmozj8U1deVEicrXwR7GtaenZgtT4MJiO2k1zEihjvE9Od3O3jtBEibxqjEJfNdCP1EDbrvzYfq-FEYFFJhGHts6le_g4OBdZolAEef9Hpf3ClbyVNGnWHeOWGu76qX_cpF1vOiDljz--BgUj-_7hiVCGRTXnYwxX5tr-dCC4wICCMnUU-Adp5W8X3HhxxDIfvF6W8Isnr-McA9F9rNqbbgbp2DN4PxnPKnGgiB94cpha0_CbhKSjJ4bZJ5ltzMucwwZxgFr35Z2huetb0-7W-3uETWBEaFp05bvQ91RkXAy87zEHiQH6cH7nBaPQ0bGIzwYCraQLxbLdk9_uLGDg4Yr_ussMJy1XywlRDjwrMJJMfA-nm7TZV6lql9oVrydJTntgUt_sfK732O6rz9qPpAUCufbvCfyBUkXidluUvItRxNsTcAIc73E77Cs50t75am14TIYMtAPOwYqa8NlCmy_thpdlwUddm-5nMaX-3NjKb7984kIjjGL5K-UKxmihfH_CetiTW87E84f-AfIWLFhKRWaAZLiLiCboQrjOjQkJrNUnOmhyNhB1sAaSPqC8QGNDpnaIgFRNyF6_875kTpzkmP7ydUdNmBtjxk0th4xnr1lpyPYR8tNyZkir6y0)

