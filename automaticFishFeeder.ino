#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Keypad.h>
#include <Wire.h> 
#include <EEPROM.h>

const int buzzerPin = 2;
const byte ROWS = 4;  //Four rows
const byte COLS = 4;  //Four columns

// Define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22, 24, 26, 28};  // Connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 32, 34, 36};  // Connect to the column pinouts of the keypad

// Initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(0x27, 16, 4);

// Initialize DS1307RTC
RTC_DS1307 rtc;

const unsigned long displayTimer = 1000;
unsigned long displayPrevTime = 0;
unsigned long displayCurTime = 0;

// Declare array of structures for feed time schedules
struct time {
  int hour; 
  int minute;
  bool isActivated;
};

time feedTime[3];
int feedSchedCtr = 0, feedSchedPos = -1;
int curHour, choice, sub;
bool deleteFlag = false;

void setup(){
  rtc.begin();
  // Set initial time (adjust as needed)
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.init();
	lcd.backlight();
	lcd.clear();
	lcd.setCursor(0,0); 

  pinMode(buzzerPin, OUTPUT);
}

void loop(){
  // feedTime[0].hour = EEPROM.read(0);
  // feedTime[0].minute = EEPROM.read(1);
  char keyInput = customKeypad.getKey();  // For getting keypad inputs
  DateTime now = rtc.now();

  displayTime(now); // Function call to display current time

  // Checks if the current time is equal to set time schedule to start feeding
  for (int i = 0; i < 3; i++) {
    if (curHour == feedTime[i].hour && now.minute() == feedTime[i].minute && feedTime[i].isActivated == true) {
      triggerAlarm();
    }
  }

  if (keyInput == 'D'){
    setTime();  // Function call to set time 
  }
  else if(keyInput == 'A') {
    backToMenu:
    feedSchedMenu();  // Function call to display scheduling options

    // Ask input
    lcd.setCursor(0, 3);
    lcd.print("Enter your choice:");
    int choice = getTimeInput(19, 3, 1, 3);

    switch(choice) {
      case 1:
        viewSched();    // Function call to display the feeding schedules
        goto backToMenu;
        break;

      case 2:
        addFeedSched();   // Function call for adding feeding schedules 
        goto backToMenu;
        break;

      case 3:
        deleteFlag = true;
        viewSched();
        deleteFeedSched();    // Function call for deleting schedules
        goto backToMenu;
        break;
      
      case 0:
        return;
        break;
    }
  }
}

void displayTime(DateTime currentTime) {
  displayCurTime = millis();
  
  if(displayCurTime - displayPrevTime >= displayTimer) {
    lcd.clear();

    // Display the date
    lcd.setCursor(0, 0);
    lcd.print("DATE:");
    lcd.setCursor(6, 0);
    if(currentTime.month() < 10) {
      lcd.print("0");
    }
    lcd.print(currentTime.month(), DEC);
    lcd.print("/");

    if(currentTime.day() < 10) {
      lcd.print("0");
    }
    lcd.print(currentTime.day(), DEC);
    lcd.print("/");
    lcd.print(currentTime.year(), DEC);

    // Display the time;
    curHour = currentTime.hour();
    lcd.setCursor(0, 3);
    lcd.print(curHour);
    lcd.setCursor(0, 1);
    lcd.print("TIME:");
    // Convert the 24 hour format into 12 hour format
    if(curHour == 23 && choice == 2) {
      lcd.setCursor(15, 1);
      lcd.print("PM");
      curHour -= sub;
    }
    else if(curHour > 12) {
      lcd.setCursor(15, 1);
      lcd.print("PM");
      curHour -= 12;
      
      if(curHour == 0) {
        curHour += 12; 
      }
    }
    else {
      lcd.setCursor(15, 1);
      lcd.print("AM");

      if(curHour == 0) {
        curHour += 12; 
      }
    }

    lcd.setCursor(6, 1);
    if(curHour < 10) {
      lcd.print("0");
    }
    lcd.print(curHour, DEC);
    lcd.print(":");


    lcd.setCursor(9, 1);
    if(currentTime.minute() < 10) {
      lcd.print("0");
    }
    lcd.print(currentTime.minute(), DEC);
    lcd.print(":");

    lcd.setCursor(12, 1);
    if(currentTime.second() < 10) {
      lcd.print("0");
    }
    lcd.print(currentTime.second(), DEC);

    displayPrevTime = displayCurTime;
  }
}

void setTime() {
  // Set the date
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Month: ");
  int month = getTimeInput(13, 0, 2, 12);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Day Of Week: ");
  int day = getTimeInput(0, 1, 2, 31);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Year: ");
  int year = getTimeInput(12, 0, 4, 9000);

  // Set the time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Hours: ");
  int hour = getTimeInput(13, 0, 2, 12);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Minutes: ");
  int minute = getTimeInput(15, 0, 2, 60);

  // Selection for AM and PM
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[1] - AM");
  lcd.setCursor(0, 1);
  lcd.print("[2] - PM");
  lcd.setCursor(0, 2);
  lcd.print("Enter choice: ");
  choice = getTimeInput(14, 2, 1, 2);

  // Convert the input time into 24 hour format
  if(choice == 1) {
    hour +=  0;
  }
  else {
    if(hour == 12) {
      hour += 11;
      sub = 11;
    }
    else {
       hour += 12;
       sub = 12;
    }
  }

  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Enter Seconds: ");
  // int second = getTimeInput();

  // Set the date and time
  rtc.adjust(DateTime(year, month, day, hour, minute, 50));
}

void viewSched(){
  lcd.clear();

  // Display the feeding shedules
  while(true){
    for(int i = 0; i < 3; i++){
      lcd.setCursor(0, i);
      // Adds leading zero for time less than 10 and convert it into string
      String fHour = (feedTime[i].hour < 10 ? "0" : "") + String(feedTime[i].hour);
      String fMinute = (feedTime[i].minute < 10 ? "0" : "") + String(feedTime[i].minute);
      lcd.print(String(i + 1) + ") " + fHour + ":" + fMinute);
    }
    
    // break the loop
    if(deleteFlag == true) {
      break;
    }

    // Exit and back to menu
    char exitKey = customKeypad.getKey();
    if(exitKey == 'B' && deleteFlag == false) {
      break;
    }
    else if(deleteFlag == true){
      break;
    }
  }
}

void addFeedSched() {
  if(feedSchedCtr < 3){
    for(int i = 0; i < 3; i++) {
      feedSchedPos++;

      if(feedTime[i].isActivated == false){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Schedule " + String(feedSchedPos + 1));
        
        lcd.setCursor(0, 1);
        lcd.print("Enter Hour:");
        feedTime[feedSchedPos].hour = getTimeInput(12, 1, 2, 12);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Schedule " + String(feedSchedPos + 1));

        lcd.setCursor(0, 1);
        lcd.print("Enter Minute:");
        feedTime[feedSchedPos].minute = getTimeInput(14, 1, 2, 60);
        feedTime[feedSchedPos].isActivated = true;

        // int tmpCtnHour = feedTime[feedSchedPos].hour;
        // int tmpCtnMin = feedTime[feedSchedPos].minute;
        // EEPROM.update(0, tmpCtnHour);
        // EEPROM.update(1, tmpCtnMin);

        feedSchedCtr++;
        break;
      }
    }

    if (feedSchedCtr >= 3) {
      // All three schedules have been set, display a message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("All 3 schedules set");
      delay(3000);
    }
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("All 3 schedules set");
    delay(3000);
  }

  feedSchedPos = -1;
}

void deleteFeedSched() {
  lcd.setCursor(0, 3);
  lcd.print("Enter the number: ");
  int delInput = getTimeInput(18, 3, 1, 3);

  for(int i = 0; i < 3; i++){
    if(feedTime[i].hour == feedTime[delInput - 1].hour && 
      feedTime[i].minute == feedTime[delInput - 1].minute)
    {
      feedTime[i].hour = 0;
      feedTime[i].minute = 0;
      feedTime[i].isActivated = false;

      feedSchedCtr--;
    }
  }
  
  deleteFlag = false;
}

int getTimeInput(int cursorPosCols, int cursorPosRow, int charLen, int maxVal) {
  int cursorPos = cursorPosCols - 1;
  int cursorPosCtr = 0;
  String container = "";
  top:
  lcd.setCursor(cursorPosCols, cursorPosRow);

  while(true) {
    char keyInput = customKeypad.getKey();
    
    if(keyInput == '#') {
      break;
    } 
    else if(isDigit(keyInput) && cursorPosCtr < charLen) {
        container += keyInput;
        lcd.print(keyInput);
        cursorPosCtr++;
    } 
    else if(keyInput == '*' && cursorPosCtr > 0) {
      lcd.setCursor(cursorPos + cursorPosCtr, cursorPosRow);
      lcd.print(' ');
      lcd.setCursor(cursorPos + cursorPosCtr, cursorPosRow);
      container.remove(container.length() - 1);
      cursorPosCtr--;
    }
  }
  
  if(container != "" && container.toInt() >= 0 && container.toInt() <= maxVal) {
    return container.toInt();
  }
  else {
    lcd.setCursor(0, 3);
    lcd.print("Invalid Input");
    goto top;
  }
}

void feedSchedMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[1]-View Schedule");
  lcd.setCursor(0, 1);
  lcd.print("[2]-Add Schedule");
  lcd.setCursor(0, 2);
  lcd.print("[3]-Delete Schedule");
}

void triggerAlarm() {
  tone(buzzerPin, 350);
}