idf_component_register(SRCS "main.c"
                    SRCS "app_spp_msg_prs.c"
                    SRCS "app_spp_msg_set.c"
                    SRCS "console_uart.c"
                    SRCS "../../common/frame.c"
                    SRCS "imu.c"
                    PRIV_REQUIRES bt nvs_flash esp_driver_uart esp_driver_i2c
                    INCLUDE_DIRS "."
                                 "../../common"
                                    )
