# TODO: подумати як робити. бо не має доступу до 192.168.156.116 з desktop
# як варіант збирати бінарник, заливати на HA, і оновлювати звідти.
# або custom esphome

# arduino-cli core install home-assistant-integration
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 ./bath_fan/bath_fan.ino
arduino --upload --port 192.168.156.116 --board esp8266:esp8266:nodemcuv2 --auth your_OTA_password src/bath_fan.ino
