# hivecom

> **Projects aim**: Automatically monitor a beehive to assist a beekeeper in interpreting the bees health and current state in short and long terms. 


**Starting situation**: As I got a lot into electronics and making in the high school, my teacher provided me with the possiblity to do an extra learning project to contribute to my high school diploma 2021. And we do beekeeping as an extracurricular at our high school!

**Why am I doing this**: I really like electronics, understanding systems and using / creating them. While playing around with Raspberry Pis / Arduinos and sensor kits, my head filled with ideas. My grandma was a beekeeper so I always had a good connection to bees. My teacher actually had the idea of monitoring the bees at the school.

**My ambition**: Besides getting into the material myself and trying out different protocols, techniques and parts, I want to have a nice, elegant solution in the end.

**Roadmap**: The project / making part is divided into two parts as I built two mostly completely different systems for monitoring.

**This repository**: Summary / loose documentation of the project to store experiences into.

**Credits**: Many thanks to my kind teacher for supporting me, not only for making this project even possible, but also for the anwering of all kinds of questions regarding beekeeping and funding this project.

Also thanks to my tutor at the Schülerforschungszentrum Hamburg for supporting / aiding me building and testing the weight scale and the power supply.

# First solution (called Honey Pi) 

<img src="./img/HoneyPi-Fertig.jpg">

<img src="./img/IMG_20210206_154843.jpg">

<img src="./img/image2.png">
(screenshot from the high school diploma documentation)

<img src="./img/image4.png">
(screenshot from the high school diploma documentation)

<img src="./img/Old-scale-build.png" width="50%">

#### Online service

<img src="./img/image.png">
Thingspeak IoT software solution from Mathworks

<img src="./img/image3.png">
Own website utilizing Thingspeak API for more beautiful graph display

<br>

## Experiences

This first solution took around one year to build. I submitted this to my high school exam and got 15/15 points. Yeah!

## What are the systems components

Based on [honey-pi.de](honey-pi.de), I combined the following parts:
1. Small car battery, solar panel, charging module
2. Raspberry Pi Zero (not Zero W) mini linux computer, containing prebuilt Python agents for data upload
3. Sensors:
    - 1x DHT22 sensor for temperature & humidity inside the beehive (custom case for bee protection)
    - 1x DHT22 sensor for temperature & humideity outside
    - 1x Bosche H30a weight cell + HX711 weightcell amplifier for continuously weighting the whole beehive
    - 1x voltage divider for checking battery voltage (of around 12V)
4. Uploading via USB-Surfstick with sim card over mobile radio
5. Using the Thingspeak webservice from Mathworks
    1. Stores data for one year for free
    2. Provides nice Rest API for communication between Raspberry Pi & Thingspeak
    3. Modular dashboard vizualization of the data on graphs & widgets
6. Own website that gatheres the data from Thingspeak and displays it in a prettier way

**Advantages of the system**:
- completely autonomous, if you ignore mobile radio connection strength and if you choose a free mobile phone contract
- beefy power supply & a lot of processing power
- Convenient webservice for configuration

**Disadvantages of the system**:
- Linux & so much processing power on an IoT device is overkill (I do not need a lot of data analysis onboard)
- I did not really understand, what those Python scripts really do and I did not trust them
- Shaky beehive position, as it was placed on one small weight cell
- Raspberry Pi always fully on (ca. 150mA at 5V (0.75W)), no sleep function
- Dependancy on ThingSpeak online service with its one year data storage
- It's not really my solution xD

Especially in my implementation:
- Bad connection to sensors through utilizing prototyping breadboards as permanent circuit
- Somehow the system didn't worked really long and I was annoyed and didn't know what to do
- DHT22 sensor inside the beehive was broken as formic acid for bee treatment destroyed it. Also, in beforehand it was always wet because of condensation water in the top of the hive.

**Other experiences building the project**
- Difficulties with high temperature-dependancy in weight
- the beefy car battery still ran out


# Second solution (called HiveCom)

After my high school exam, the Honey-Pi solution didn't work properly after a short period of time, so I decided to start a completely new project. I didn't really knew why exactly I spent so much time/energy on it, but somehow I wanted to end this project recently and get it finally working, so now its 2024. I spent a lot of my free time between the study semesters (5th / 6th) of computer science & engineering on this and I am proud that it finally is done!

**The live data is publicly available here**:
[hivecom.neozeo.de](hivecom.neozeo.de)


In this solution, I did everything by myself:
1. IoT device at the bee hive
2. Lora/Wifi Gateway device
3. Self-designed LoRa "protocol"
4. Self-setup docker environment on virtual server with:
    1. Influx data base
    2. Grafana dashboard vizualization


**The system consists of three big parts**:

## 1. IoT node at the beehive

<img src="./img/IoT-Node.jpg" width="45%">
<img src="./img/IoT-Node2.png" width="30%">

1. Powered by manually rechargable LiPo batteries
2. ESP32 as microcontroller for sensor data handling & upload
3. Sensors
    - DHT22 for outside temperature & humidity measurement
    - DS18B20 (watertight) for inside temperature measurement
    - 4x smaler Bosche H10a weight cells plus 4x HX711 weight cell amplifier chips
4. ATtiny84 as second microcontroller for HX711 management
    (The ESP32 has not enough pins to connect all sensors, so I included an Attiny as an I2C-Slave Chip that connects the HX711 chips to the ESP32)
5. Periodic data upload over LoRa

For the schematic, see "IoT Node Schematic.pdf" file in this repository

Features:
- Maybe half year of battery power supply by using deep sleep mode
- Debug mode for checking sensor values at the hive
- Changing upload interval length via debug mode button (1min, 5min, 20min, 60min, 120min, 360min)
- Easy access to components & see-through case for extra fancyness


## 2. LoRa/WiFi gateway

<img src="./img/Gateway.jpg" width="40%">

Small device in the school that receives the LoRa Message broadcasted from the IoT node. It is connected to the school WiFi and uploads the bee hive & maintenance data to my Influx DB.

- Left-button function: See last uploaded sensor values
- Middle-button function: View upload log or maybe errors
- Right-button function: RICK ROLL video! (I didn't know what to do with the third button that I've soldered on before I had a plan)

## 3. Self-designed LoRa "Protocol"

LoRa is a nice technology by Semtech that enables long range, low energy data transfer of small payloads. In the Maker-World, there exists a lot of boards that contain LoRa-chips. I chose an ESP32-microcontroller-board with a LoRa chip and also battery charging capability. The ESP32 is a powerful chip with versatile features, perfect for IoT-devices.

However, LoRa does not have a builtin security and you broadcast it to everyone. As I want to see only the data from the Iot-Node, I also send a hash of the payload + password as a signature.





First, the IoT node gatheres all the sensor data and then builds the packa





I also could have chosen to just use the standard LoRaWAN protocol, but then I also would have needed to spend 100 euros or so on an open LoRaWAN gateway. I had two LoRa-capable devices laying around already so I chose using them.


## 4. Web solution

<img src="./img/Grafana-Solution.png" width="80%">

View the live hive data at: [hivecom.neozeo.de](hivecom.neozeo.de)!

Webservice architecture:
- 1-blu.de virtual linux server (Ubuntu 20 LTS)
- Docker:
    - Portainer for docker management
    - Nginx reverse proxy for subdomain management
    - Influx DB for storing hive data (and maybe trigger webhooks for notifications in the future)
    - Grafana for displaying data from Influx DB

To get this project finally done, I had to do a lot of smaller projects that somehow converged to the final system. I didn't plan these steps, but at the end it somehow came all together.

## Road map side-projects (not in order)

1. Testing & playing around with Lilygo ESP32 board

<img src="./img/Liligo-Pinout.png" width="50%">

2. Think about how to get around with weight calibrating
3. Buying (too cheap) weight cells & analyze them (3D-print, attach to wooden board, test)
4. Calibrating weight cells & checking temperature dependancy

<img src="./img/Weight-Calibrate.jpg" width="40%">

<img src="./img/Weight-Calibrate2.png" width="30%">

<img src="./img/Weight-Build.jpg" width="30%">
<img src="./img/Weight-Build2.jpg" width="30%">

Putting my speaker on top of the scale for final calibrating

5. Buying more parts
6. Choosing Temperature & Humidity sensors

7. Building the weight scale
    1. Laser-Cutting the distance parts
    2. Cutting of the screws heads

<img src="./img/Lasercut.jpg" width=50%>

8. LoRa-Interfacing & Developing analysis scripts (Echo, Bluetooth to LoRa terminal)
9. LoRa range limit test at spreading factor 12
10. Designing the LoRa protocol & hardware accelerated hashing
11. Power supply & recharging
12. ATtiny as additional slave IO device & Programming ATtiny
13. Get ATtiny-ESP32 I2C connection working
14. Soldering the circuit & finding a small, some how "waterproof case"

<img src="./img/Soldering.jpg" width="50%">

15. Getting into the schools enterprise WPA2 wifi with a microcontroller
16. Programming UI on IoT node and gateway
17. Designing a case for the gateway (3D-printed, Fusion 360)

<img src="./img/Gateway-Device-Case-Plan.jpeg" width="50%">

<img src="./img/Gateway-Case.png" width="40%">

<img src="./img/Gateway-Case-Print.jpg" width="30%">

18.  Getting rick roll video onto gateway device

<img src="./img/Gateway-Rickroll.png" width="40%">

19. Approximate battery status from battery voltage
20. Play around with Arduino InfluxDB library
21. Configure linux server docker environment & configure Nginx to web-access the services
22. Play around with InfluxDB & Grafana
23. Raspberry Pi Home Weather Station Test with Sense Hat & Test Run
24. Configure API keys & database for data upload
25. Configure Grafana Dashboard
26. Test run for a few days & continuosly analyzing
27. Testing wrong signature LoRa messages

ToDo:
1. Insert images into grafana dashboard 
2. configure automatical whatsapp notification on hive alarms
3. Documenting the whole f***ing project

Install it!


# Analysis



# Failed experiments


## Using cheap weight cells

<img src="./img/cheap-weight-cells.png">

I tried using cheap weight cells (ca. 13 euros sum) from Amazon, but the values had a difference of ~130g from the real weight. Also, for every measurement, the values differed from the previous values a lot and I didn't got a clear outcome. I wanted more precise values.

## Designing a custom DHT22 sensor housing for the inside of the hive

<img src="./img/dht22-cases.png">

<img src="./img/dht22 inside.png" width=20%>

I've heard that bees close holes of specific sizes in the beehive to prevent other insects to getting inside the hive. For this, they use their self produced propolis, that is also antibacterial and protects from diseases. Research in the internet did not reveal a specific hole size that gets closed, however it should be between 1-6mm. I designed different cases for the DHT22 to test that out.

However it didn't matter because of the humidity always beeing wet and also broke
¯\\\_(ツ)_/¯

## Getting good temperature-weight-calibration

<img src="./img/temperature-experiment.png" width=80%>

Probably a little embarrassing, as I tried to measure the temperature-dependancy with a hair dryer in a verrry DIY way. I tried to put one DHT22 directly next to weight scale between the wooden boards, but it worked as well as it looked xD. I put the books around the weight scale to prevent the warm air going fast outside the scale again.

Here is some data I collected in this experiment:

<img src="./img/temperature-dependancy.png" width=70%>

However, sometimes the upload process failed and I didn't know, what to do exactly with this data. Having a low-temperature oven or some 