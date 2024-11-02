#include <Servo.h>
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "EasyNextionLibrary.h"

#define DHT11_PIN 6
#define NEXTION_REFRESH_INTERVAL 100
#define MAX_SCHEDULES 4
#define SCHEDULE_CHECK_INTERVAL 1000

struct Schedule
{
    uint8_t hour;
    uint8_t minute;
    uint8_t containerNum;
    bool enabled;
    bool dispensed;
};

DHT dht11(DHT11_PIN, DHT11);
ThreeWire myWire(4, 5, 2);
RtcDS1302<ThreeWire> Rtc(myWire);
Servo servoA, servoB, servoC, servoD;
EasyNex myNex(Serial2);

unsigned long lastRefreshTime = 0;
unsigned long lastServoMoveTime = 0;
unsigned long lastScheduleCheck = 0;
bool isServoMoving = false;

int medicineCount1 = -1;
int medicineCount2 = -1;
int medicineCount3 = -1;
int medicineCount4 = -1;

const int irSensor1 = 22;
const int irSensor2 = 24;
const int irSensor3 = 26;
const int irSensor4 = 28;

bool prevIrState1 = false;
bool prevIrState2 = false;
bool prevIrState3 = false;
bool prevIrState4 = false;

Schedule schedules[MAX_SCHEDULES] = {
    {8, 0, 1, true, false},
    {12, 0, 2, true, false},
    {16, 0, 3, true, false},
    {20, 0, 4, true, false} 
};

void formatTimeAMPM(char* timeStr, uint8_t hour, uint8_t minute, uint8_t second = 255) {
    const char* period = (hour >= 12) ? "PM" : "AM";
    uint8_t displayHour = hour % 12;
    if (displayHour == 0) displayHour = 12;
    
    if (second != 255) {
        sprintf(timeStr, "%02d:%02d:%02d %s", displayHour, minute, second, period);
    } else {
        sprintf(timeStr, "%02d:%02d %s", displayHour, minute, period);
    }
}


void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600);
    myNex.begin(9600);

    dht11.begin();

    servoA.attach(13);
    servoB.attach(12);
    servoC.attach(11);
    servoD.attach(10);

    servoA.write(0);
    servoB.write(0);
    servoC.write(0);
    servoD.write(0);

    pinMode(irSensor1, INPUT);
    pinMode(irSensor2, INPUT);
    pinMode(irSensor3, INPUT);
    pinMode(irSensor4, INPUT);

    initializeRTC();

    delay(1000);

    myNex.writeNum("mA1.val", medicineCount1);
    myNex.writeNum("mA2.val", medicineCount2);
    myNex.writeNum("mA3.val", medicineCount3);
    myNex.writeNum("mA4.val", medicineCount4);
    myNex.writeNum("manual1.val", 0);
    myNex.writeNum("manual2.val", 0);
    myNex.writeNum("manual3.val", 0);
    myNex.writeNum("manual4.val", 0);

    updateTimeIndicators();

    delay(1000);

    for (int i = 0; i < MAX_SCHEDULES; i++)
    {
        char timeText[15];
        char indicatorName[20];
        formatTimeAMPM(timeText, schedules[i].hour, schedules[i].minute);
        sprintf(indicatorName, "timeIndicator%d.txt", i + 1);
        myNex.writeStr(indicatorName, timeText);
        delay(100);
    }
}

void updateTimeIndicators()
{
    for (int i = 0; i < MAX_SCHEDULES; i++)
    {
        char indicatorName[20];
        char timeText[15];

        for (int j = 0; j < MAX_SCHEDULES; j++)
        {
            if (schedules[j].containerNum == (i + 1))
            {
                sprintf(indicatorName, "timeIndicator%d", i + 1);

                if (schedules[j].dispensed)
                {
                    myNex.writeStr(indicatorName, "Dispensed");
                }
                else
                {
                    formatTimeAMPM(timeText, schedules[j].hour, schedules[j].minute);
                    myNex.writeStr(indicatorName, timeText);
                }
                break;
            }
        }
    }
}

void loop()
{
    unsigned long currentTime = millis();

    myNex.NextionListen();

    if (currentTime - lastRefreshTime >= NEXTION_REFRESH_INTERVAL)
    {
        updateServoControls();
        updateSensorData();
        lastRefreshTime = currentTime;
    }

    if (currentTime - lastScheduleCheck >= SCHEDULE_CHECK_INTERVAL)
    {
        checkSchedules();
        lastScheduleCheck = currentTime;
    }

    checkIRSensors();
}

void checkSchedules()
{
    RtcDateTime now = Rtc.GetDateTime();
    static uint8_t lastDay = 0;

    if (now.Day() != lastDay)
    {
        for (int i = 0; i < MAX_SCHEDULES; i++)
        {
            schedules[i].dispensed = false;

            char timeText[15];
            char indicatorName[20];
            formatTimeAMPM(timeText, schedules[i].hour, schedules[i].minute);
            sprintf(indicatorName, "timeIndicator%d.txt", i + 1);
            myNex.writeStr(indicatorName, timeText);
            delay(50);
        }
        lastDay = now.Day();
    }

    for (int i = 0; i < MAX_SCHEDULES; i++)
    {
        if (schedules[i].enabled && !schedules[i].dispensed &&
            now.Hour() == schedules[i].hour &&
            now.Minute() == schedules[i].minute)
        {

            int *medicineCount = getMedicineCount(schedules[i].containerNum);
            if (medicineCount != nullptr && *medicineCount > 0)
            {
                dispenseFromContainer(schedules[i].containerNum);
                schedules[i].dispensed = true;

                char indicatorName[20];
                sprintf(indicatorName, "timeIndicator%d.txt", i + 1);
                myNex.writeStr(indicatorName, "Dispensed");
                delay(50);
            }
        }
    }
}

int *getMedicineCount(int containerNum)
{
    switch (containerNum)
    {
    case 1:
        return &medicineCount1;
    case 2:
        return &medicineCount2;
    case 3:
        return &medicineCount3;
    case 4:
        return &medicineCount4;
    default:
        return nullptr;
    }
}

void dispenseFromContainer(int containerNum)
{
    switch (containerNum)
    {
    case 1:
        if (medicineCount1 > 0)
        {
            startServoMove(servoA, medicineCount1, "mA1.val", "manual1.val");
            myNex.writeStr("timeIndicator1.txt", "Dispensed");
        }
        break;
    case 2:
        if (medicineCount2 > 0)
        {
            startServoMove(servoB, medicineCount2, "mA2.val", "manual2.val");
            myNex.writeStr("timeIndicator2.txt", "Dispensed");
        }
        break;
    case 3:
        if (medicineCount3 > 0)
        {
            startServoMove(servoC, medicineCount3, "mA3.val", "manual3.val");
            myNex.writeStr("timeIndicator3.txt", "Dispensed");
        }
        break;
    case 4:
        if (medicineCount4 > 0)
        {
            startServoMove(servoD, medicineCount4, "mA4.val", "manual4.val");
            myNex.writeStr("timeIndicator4.txt", "Dispensed");
        }
        break;
    }
}

void checkIRSensors()
{
    bool irState1 = digitalRead(irSensor1);
    bool irState2 = digitalRead(irSensor2);
    bool irState3 = digitalRead(irSensor3);
    bool irState4 = digitalRead(irSensor4);

    if (irState1 && !prevIrState1)
    {
        medicineCount1++;
        myNex.writeNum("mA1.val", medicineCount1);
        Serial.print("Medicine count 1: ");
        Serial.println(medicineCount1);
    }
    prevIrState1 = irState1;

    if (irState2 && !prevIrState2)
    {
        medicineCount2++;
        myNex.writeNum("mA2.val", medicineCount2);
        Serial.print("Medicine count 2: ");
        Serial.println(medicineCount2);
    }
    prevIrState2 = irState2;

    if (irState3 && !prevIrState3)
    {
        medicineCount3++;
        myNex.writeNum("mA3.val", medicineCount3);
        Serial.print("Medicine count 3: ");
        Serial.println(medicineCount3);
    }
    prevIrState3 = irState3;

    if (irState4 && !prevIrState4)
    {
        medicineCount4++;
        myNex.writeNum("mA4.val", medicineCount4);
        Serial.print("Medicine count 4: ");
        Serial.println(medicineCount4);
    }
    prevIrState4 = irState4;
}

void updateServoControls()
{
    if (!isServoMoving)
    {
        int manual1_val = myNex.readNumber("manual1.val");
        if (manual1_val == 1 && medicineCount1 > 0)
        {
            startServoMove(servoA, medicineCount1, "mA1.val", "manual1.val");
        }

        int manual2_val = myNex.readNumber("manual2.val");
        if (manual2_val == 1 && medicineCount2 > 0)
        {
            startServoMove(servoB, medicineCount2, "mA2.val", "manual2.val");
        }

        int manual3_val = myNex.readNumber("manual3.val");
        if (manual3_val == 1 && medicineCount3 > 0)
        {
            startServoMove(servoC, medicineCount3, "mA3.val", "manual3.val");
        }

        int manual4_val = myNex.readNumber("manual4.val");
        if (manual4_val == 1 && medicineCount4 > 0)
        {
            startServoMove(servoD, medicineCount4, "mA4.val", "manual4.val");
        }
    }
    else if (millis() - lastServoMoveTime >= 1000)
    {
        resetServos();
    }
}

void startServoMove(Servo &servo, int &medicineCount, const char *medicineVar, const char *manualVar)
{
    servo.write(180);
    lastServoMoveTime = millis();
    isServoMoving = true;

    medicineCount--;
    myNex.writeNum(medicineVar, medicineCount);
    myNex.writeNum(manualVar, 0);

    Serial.print("Servo moved, updated count: ");
    Serial.println(medicineCount);
}

void resetServos()
{
    servoA.write(0);
    servoB.write(0);
    servoC.write(0);
    servoD.write(0);
    isServoMoving = false;
}

void updateSensorData()
{
    float humidity = dht11.readHumidity();
    if (!isnan(humidity))
    {
        myNex.writeNum("humid.val", (float)humidity);
    }

    RtcDateTime now = Rtc.GetDateTime();
    char timeStr[15];
    formatTimeAMPM(timeStr, now.Hour(), now.Minute(), now.Second());
    myNex.writeStr("datecator.txt", timeStr);
}


void initializeRTC()
{
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.IsDateTimeValid())
    {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time! (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
}
