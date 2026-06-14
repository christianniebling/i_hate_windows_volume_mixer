#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define SAMPLE_TIME_MS 10
#define NUM_CHANNELS 3

// GPIO ASSIGNMENTS
// These are GPIO numbers
#define SLIDER_1 26
#define SLIDER_2 27
#define SLIDER_3 28

// Button pins
#define BUTTON_1 15
#define BUTTON_2 11
#define BUTTON_3 10

// TFT pins
#define TFT_MOSI      3
#define TFT_SCLK      2
#define TFT_CS        1
#define TFT_DC        5
#define TFT_RST       4 // Set to -1 if tied to Pico 3V3

// Initialize Adafruit ST7789 using Hardware SPI
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

String section1 = "Data 1";
String section2 = "Data 2";
String section3 = "Data 3";

unsigned long lastADCMillis = 0;

const uint button_pins[NUM_CHANNELS] = {BUTTON_1, BUTTON_2, BUTTON_3};
const uint pot_pins[NUM_CHANNELS] = {SLIDER_1, SLIDER_2, SLIDER_3};

uint16_t pot_values[NUM_CHANNELS] = {0};

// Setup -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  
  // Configure the ADC pins as inputs
  pinMode(SLIDER_1, INPUT);
  pinMode(SLIDER_2, INPUT);
  pinMode(SLIDER_3, INPUT);

  // Initialize the 1.9" 320x170 display
  tft.init(170, 320); 
  
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  tft.setSPISpeed(50000000);

  // Set rotation to landscape (Try 1 or 3 to find your preferred orientation)
  tft.setRotation(3); 
  
  tft.fillScreen(ST77XX_BLACK);

  // Draw the layout structure lines once initially
  drawLayoutFrames();
  updateDisplayData();

  Serial.println(F("System Ready. Send format: 'val1,val2,val3'"));
}

// Main Loop -------------------------------------------------------------------------

void loop() {
  checkIncomingSerial();
  readAndReportSliders();
}


// FUNCTIONS -------------------------------------------------------------------------

void checkIncomingSerial() {
  // Check if text has arrived in the Serial buffer
  if (Serial.available() > 0) {
    // Read the incoming line until a newline character
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim(); // Trim white space or hidden carriage returns
    
    if (incomingData.length() > 0) {
      // Find the comma indices
      int firstComma = incomingData.indexOf(',');
      int secondComma = incomingData.indexOf(',', firstComma + 1);
      
      // Basic validation: Make sure we found at least two commas
      if (firstComma != -1 && secondComma != -1) {
        // Extract substrings matching your data frames
        section1 = incomingData.substring(0, firstComma);
        section2 = incomingData.substring(firstComma + 1, secondComma);
        section3 = incomingData.substring(secondComma + 1);
        
        // Refresh display with new values
        updateDisplayData();
      } else {
        Serial.println(F("Error: Invalid format. Please send as: app1,app2,app3"));
      }
    }
  }
}

void readAndReportSliders() {
  unsigned long currentMillis = millis();
  if (currentMillis > lastADCMillis + SAMPLE_TIME_MS) {
    lastADCMillis = currentMillis;

    for (int i = 0; i < NUM_CHANNELS; i++) {
      pot_values[i] = analogRead(pot_pins[i]);
    }

    Serial.printf("%u|%u|%u\n", pot_values[0], pot_values[1], pot_values[2]);
  }
}


int map(int input, int in_min, int in_max, int out_min, int out_max) {
    return ((input - in_min) * (out_max - out_min))/(in_max - in_min) + out_min;
}

// Map a 0-4096 value to a volume level
int map_to_vol_range(int input) {
    return map(input, 0, 4095, 0, 100);
}


// Function to draw column containers
void drawLayoutFrames() {
  int screenWidth = tft.width();   // Should be 320
  int screenHeight = tft.height(); // Should be 170
  
  // Calculate dynamic width split for 3 equal columns (~106 pixels each)
  int colWidth = screenWidth / 3;
  
  // Draw layout dividing lines (Dark Gray to keep it clean)
  uint16_t dividerColor = tft.color565(80, 80, 80);
  tft.drawFastVLine(colWidth, 0, screenHeight, dividerColor);
  tft.drawFastVLine(colWidth * 2, 0, screenHeight, dividerColor);
}

// Function to handle redrawing text blocks safely without flicker
void updateDisplayData() {
  int colWidth = tft.width() / 3; // ~106 pixels
  int textY = 75;                // Centered vertically on a 170px tall display
  
  tft.setTextWrap(true);
  tft.setTextSize(2); // Legible size for mid-range viewing
  
  // --- SECTION 1 (Left Column) ---
  // Overwrite text background cleanly by pairing text color with a black text background
  tft.fillRect(2, textY - 5, colWidth - 4, 30, ST77XX_BLACK); 
  tft.setCursor(10, textY);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK); 
  tft.print(section1);
  
  // --- SECTION 2 (Center Column) ---
  tft.fillRect(colWidth + 2, textY - 5, colWidth - 4, 30, ST77XX_BLACK);
  tft.setCursor(colWidth + 10, textY);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  tft.print(section2);
  
  // --- SECTION 3 (Right Column) ---
  tft.fillRect((colWidth * 2) + 2, textY - 5, colWidth - 4, 30, ST77XX_BLACK);
  tft.setCursor((colWidth * 2) + 10, textY);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.print(section3);
}