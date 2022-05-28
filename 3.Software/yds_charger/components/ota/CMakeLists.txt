idf_component_register(SRCS "ota.c" "ble_helper/bluedroid_gatts.c" "ble_helper/nimble_gatts.c"
                    "ble_helper/ble_api.c"
                    INCLUDE_DIRS "include" "./ble_helper/include/"
                    PRIV_REQUIRES app_update esp_http_client esp_https_ota nvs_flash protocol_examples_common
                    # Embed the server root certificate into the final binary
                    EMBED_TXTFILES ./server_certs/ca_cert.pem
                   )