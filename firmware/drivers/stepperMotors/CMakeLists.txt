set(srcs)
set(include_dirs ".")

list(APPEND srcs "stepper.h" "stepper.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES driver freertos
                       WHOLE_ARCHIVE)