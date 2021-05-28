# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "gen_signal.c" "main.c" "i2c_dac.c" "config_timers.c" "wifi_mqtt_interface.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
