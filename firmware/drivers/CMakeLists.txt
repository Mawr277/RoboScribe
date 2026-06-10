idf_component_register(SRCS "MPU6050/MPU6050.c"
                            "servo/servo.c"
                            "SDcard/SDcard.c"
                            "wifiComms/file_server.c"
                            "stepperMotors/stepper.c"
                            "stepperMotors/stepperMotorEncoder.c"
                            "robot_data.h"
                       INCLUDE_DIRS . "MPU6050" "servo" "SDcard" "wifiComms" "stepperMotors"
                       REQUIRES driver freertos fatfs sdmmc esp_http_server )
