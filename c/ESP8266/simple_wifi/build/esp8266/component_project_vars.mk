# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(IDF_PATH)/components/esp8266/include $(IDF_PATH)/components/esp8266/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/esp8266 -lesp8266 -L$(IDF_PATH)/components/esp8266/lib -lgcc -lhal -lcore_dbg -lnet80211_dbg -lphy -lpp_dbg -lsmartconfig -lssc -lwpa_dbg -lespnow_dbg -lwps_dbg -L $(IDF_PATH)/components/esp8266/ld -T esp8266_out.ld -T esp8266_common_out.ld -Wl,--no-check-sections -u call_user_start -T esp8266.rom.ld -T esp8266.peripherals.ld
COMPONENT_LINKER_DEPS += $(IDF_PATH)/components/esp8266/lib/libgcc.a $(IDF_PATH)/components/esp8266/lib/libhal.a $(IDF_PATH)/components/esp8266/lib/libcore_dbg.a $(IDF_PATH)/components/esp8266/lib/libnet80211_dbg.a $(IDF_PATH)/components/esp8266/lib/libphy.a $(IDF_PATH)/components/esp8266/lib/libpp_dbg.a $(IDF_PATH)/components/esp8266/lib/libsmartconfig.a $(IDF_PATH)/components/esp8266/lib/libssc.a $(IDF_PATH)/components/esp8266/lib/libwpa_dbg.a $(IDF_PATH)/components/esp8266/lib/libespnow_dbg.a $(IDF_PATH)/components/esp8266/lib/libwps_dbg.a $(IDF_PATH)/components/esp8266/ld/esp8266.rom.ld $(IDF_PATH)/components/esp8266/ld/esp8266.peripherals.ld
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += esp8266
component-esp8266-build: 
