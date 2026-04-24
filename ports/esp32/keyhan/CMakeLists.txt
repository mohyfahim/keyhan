idf_component_register(
    SRC_DIRS "../../../src/core"  "../../../src/port/espidf" 
    INCLUDE_DIRS "../../../include/"  "../../../src/"
    PRIV_REQUIRES esp_event esp_http_client esp_netif esp-tls 
)
