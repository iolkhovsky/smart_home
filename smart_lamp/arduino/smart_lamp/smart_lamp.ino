
#include <ArduinoBLE.h>
#include <Adafruit_NeoPixel.h>

const char* nameOfPeripheral = "SmartLamp";
const char* uuidOfService = "00001101-0000-1000-8000-00805f9b34fb";
const char* uuidOfRxChar = "00001142-0000-1000-8000-00805f9b34fb";
const char* uuidOfTxChar = "00001143-0000-1000-8000-00805f9b34fb";

BLEService smartLampService(uuidOfService);

const int WRITE_BUFFER_SIZE = 256;
bool WRITE_BUFFER_FIZED_LENGTH = false;

BLECharacteristic rxChar(uuidOfRxChar, BLEWriteWithoutResponse | BLEWrite, WRITE_BUFFER_SIZE, WRITE_BUFFER_FIZED_LENGTH);
BLEByteCharacteristic txChar(uuidOfTxChar, BLERead | BLENotify | BLEBroadcast);

short sampleBuffer[256];
volatile int samplesRead;

int LED_PIN = A7;
int PHOTORESISTOR_PIN = A6;
int LED_LAMP_PIN = A5;

#define CLASTER_COUNT 7

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(CLASTER_COUNT, LED_LAMP_PIN, NEO_GRB + NEO_KHZ800);

void setup() {

  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_LAMP_PIN, OUTPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  startBLE();

  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(smartLampService);
  smartLampService.addCharacteristic(rxChar);
  smartLampService.addCharacteristic(txChar);
  BLE.addService(smartLampService);

  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
  
  rxChar.setEventHandler(BLEWritten, onRxCharValueUpdate);
  
  BLE.advertise();
  
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println(nameOfPeripheral);
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(smartLampService.uuid());
  Serial.print("rxCharacteristic UUID: ");
  Serial.println(uuidOfRxChar);
  Serial.print("txCharacteristics UUID: ");
  Serial.println(uuidOfTxChar);

  Serial.println("Bluetooth device active, waiting for connections...");

  matrix.begin();
}


void loop()
{
  BLEDevice central = BLE.central();
  
  if (central)
  {
    txChar.writeValue(digitalRead(PHOTORESISTOR_PIN));
  }
}

void startBLE() {
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
}

void onRxCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Characteristic event, read: ");
  char rx_buffer[256];
  int dataLength = rxChar.readValue(rx_buffer, 256);
  if (dataLength > 0)
  {
    rx_buffer[dataLength] = 0;
  }
  for(int i = 0; i < dataLength; i++) {
    Serial.print((char)rx_buffer[i]);
  }
  Serial.println();
  Serial.print("Value length = ");
  Serial.println(rxChar.valueLength());
  
  String cmd_text(rx_buffer);
  String cmd = getValue(cmd_text, ' ', 0);
  if (cmd == String("rgb")) {
    int r = 0;
    int g = 0;
    int b = 0;
    
    String rstr = getValue(cmd_text, ' ', 1);
    if (isValidNumber(rstr))
      r = rstr.toInt();
    String gstr = getValue(cmd_text, ' ', 2);
    if (isValidNumber(gstr))
      g = rstr.toInt();
    String bstr = getValue(cmd_text, ' ', 3);
    if (isValidNumber(bstr))
      b = rstr.toInt();

    lampControl(r, g, b);
  }
}

void onBLEConnected(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  connectedHandler();
}

void onBLEDisconnected(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  disconnectedHandler();
}


void connectedHandler() {
  infoLedOn();
  delay(100);
  infoLedOff();
}


void disconnectedHandler() {
  infoLedOn();
  delay(100);
  infoLedOff();
  delay(100);
  infoLedOn();
  delay(100);
  infoLedOff();
}

void lampControl(int r, int g, int b) {
  colorWipe(matrix.Color(r, g, b), 1);
}

void infoLedOn() {
  digitalWrite(LED_PIN, HIGH);
}

void infoLedOff() {
  digitalWrite(LED_PIN, LOW);
}

void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < matrix.numPixels(); i++) {
    matrix.setPixelColor(i, c);
    matrix.show();
    delay(wait);
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

boolean isValidNumber(String str){
   boolean isNum=false;
   for(byte i=0;i<str.length();i++)
   {
       isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
       if(!isNum) return false;
   }
   return isNum;
}
