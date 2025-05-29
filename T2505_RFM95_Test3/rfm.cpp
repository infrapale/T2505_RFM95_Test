
#include <SPI.h>
#include <RH_RF95.h>
#include "main.h"
#include "io.h"
#include #rfm.h"

void rfm_initialize(void)
{
 if (!rf95.init())
    Serial.println("init failed");

}