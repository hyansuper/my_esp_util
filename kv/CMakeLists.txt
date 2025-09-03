idf_component_register(SRCS "kv.c"
                       INCLUDE_DIRS "."
                       REQUIRES nvs_flash)

add_compile_definitions(APP_NAME="${CMAKE_PROJECT_NAME}")