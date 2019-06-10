/****************************************

****************************************/

#include "hal.h"
#include "jstp.h"
 

void setup() 
{
  Serial.begin(115200);
  hal_init();
  jstp_init();
}
 
 
void loop() 
{
   char tx;
   while (true)
   {
      while( Serial.available()) 
      {
        jstp_rx_push_char(Serial.read());
      }
      jstp_tick();
      while(jstp_tx_pop_char(&tx))
      {
         Serial.write(tx);
      }
      
   }
  
}
