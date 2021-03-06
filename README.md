# LED Matrix Clock

A Raspberry Pi powered LED matrix clock

![LED matrix clock](http://i.imgur.com/qxTNKxv.jpg)

### Features
* Current time and date (via NTP)
* Weather (via [Yahoo! weather API](https://developer.yahoo.com/weather/))
* Next bus ETA (via [NextBus API](https://www.nextbus.com/))
* Network WAN bandwidth meter (via [DD-WRT SNMP](http://www.dd-wrt.com/wiki/index.php/SNMP))
* Web server for displaying scrolling text messages
* Auto adjusting LED brightness (via photoresistor sensor)


## Clock Construction
### [Album](http://imgur.com/a/v690h)

I used a 64 x 16 red LED Matrix I found on [Ebay](http://www.ebay.com/itm/141637118333).


## Raspberry Pi Setup
1. Install [Minibian](https://minibianpi.wordpress.com/). (No need for a bloated install with a GUI)
2. Use gparted or another partition editor to expand the file-system to the fill SD card size.
3. Configure WIFI

   1. Install WIFI tools: `apt install iw wpasupplicant`
   2. Edit `/etc/network/interfaces` with network settings:

      ```
      #auto eth0  
      auto wlan0  
      allow-hotplug wlan0  
      iface wlan0 inet dhcp  
      wpa-ssid "SSID"  
      wpa-psk "PASS"
      ```
4. Configure Timezone: `dpkg-reconfigure tzdata`
5. Install required C libs: `apt install libxml2-dev libcurl4-gnutls-dev wiringpi libsnmp-dev`
6. Clone this repository and matrix submodule

    ```
    git clone --recursive https://github.com/lanrat/led-clock.git
    cd led-clock
    apt-get install libcurl4-openssl-dev libxml2-dev wiringpi libsnmp-dev snmp
    ```
7. Compile: `make` (this may take over a minute the first time)
8. Install systemd init script to start on boot `make install`
9. Start the clock `make start`


## Weather Font
The weather font was built using [bdfedit](http://hea-www.harvard.edu/~fine/Tech/bdfedit.html). Each char in the font corresponds to the correct [Yahoo! weather code](https://developer.yahoo.com/weather/documentation.html#codes).
