idf_component_register(SRCS "mjson/src/mjson.c" "ext_mjson.c"
                       INCLUDE_DIRS "mjson/src" ".")

add_compile_definitions(MJSON_ENABLE_PRINT=0 
                        MJSON_ENABLE_RPC=0 
                        MJSON_ENABLE_BASE64=0
                        MJSON_ENABLE_MERGE=0
                        MJSON_ENABLE_PRETTY=0
                        MJSON_ENABLE_NEXT=1)