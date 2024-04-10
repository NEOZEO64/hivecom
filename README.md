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
    - 1x DHT22 sensor for temperature & humidity inside the beehive
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

**Disadvantages of the system**:
- Linux on an IoT device is overkill (I do not need a lot of data analysis)
- I still do not really understand, what those Python scripts really do and I do not trust those
- Shaky beehive positon, as it was placed on one small weight cell
- Raspberry Pi always fully on (ca. 150mA at 5V (0.75W)), no sleep function
- It's not really my solution xD

Especially in my implementation:
- Bad connection to sensors through utilizing prototyping breadboards as permanent circuit
- Somehow the system didn't worked really long and I was annoyed and didn't know what to do
- DHT22 sensor inside the beehive was broken as formic acid for bee treatment destroyed it

**Other experiences building the project**
- 



- a lot of temperature-dependancy in the weight


## Second solution (called Hive Com)

After my high school exam, the Honey-Pi solution didn't work properly after short time, so I decided to start a completely new project. I didn't really knew why exactly I spent so much energy on it, but somehow I wanted to end this project recently and get it finally working, so now its 2024. I spent a lot of my free time between the study semesters (5th / 6th) of computer science & engineering on this and I am proud that it finally is done!

In this solution, I did all by myself:
1. IoT device with an ESP32 microcontroller & LoRa interface
2. Gateway device that accepts LoRa data & uploads it via WiFi
3. Self-hosted Influx data base
4. Self-hosted Grafana dashboard vizualization
5. Self-designed LoRa "protocol" (I wanted to save money by circumventing a LoRaWAN gateway)

IoT node placed under beehive:
1. Powered by manually rechargable LiPo batteries
2. ESP32 as microcontroller for sensor data handling
3. Sensors:
    - DHT22 for outside

This repository should contain a documentation of a beehive monitoring solution.

Tile-projects:

1. Testing & playing around with Lilygo ESP32
2. Don't know, what to do with weight calibrating
3. Calibrating weight cells & Checking weight cells against temperature dependancy
4. Buying too cheap weight cells & analyze (3D-print, attach to wooden board, test)
5. Buying more parts
6. Thinking about weight scale construction
7. Choosing Temperature & Humidity sensors
8. Building the weight scale
    1. Laser-Cutting the distance parts
    2. Cutting of the screws heads
9. LoRa-Interfacing & Developing analysis scripts (Echo, Bluetooth terminal)
10. LoRa Manual Range Limit Test
11. Designing the LoRa protocol & onboard, hardware accelerated hashing
12. Power supply & recharging
13. ATtiny as additional slave IO device & Programming ATtiny
14. ATtiny & ESP32 communication as master-slave via I2C
15. Soldering the circuit & finding a small, some how "waterproof case"
16. Getting into the schools wifi with a microcontroller
17. Programming UI on IoT node and gateway
18. Designing a case for the gateway (3D-printed, in Fusion 360)
19. Getting rick roll video onto gateway device
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
3. Documenting the whole f***ing project to remember it

Install it!


## Analysis