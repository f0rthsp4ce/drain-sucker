idf_component_register(SRCS "wifi.c" "algorithm.c" "tp_interrupt_main.c" "pump.c" "water_detector.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver esp_event esp_wifi)
