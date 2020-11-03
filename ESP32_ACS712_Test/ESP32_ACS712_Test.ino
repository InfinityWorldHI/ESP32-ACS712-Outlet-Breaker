#define pinAC  34
//***********************************************************
//ESP32 Temprerature
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
//***********************************************************
int mVperAmp = 185; // use 100 for 20A Module and 66 for 30A Module and 185 for 5A Module

double mVolt = 0;
double VRMS = 0;
double mAmps = 0;
double Watts = 0;
int ACvolts = 220;
//***********************************************************
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  analogReadResolution(11);
  analogSetWidth(11);
  analogSetPinAttenuation(pinAC, ADC_11db);
  adcAttachPin(pinAC);
  adcStart(pinAC);
  
  int ESP_temp = (temprature_sens_read() - 32) / 1.8;
  
  Serial.print("ESP32 Temperature = ");
  Serial.println(ESP_temp);
}
//***********************************************************
void loop() {
  mVolt = getVPP();
//  VRMS = (mVolt / 2.0) * 0.707; //root 2 is 0.707
  mAmps = (mVolt * 1000) / mVperAmp;
  Watts = (ACvolts * mAmps) / 1000;
  Serial.print("  Amps = ");
  Serial.print(mAmps);
  Serial.print(" mA");
  Serial.print("  Power = ");
  Serial.print(Watts);
  Serial.println(" mW");
  delay(500);
}
//***********************************************************
float getVPP(){
  double volts = 0;
  long ADCval = 0;
  int subDCval = 0;
  for (int i=0; i<50; i++) {
    ADCval += analogRead(pinAC);
    delay(2);
  }
  ADCval /= 50;
  Serial.print("ADC = ");
  Serial.print(ADCval);
  
  subDCval = ADCval - 1360;
  if (subDCval < 10 && subDCval > 0) {
    ADCval = 0;
  }
  volts = (ADCval * 3530 ) / 2047.0;
  
  Serial.print("  Sub ADC = ");
  Serial.print(subDCval);
  Serial.print("  Voltage = ");
  Serial.print(volts);
  Serial.print(" mV");

  return volts;
}
