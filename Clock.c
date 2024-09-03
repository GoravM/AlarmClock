#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <pcf8574.h>
#include <lcd.h>
#include <time.h>
#include <signal.h>
#include <string.h>

// LED wPi pins
#define GREEN 24
#define RED 25

void init_LED(){
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
}


// Buzzer wPi pin
#define Buzzer 29

void init_Buzzer(){
	pinMode(Buzzer, OUTPUT);
}


// Keypadd wPi pins
#define ROWS 4
#define COLS 4

char pressedKey = '\0';
int rowPins[ROWS] = {1, 4, 5, 6};
int colPins[COLS] = {12, 3, 2, 0};

char keys[ROWS][COLS] = {
   {'1', '2', '3', 'A'},
   {'4', '5', '6', 'B'},
   {'7', '8', '9', 'C'},
   {'*', '0', '#', 'D'}
};

// setup keypad pins
void init_keypad(){
    for (int c = 0; c < COLS; c++){
        pinMode(colPins[c], OUTPUT);   
        digitalWrite(colPins[c], HIGH);
    }
    for (int r = 0; r < ROWS; r++){
        pinMode(rowPins[r], INPUT);   
        pullUpDnControl(rowPins[r], PUD_UP);
    }
}

// find row
int findRow(){
    for (int r = 0; r < ROWS; r++){
        if (digitalRead(rowPins[r]) == LOW){
            return r;
        }
    }
    return -1;
}

// find col and row and then output using 2d array
char get_key(){
    int rowIndex;

    for (int c = 0; c < COLS; c++){
        digitalWrite(colPins[c], LOW);
        rowIndex = findRow();
        if (rowIndex > -1){
            if (!pressedKey)
                pressedKey = keys[rowIndex][c];
            return pressedKey;
        }
        digitalWrite(colPins[c], HIGH);
    }

    pressedKey = '\0'; // null if nothing is pressed
    return pressedKey;
}


// LCD screen Setup
int pcf8574_address = 0x27;
#define BASE 64         // BASE any number above 64
// Define the output pins of the PCF8574, which are directly connected to the LCD1602 pin.
#define RS      BASE+0
#define RW      BASE+1
#define EN      BASE+2
#define LED     BASE+3
#define D4      BASE+4
#define D5      BASE+5
#define D6      BASE+6
#define D7      BASE+7
int lcdhd;// used to handle LCD

int detectI2C(int addr){
    int _fd = wiringPiI2CSetup (addr);   
    if (_fd < 0){		
        printf("Error address : 0x%x \n",addr);
        return 0;
    } 
    else{	
        if(wiringPiI2CWrite(_fd,0) < 0){
            printf("Not found device in address 0x%x \n",addr);
            return 0;
        }
        else{
            return 1;
        }
    }
}

void init_LCD(){
    if(detectI2C(0x27)){
        pcf8574_address = 0x27;
    }else{
        printf("Incorrect I2C address\n");
    }

    pcf8574Setup(BASE,pcf8574_address);//initialize PCF8574
    for(int i = 0; i < 8; i++){
        pinMode(BASE+i,OUTPUT);     //set PCF8574 port to output mode
    }

    digitalWrite(LED,HIGH);     //turn on LCD backlight
    digitalWrite(RW,LOW);       //allow writing to LCD
	lcdhd = lcdInit(2,16,4,RS,EN,D4,D5,D6,D7,0,0,0,0);// initialize LCD 
    if(lcdhd == -1){
        printf("lcdInit failed");
    }
}

void cleanLCD(){
    lcdPosition(lcdhd,0,0);
    lcdPrintf(lcdhd,"                ");
    lcdPosition(lcdhd,0,1);
    lcdPrintf(lcdhd,"                ");
}


// set alarm function
int set_hour = -1;
int set_minute = -1;
int set_second = -1;

int set_alarm(){
	cleanLCD();
    int cont = 1;
    int count = 2;
    char hour[2] = "\0";
    char min[2] = "\0";
    char sec[2] = "\0";
    char x = '\0';

    while (cont){
        // set alarm hour
        while (cont){
			delay(150);
            lcdPosition(lcdhd,0,0);
            lcdPrintf(lcdhd,"Set Hour:");
            lcdPosition(lcdhd,10,0);
            lcdPrintf(lcdhd,"%s", hour);
            lcdPosition(lcdhd,0,1);
            lcdPrintf(lcdhd,"Back 'D'");

            // get input
            x = get_key();
            if (x == 'A' || x == 'B' || x == 'C' || x == '*' || x == '#'){
                // dont accept input
            }

            // break out of loop and cancel everything
            else if (get_key() == 'D'){
				cleanLCD();
                set_hour = -1;
                cont = 0;
                break;
            }

            // made 2 inputs and sets min finally
            else if (count == 0){
				cleanLCD();
                set_hour = atoi(hour);
                set_hour = set_hour % 60;
                count = 2;
                break;
            }
            
            // append input to a string
            else if (x){
                count--;
                int lenh = strlen(hour);
                hour[lenh] = x;
                x = '\0';
            }

            else {
                // do nothing
            }
        }
        // set alarm minute
        while (cont){
			delay(150);
            lcdPosition(lcdhd,0,0);
            lcdPrintf(lcdhd,"Set Min:");
            lcdPosition(lcdhd,10,0);
            lcdPrintf(lcdhd,"%s", min);
            lcdPosition(lcdhd,0,1);
            lcdPrintf(lcdhd,"Back 'D'");

            // get input
            x = get_key();
            if (x == 'A' || x == 'B' || x == 'C' || x == '*' || x == '#'){
                // dont accept input
            }

            // break out of loop and cancel everything
            else if (get_key() == 'D'){
				cleanLCD();
                set_hour = -1;
                cont = 0;
                break;
            }

            // made 2 inputs and sets min finally
            else if (count == 0){
				cleanLCD();
                set_minute = atoi(min);
                set_minute = set_minute % 60;
                count = 2;
                break;
            }
            
            // append input to a string
            else if (x){
                count--;
                int lenm = strlen(min);
                min[lenm] = x;
                x = '\0';
            }

            else {
                // do nothing
            }
        }
        // set alarm second
        while (cont){
			delay(150);
            lcdPosition(lcdhd,0,0);
            lcdPrintf(lcdhd,"Set Sec:");
            lcdPosition(lcdhd,10,0);
            lcdPrintf(lcdhd,"%s", sec);
            lcdPosition(lcdhd,0,1);
            lcdPrintf(lcdhd,"Back 'D'");

            // get input
            x = get_key();
            if (x == 'A' || x == 'B' || x == 'C' || x == '*' || x == '#'){
                // dont accept input
            }

            // break out of loop and cancel everything
            else if (get_key() == 'D'){
				cleanLCD();
                set_hour = -1;
                set_minute = -1;
                cont = 0;
                break;
            }

            // made 2 inputs and sets seconds finally
            else if (count == 0){
				cleanLCD();
                set_second = atoi(sec);
                set_second = set_second % 60;
                count = 2;
                return 1;
            }
            
            // append input to a string
            else if (x){
                count--;
                int lens = strlen(sec);
                sec[lens] = x;
                x = '\0';
            }

            else {
                // do nothing
            }
        }
    }
    set_hour = -1;
    set_minute = -1;
    set_second = -1;
    return 0;
}

// check alarm function and play alarm if this is true
void check_alarm(){
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // if set alarm time == curr time turn on the alarm
    if (set_hour == timeinfo->tm_hour && set_minute == timeinfo->tm_min && set_second == timeinfo->tm_sec){
		cleanLCD();
        lcdPosition(lcdhd,0,0);
        lcdPrintf(lcdhd,"Press '#'");
        while(get_key() != '#'){
            digitalWrite(Buzzer, HIGH);
            digitalWrite(GREEN, HIGH);
            delay(500);
            digitalWrite(GREEN, LOW);
            digitalWrite(Buzzer, LOW);
            digitalWrite(RED, HIGH);
            delay(500);
            digitalWrite(RED, LOW);
        }
        // alarm is off
        digitalWrite(GREEN, LOW);
        digitalWrite(RED, LOW);
        digitalWrite(Buzzer, HIGH);
        cleanLCD();
    }
}

int play_CPU = 1;

void printCPUTemperature(){// sub function used to print CPU temperature
    FILE *fp;
    char str_temp[15];
    float CPU_temp;
    // CPU temperature data is stored in this directory.
    
    while (play_CPU){
		fp=fopen("/sys/class/thermal/thermal_zone0/temp","r");
        cleanLCD();
        fgets(str_temp,15,fp);      // read file temp
        CPU_temp = atof(str_temp)/1000.0;   // convert to Celsius degrees
        lcdPosition(lcdhd,0,0);
        lcdPrintf(lcdhd,"CPU:%.2fC",CPU_temp);// Display CPU temperature on LCD
        lcdPosition(lcdhd,0,1);
        lcdPrintf(lcdhd, "Back 'D'");
        if (get_key() == 'D'){
            play_CPU = 0;
        }
        check_alarm();
        delay(100);
		fclose(fp);
    }
    play_CPU = 1;
    cleanLCD();
}

int play_TIME = 1;

void printDataTime(){//used to print system time 
    time_t rawtime;
    struct tm *timeinfo;
    
    while (play_TIME){
		time(&rawtime);// get system time
		timeinfo = localtime(&rawtime);//convert to local time
        cleanLCD();
        lcdPosition(lcdhd,0,0);
        lcdPrintf(lcdhd,"Time:%02d:%02d:%02d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec); //Display system time on LCD
        lcdPosition(lcdhd,0,1);
        lcdPrintf(lcdhd, "Back 'D'");
        if (get_key() == 'D'){
            play_TIME = 0;
        }
        check_alarm();
        delay(100);
    }
    play_TIME = 1;
    cleanLCD();
}


// end loop
int play = 1;
void cleanup(int signo){
	cleanLCD();
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(Buzzer, HIGH);
    lcdPosition(lcdhd,0,0);
    lcdPrintf(lcdhd,"POWER OFF");
	play = 0;
	delay(1000);
    cleanLCD();
}

// main program
int main(void){
    signal(SIGINT, cleanup);    // program terminated using ctrl+c calls cleanup class
    signal(SIGTERM, cleanup);   // kill command
    signal(SIGHUP, cleanup);    // closing the window
    
    // setup wiringPi
    wiringPiSetup();
    // setup LEDs
    init_LED();
    // setup Buzzer
    init_Buzzer();
    // setup Keypad
    init_keypad();
    // setup LCD screen
    init_LCD();
	// clear LCD
    cleanLCD();
    // turn off alarm
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(Buzzer, HIGH);
    
    while (play){
        lcdPosition(lcdhd,0,0);
        lcdPrintf(lcdhd,"A:CPU TEMP");
        lcdPosition(lcdhd,0,1);
        lcdPrintf(lcdhd,"B:Time");
        lcdPosition(lcdhd,8,1);
        lcdPrintf(lcdhd,"C:Alarm");
		
        if (get_key() == '*'){
			cleanLCD();
			lcdPosition(lcdhd,0,0);
			lcdPrintf(lcdhd,"POWER OFF");
            delay(1000);
            cleanLCD();
            play = 0;
        }
        if (get_key() == 'A'){
            printCPUTemperature();
        }
        if (get_key() == 'B'){
            printDataTime();
        }
        if (get_key() == 'C'){
            set_alarm();
        }
        check_alarm();
        delay(100); 
    }
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(Buzzer, HIGH);
}
