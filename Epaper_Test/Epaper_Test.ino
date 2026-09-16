#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>


// =====================================================
// Seeed Studio ePaper Driver Board 114993558
// XIAO ESP32-S3 Sense
// =====================================================

#define EPD_CS     D1
#define EPD_DC     D3
#define EPD_RST    D0
#define EPD_BUSY   D2


// =====================================================
// Display: GDEY0213B74
// 2.13 inch
// 122 x 250 Pixel
// SSD1680
// =====================================================

GxEPD2_BW<
  GxEPD2_213_GDEY0213B74,
  GxEPD2_213_GDEY0213B74::HEIGHT
>
display(
  GxEPD2_213_GDEY0213B74(
    EPD_CS,
    EPD_DC,
    EPD_RST,
    EPD_BUSY
  )
);


// =====================================================
// Zeichnen
// =====================================================

void drawContent()
{
  display.setRotation(1);

  display.setFont(&FreeMonoBold9pt7b);

  display.setTextColor(GxEPD_BLACK);


  // Hintergrund weiß

  display.fillScreen(GxEPD_WHITE);


  // Rahmen

  display.drawRect(
    2,
    2,
    display.width()-4,
    display.height()-4,
    GxEPD_BLACK
  );


  // Text

  display.setCursor(15,35);
  display.println("Hallo Marcel!");


  display.setCursor(15,60);
  display.println("XIAO ESP32-S3");


  display.setCursor(15,85);
  display.println("E-Paper Test");


  // Smiley

  int x = 180;
  int y = 80;
  int r = 25;


  display.drawCircle(
    x,
    y,
    r,
    GxEPD_BLACK
  );


  display.fillCircle(
    x-9,
    y-8,
    3,
    GxEPD_BLACK
  );


  display.fillCircle(
    x+9,
    y-8,
    3,
    GxEPD_BLACK
  );


  display.drawLine(
    x-10,
    y+10,
    x+10,
    y+10,
    GxEPD_BLACK
  );
}


// =====================================================
// Setup
// =====================================================

void setup()
{

  Serial.begin(115200);

  delay(1000);


  Serial.println("E-Paper Start");


  display.init(115200);


  display.setFullWindow();


  display.firstPage();


  do
  {

    drawContent();

  }
  while(display.nextPage());


  Serial.println("Anzeige fertig");


  delay(3000);


  display.hibernate();

}



// =====================================================

void loop()
{

}