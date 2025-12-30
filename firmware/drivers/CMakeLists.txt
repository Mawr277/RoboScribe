idf_component_register(SRCS "MPU6050/MPU6050.c" "servo/servo.c"
                    INCLUDE_DIRS "MPU6050/" "servo/"
                    REQUIRES driver freertos)
