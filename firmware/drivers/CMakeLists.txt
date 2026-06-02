idf_component_register(SRCS "MPU6050/MPU6050.c" "servo/servo.c" "SDcard/SDcard.c"
                    INCLUDE_DIRS "MPU6050/" "servo/" "SDcard/"
                    REQUIRES driver freertos esp_vfs_fat sdmmc_cmd)
