# Find all source files in a single current directory
# Save the name to DIR_led_SRCS
aux_source_directory(. DIR_ntp_SRCS)

# include_directories(../Config)

add_library(picow_ntp_client_background ${DIR_ntp_SRCS})
target_compile_definitions(picow_ntp_client_background PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picow_ntp_client_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
target_link_libraries(picow_ntp_client_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

add_library(picow_ntp_client_poll ${DIR_ntp_SRCS})
target_compile_definitions(picow_ntp_client_poll PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picow_ntp_client_poll PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(picow_ntp_client_poll
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        )

