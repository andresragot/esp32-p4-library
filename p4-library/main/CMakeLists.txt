idf_component_register(SRCS "main.c"
                            "dsi_panel_espressif_ek79007.c"
                            INCLUDE_DIRS "."
                            REQUIRES driver
                            REQUIRES esp_lcd
                            REQUIRES esp_timer
                            
                        )