# Automatic Medicine Box 🏥

An Arduino-based automated medicine dispensing system designed to help users maintain their medication schedule with precision and ease.

## Features ✨

- **Automated Dispensing**: Scheduled medication dispensing using RTC (Real-Time Clock)
- **Manual Override**: Optional manual dispensing through touch screen interface
- **Medicine Tracking**: IR sensor-based medicine counting system
- **Interactive Display**: Touch screen interface showing:
  - Current time
  - Medicine counts per container
  - Dispensing schedules
  - Manual dispensing controls
- **Multiple Containers**: Support for 4 separate medicine containers
- **Environmental Monitoring**: Built-in humidity sensing

## Hardware Requirements 🛠️

- Arduino MEGA 2560
- RTC DS1302
- 4× Servo Motors
- 4× IR Sensors
- Nextion Touch Screen Display
- DHT11 Temperature & Humidity Sensor
- Power Supply
- Medicine Storage Containers

## Pin Configuration 📌

```
DHT11 Sensor: Pin 6
RTC Pins: 
- Data: Pin 4
- Clock: Pin 5
- Reset: Pin 2

Servo Motors:
- Servo A: Pin 13
- Servo B: Pin 12
- Servo C: Pin 11
- Servo D: Pin 10

IR Sensors:
- Sensor 1: Pin 22
- Sensor 2: Pin 24
- Sensor 3: Pin 26
- Sensor 4: Pin 28
```
Place the IR and Servo Motors accordingly. IR Sensor 1 must be paired with Servo A.. and so on.

## Customizable Parameters 🔧

Users can modify the following parameters in the code according to their needs:

1. **Dispensing Schedule** (24-hour time format.)
```cpp
Schedule schedules[MAX_SCHEDULES] = {
    {8, 0, 1, true, false},  // 8:00 AM - Container 1
    {12, 0, 2, true, false}, // 12:00 PM - Container 2
    {16, 0, 3, true, false}, // 4:00 PM - Container 3
    {20, 0, 4, true, false}  // 8:00 PM - Container 4
};
```

2. **Refresh Intervals**
```cpp
#define NEXTION_REFRESH_INTERVAL 100    // Display refresh rate (ms)
#define SCHEDULE_CHECK_INTERVAL 1000    // Schedule check frequency (ms)
```
A faster interval means a much more accurate time monitoring, but a much higher chance for lag in the system itself.

## Setup Instructions 🚀

1. Install the required libraries:
   - Servo.h
   - DHT.h
   - ThreeWire.h
   - RtcDS1302.h
   - EasyNextionLibrary.h

2. Connect the hardware components according to the pin configuration.

3. Upload the code to your Arduino MEGA.

4. Power up the system and set the initial time through the RTC.

**If you wish to modify the UI, please download Nextion Editor and open the included .HMI file.**
Don't change the ID of the following elements:

**Time Display for Schedule:**  ```timeIndicator1, timeIndicator2, timeIndicator3, timeIndicator4```

**Dispense Buttons:**  ```manual1, manual2, manual3, manual4```

**Humidity Display:**  ```humid```

**Time Display Indicator:** ```datecator```

**Medicine Count:** ```mA1, mA2, mA3, mA4```

## Usage 📱

1. **Adding Medicine**:
   - Open the container lid
   - Insert medicines one by one
   - IR sensors will automatically count the medicines

2. **Scheduling**:
   - Default schedules are pre-programmed
   - Modify the schedules array in the code for different timing

3. **Manual Dispensing**:
   - Use the touch screen interface
   - Press the manual dispense button for the desired container

4. **Monitoring**:
   - View medicine counts on the display
   - Check upcoming dispensing times
   - Monitor humidity
