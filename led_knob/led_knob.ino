// Menu Setup
int menuItem = 1;   // menu item counter
int frame = 1;    // frame counter
String menuItem1 = "Margarita";
int margarita[5] = {0,0,0,0,0};
String menuItem2 = "Screw Driver";
int screwDriver[5] = {0,0,0,0,0};
String menuItem3 = "Cosmopolitan";
int cosmo[5] = {0,0,0,0,0};
String menuItem4 = "Martini";
int martini[5] = {0,0,0,0,0};

// Rotatry Encoder
int counter = 0;
int CLK  = 16;    // CLK pin
int DT = 17;    // DT pin
int SW = 21;    // SW pin
int currentStateCLK;    // current CLK state
int lastStateCLK;   // last CLK state
String currentDir = "";   // current direction
unsigned long lastButtonPress = 0;    // last btn press duration

// LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 , if not working 0x3F
// a 16 chars and 2 lines display
int sdaPin = 23;   // SDA pin 
int sclPin = 22;   // SCL pin 

void setup() {
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);    // init CLK state

  lcd.autoscroll();   // enable each line to auto scroll from left -> right
  lcd.begin(sdaPin, sclPin);    // init the lcd with SDA and SCL pins
  lcd.backlight();    // turn on backlight
  // welcome screen
  lcd.setCursor(0,0);   // col: 0, row: 0
  // lcd.print("Welcome to");
  // lcd.setCursor(0,1);   // col: 0, row: 1
  // lcd.print("Drinking Bot!");
  // delay(5000);
  // lcd.clear();
  Serial.begin(9600);
}

void loop() {
  // render menu
  drawMenu();

  currentStateCLK = digitalRead(CLK);
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
      lcd.clear();
      if (menuItem == 2 && frame == 2) {
        menuItem--;
        frame--;
      } else if (menuItem == 3 && frame == 3) {
        menuItem--;
        frame--;
      } else if (menuItem > 1) {
        menuItem--;
      }
			currentDir ="CCW";
		} else {
      lcd.clear();
			// Encoder is rotating CW so increment
      if (menuItem == 2 && frame == 1) {
        menuItem++;
        frame++;
      } else if (menuItem == 3 && frame == 2) {
        menuItem++;
        frame++;
      } else if (menuItem < 4) {
        menuItem++;
      }
			currentDir ="CW";
		}
		Serial.print("Direction: ");
		Serial.print(currentDir);
		Serial.print(" | Menu Item: ");
		Serial.println(menuItem);
	}

	// Remember last CLK state
	lastStateCLK = currentStateCLK;

	// Read the button state
	int btnState = digitalRead(SW);
	// If we detect LOW signal, button is pressed
	if (btnState == LOW) {
		// if 50ms have passed since last LOW pulse, it means that the
		// button has been pressed, released and pressed again
		if (millis() - lastButtonPress > 50) {
			pushButton();
      delay(5000);
      // TODO: 
      // when drink is done
      // return to menu
		}
		// Remember last button press event
		lastButtonPress = millis();
	}

	// debounce reading
	delay(5);
}

// once btn pushed, transition to next scene
void pushButton() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your drink is");
  lcd.setCursor(0, 1);
  lcd.print("on the way...");
}

// render menu
void drawMenu() {
  if (menuItem == 1 && frame == 1) {
    displayMenuItem(menuItem1, 0, true);
    displayMenuItem(menuItem2, 1, false);
  } else if (menuItem == 2 && frame == 1) {
    displayMenuItem(menuItem1, 0, false);
    displayMenuItem(menuItem2, 1, true);
  } else if (menuItem == 2 && frame == 2) {
    displayMenuItem(menuItem2, 0, true);
    displayMenuItem(menuItem3, 1, false);
  } else if (menuItem == 3 && frame == 2) {
    displayMenuItem(menuItem2, 0, false);
    displayMenuItem(menuItem3, 1, true);
  } else if (menuItem == 3 && frame == 3) {
    displayMenuItem(menuItem3, 0, true);
    displayMenuItem(menuItem4, 1, false);
  } else if (menuItem == 4 && frame == 3) {
    displayMenuItem(menuItem3, 0, false);
    displayMenuItem(menuItem4, 1, true);
  }
}

// render individual menu item
void displayMenuItem(String item, int row, boolean selected) {
  lcd.setCursor(0, row);
  if (selected) {
    // if selected, place a chevron in front of the item
    lcd.print(">"+item);
  } else {
    lcd.print(" "+item);
  }
}
