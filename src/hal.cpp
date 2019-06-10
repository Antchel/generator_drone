#include "hal.h"
#include <SPI.h>
#include <EEPROM.h>

long current_freq = 0;
long current_pow = 0;

int hal_get_pow()
{
    byte hi_pow_byte = EEPROM.read(10);
    byte low_pow_byte = EEPROM.read(11);
    current_pow = hi_pow_byte<<8 | low_pow_byte;
    return current_pow;
}

void hal_set_pow(int pow)
{
    if (pow <= 0) pow = 0;
    if (pow >= 511) pow = 511;
    current_pow = pow;
    analogWrite(PWR_PIN,pow);// Посредством ШИМ управляем усилением ADL5330 (8,5 отсчетов - 1 дБм)
    byte hi = highByte(current_pow); // старший байт
    byte low = lowByte(current_pow); // младший байт
    EEPROM.write(10, hi);  // записываем в ячейку 1 старший байт
    EEPROM.write(11, low); // записываем в ячейку 2 младший байт
}

long hal_get_freq()
{
    byte byte_freq[4];
    for(byte i = 0; i < 4; i++) byte_freq[i] = EEPROM.read(i);
    long &last_freq = (long&)byte_freq;
    return last_freq;
}

void hal_set_freq(long freq)
{
    current_freq = freq;
    long long ftw;
    ftw = freq * pow(2,48) / pow(10,9); // Перерасчет частоты для управления AD9912
    char data[6];
    for (size_t i=0;i<6;i++)
    {
        data[i] = ftw & 0xFF;
        ftw >>= 8;
    }

    SPI.begin();
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x61); // 011b + 0x01 - 1 байт адреса регистра кода частоты
    SPI.transfer(0xAB); // 2 байт адреса регистра кода частоты
    for (int i=0;i<5;i++)
    {
        SPI.transfer(data[5-i]); // запись кода частоты в регистр
    }
    digitalWrite (SPI_SS,HIGH);
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x00);
    SPI.transfer(0x05);
    SPI.transfer(0x01);
    digitalWrite (SPI_SS,HIGH);
    SPI.end();
    
    for (size_t i=0;i<4;i++)
    {
        data[i] = current_freq & 0xFF;
        EEPROM.write(i,data[i]);
        current_freq >>= 8;
    } 
}

int hal_init()
{
    // усилитель
    TCCR1A = TCCR1A & 0xe0|2; // Выбор таймера
    TCCR1B = TCCR1B & 0xe0|0x09; // Настройка вывода в режим ШИМ 9 разрядов, частота 31,5 кГц
    pinMode(PWR_PIN,OUTPUT);

    // DDS
    pinMode(SPI_SS,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    digitalWrite (SPI_SS,HIGH);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); // Установка частоты такта SPI
    SPI.setDataMode(SPI_MODE0); // Установка режима работы SPI
    SPI.setBitOrder(MSBFIRST); // Установка режима отправки посылки
    //------------------------------
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x00);// 1 байт адреса регистра конфигурации
    SPI.transfer(0x00);// 2 байт адреса регистра конфигурации
    SPI.transfer(0x18); // Записываем в регистр значение
    digitalWrite (SPI_SS,HIGH);
    //------------------------------
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x00);// 1 байт адреса регистра конфигурации
    SPI.transfer(0x10);// 2 байт адреса регистра конфигурации
    SPI.transfer(0xC0); // Записываем в регистр значение
    digitalWrite (SPI_SS,HIGH);
    //------------------------------
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x00);// 1 байт адреса регистра делителя частоты
    SPI.transfer(0x20);// 2 байт адреса регистра делителя частоты
    SPI.transfer(0x08); // Записываем в регистр делителя частоты (8+2)*2=20
    digitalWrite (SPI_SS,HIGH);
    //------------------------------
    digitalWrite (SPI_SS,LOW);
    SPI.transfer(0x02);// 1 байт адреса регистра Output drivers
    SPI.transfer(0x00);// 2 байт адреса регистра Output drivers
    SPI.transfer(0x06); // Записываем в регистр значение
    digitalWrite (SPI_SS,HIGH);
    //------------------------------
    SPI.end();

    delay(1000);
    hal_set_freq(hal_get_freq());
    hal_set_pow(hal_get_pow());
}

