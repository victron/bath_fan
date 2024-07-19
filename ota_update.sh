# install arduino-cli
curl -fsSL https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz -o arduino-cli.tar.gz
tar -xzf arduino-cli.tar.gz
sudo mv arduino-cli /usr/local/bin/
arduino-cli version

# config init
arduino-cli config init
vi /home/vic/.arduino15/arduino-cli.yaml
# vic@HA2:~/ota_update$ cat /home/vic/.arduino15/arduino-cli.yaml
# board_manager:
#   additional_urls:
#     - http://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index
arduino-cli core search esp8266
arduino-cli core install esp8266:esp8266

# clone and update repo with code
git clone git@github.com:victron/bath_fan.git
# add secrets.h
git pull origin master

# install custom libs
arduino-cli core install home-assistant-integration
arduino-cli lib install "Adafruit BME280 Library"

# update via OTA
git pull origin master
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 ./bath_fan/bath_fan.ino --output-dir ./build
python3 ~/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/tools/espota.py --ip 192.168.156.116 --auth=OTA_password  --file ./build/bath_fan.ino.bin


