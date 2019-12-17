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
![](http://www.plantuml.com/plantuml/png/hLFH2fim57ttAqBtebkLlXMAI65GzZZm0oGcTovKav0RosJittTdeurrx6a-tPnpSi-vCRcYvywFGvyazOGUXYzSonuSEvoAGX_OnpolsDUnxHL-1_SFJJssA9ngTw9d1p9HGbck_HjT51MJtdA7j3E9TApCyyZvdOdyabKrT_jiz0CSAgEJLRHZ4j5pHBRDyZDXz7rYmkXlgXiTf1apjCCf2O4oTpxjKRHADqOB-0krHDXJNDBQAznGlvPeqS0XMCwanCl5leW03zoMCoKdpGSb6ZzPm2hOEEWKUd1DpAPKmf6re-ebk6mPt07ti0ebztmMPb6izQmteLOu1kC-0VV64a0mBDa1aNSmXmefxBDN-cCU8_AEVkV671VyVxWx4A0UaCQdrGJTAqjqbrXPihG-VhZTpkVJAMErqbqFdeYgMgyK9LgQC3I8ZJSDOSkCGVLSaNZc_-XIgZVkbeixrUkR8GT2hw0bFUBV)

## Sequence diagram
![](http://www.plantuml.com/plantuml/png/hLN1Yjmm3BtxAmozj8U1deVEicrXwR7GtaenZgtT4MJiO2k1zEihjvE9Od3O3jtBEibxqjEJfNdCP1EDbrvzYfq-FEYFFJhGHts6le_g4OBdZolAEef9Hpf3ClbyVNGnWHeOWGu76qX_cpF1vOiDljz--BgUj-_7hiVCGRTXnYwxX5tr-dCC4wICCMnUU-Adp5W8X3HhxxDIfvF6W8Isnr-McA9F9rNqbbgbp2DN4PxnPKnGgiB94cpha0_CbhKSjJ4bZJ5ltzMucwwZxgFr35Z2huetb0-7W-3uETWBEaFp05bvQ91RkXAy87zEHiQH6cH7nBaPQ0bGIzwYCraQLxbLdk9_uLGDg4Yr_ussMJy1XywlRDjwrMJJMfA-nm7TZV6lql9oVrydJTntgUt_sfK732O6rz9qPpAUCufbvCfyBUkXidluUvItRxNsTcAIc73E77Cs50t75am14TIYMtAPOwYqa8NlCmy_thpdlwUddm-5nMaX-3NjKb7984kIjjGL5K-UKxmihfH_CetiTW87E84f-AfIWLFhKRWaAZLiLiCboQrjOjQkJrNUnOmhyNhB1sAaSPqC8QGNDpnaIgFRNyF6_875kTpzkmP7ydUdNmBtjxk0th4xnr1lpyPYR8tNyZkir6y0)

