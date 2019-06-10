#ifndef __HAL_H_
#define __HAL_H_

#define SPI_SS 10
#define SPI_MOSI 11
#define SPI_CLK 13
#define PWR_PIN 9

int hal_init();

void hal_set_pow(int pow);
int hal_get_pow();

void hal_set_freq(long freq);
long hal_get_freq();


#endif /* __HAL_H_ */
