/*
 *
 *  LIS2DH12 MEMS digital output motion sensor
 *  More info @ aconno.de
 *  Made by Jurica Resetar
 *  jurica_resetar@yahoo.com
 *
 */

#include "LIS2DH12.h"
#include "LIS2DH12_regs.h"

LIS2DH12::LIS2DH12(I2C *i2c_, char i2c_address) : i2c(i2c_),
                                              spiInterface(false),
                                              i2c_address(i2c_address<<1){
}

LIS2DH12::LIS2DH12(SPI *_spi, DigitalOut *_cs) : i2c(NULL),
                                                 spi(_spi),
                                                 cs(_cs),
                                                 spiInterface(true){
    *cs = 1;
    spi->format(8, 3);
    spi->frequency(1000000);
}

uint8_t LIS2DH12::whoIAm(){
    char regAddr = (char)WHO_AM_I;
    char regData;
    readFromReg(regAddr, (uint8_t*)&regData, 1);
    return (uint8_t)regData;
}

uint8_t LIS2DH12::setMode(Mode mode){
    char ctrl1Copy;
    char ctrl4Copy;
    uint8_t success;

    readFromReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    readFromReg((char)CTRL_REG4, (uint8_t*)&ctrl4Copy, 1);

    switch(mode){
        case HIGH_RES:
            ctrl1Copy &= 0xF7;
            ctrl4Copy |= 0x08;
            break;
        case NORMAL:
            ctrl1Copy &= 0xF7;
            ctrl4Copy &= 0xF7;
            break;
        case LOW_POWER:
            ctrl1Copy |= 0x08;
            ctrl4Copy &= 0xF7;
            break;
        default:
            return 0;
    }
    writeToReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    success = writeToReg((char)CTRL_REG4, (uint8_t*)&ctrl4Copy, 1);
    return success;
}

uint8_t LIS2DH12::enableAxes(Axis axis){
    char ctrl1Copy;
    readFromReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    ctrl1Copy |= axis;
    writeToReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    return 0;
}

uint8_t LIS2DH12::disableAxes(Axis axis){
    char ctrl1Copy;
    readFromReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    ctrl1Copy &= ~(1 << axis);
    writeToReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    return 0;
}

int16_t LIS2DH12::readXAxis(){
    int16_t rawData;
    char tempData;
    // Make sure new data is ready
    // do{
    //     readFromReg((char)STATUS, (uint8_t*)&tempData, 1);
    // }while(!(tempData & 0x08));
    // do{
    //     readFromReg((char)STATUS, (uint8_t*)&tempData, 1);
    // }while(!(tempData & 0x80));
    // Same data have been overwritten

    //readFromReg((char)OUT_X_H, (uint8_t*)&tempData, 1);
    //rawData = (int8_t)tempData << 8;
    readFromReg((char)OUT_X_L, (uint8_t*)&rawData, 1);
    readFromReg((char)OUT_X_H, ((uint8_t*)&rawData)+1, 1);

    if (rawData >= 0)
        rawData = (rawData>>4);
    else
        rawData = (rawData>>4) | 0xF000;

    return rawData;
}

int16_t LIS2DH12::readYAxis(){
    int16_t rawData;
    char tempData;
    //readFromReg((char)OUT_Y_H, (uint8_t*)&tempData, 1);
    //rawData = (int8_t)tempData << 8;
    readFromReg((char)OUT_Y_L, (uint8_t*)&rawData, 1);
    readFromReg((char)OUT_Y_H, ((uint8_t*)&rawData)+1, 1);

    if (rawData >= 0)
        rawData = (rawData>>4);
    else
        rawData = (rawData>>4) | 0xF000;

    return rawData;
}

int16_t LIS2DH12::readZAxis(){
    int16_t rawData = 0;
    char tempData;
    //readFromReg((char)OUT_Z_H, (uint8_t*)&tempData, 1);
    //rawData = (int8_t)tempData << 8;
    readFromReg((char)OUT_Z_L, (uint8_t*)&rawData, 1);
    readFromReg((char)OUT_Z_H, ((uint8_t*)&rawData)+1, 1);

    if (rawData >= 0)
        rawData = (rawData>>4);
    else
        rawData = (rawData>>4) | 0xF000;

    return rawData;
}

uint8_t LIS2DH12::setODR(Odr odr){
    char ctrl1Copy;
    readFromReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    ctrl1Copy |= (odr << 4);
    writeToReg((char)CTRL_REG1, (uint8_t*)&ctrl1Copy, 1);
    return 0;
}

uint8_t LIS2DH12::setScale(Scale scale){
    char ctrl4Copy;
    readFromReg((char)CTRL_REG4, (uint8_t*)&ctrl4Copy, 1);
    ctrl4Copy |= (scale << 4);
    writeToReg((char)CTRL_REG4, (uint8_t*)&ctrl4Copy, 1);
    return 0;
}

/* Interrupt activity 1 driven to INT1 pad */
// TODO: Napraviti tako da postoji samo jedna metoda int1Setup koja prima gro
// parametara: THS, DUR, Latch...
uint8_t LIS2DH12::int1Setup(uint8_t setup){
    char data = setup;
    writeToReg((char)CTRL_REG3, (uint8_t*)&data, 1);
    return 0;
}

uint8_t LIS2DH12::int1Latch(uint8_t enable){
    char ctrl5Copy;
    readFromReg((char)CTRL_REG5, (uint8_t*)&ctrl5Copy, 1);
    ctrl5Copy |= enable;
    writeToReg((char)CTRL_REG5, (uint8_t*)&ctrl5Copy, 1);
    return 0;
}

uint8_t LIS2DH12::int1Duration(uint8_t duration){
    char copy = duration;
    writeToReg((char)INT1_DURATION, (uint8_t*)&copy, 1);
    return 0;
}

uint8_t LIS2DH12::int1Threshold(uint8_t threshold){
    char copy = threshold;
    writeToReg((char)INT1_THS, (uint8_t*)&copy, 1);
    return 0;
}

uint8_t LIS2DH12::int1Config(uint8_t config){
    char copy = config;
    writeToReg((char)INT1_CFG, (uint8_t*)&copy, 1);
    return 0;
}

void LIS2DH12::clearIntFlag(){
    char data;
    readFromReg((char)INT1_SRC, (uint8_t*)&data, 1);
}

uint8_t LIS2DH12::readFromReg(uint8_t regAddr, uint8_t *buff, size_t buffSize)
{
    // Most significant bit represents read from register, bit after it
    // represents address increment if multiple read, which is enabled.
    const uint8_t spiSetup = 0xC0;

    uint8_t retVal = 0;

    if( spiInterface )
    {
        *cs = 0;

        spi->write(spiSetup | regAddr);

        while(buffSize--)
        {
            *buff = spi->write(0x00);
            buff++;
        }

        *cs = 1;
    }
    else
    {
        i2c->write(i2c_address, (const char*)&regAddr, 1);
        retVal = i2c->read((int)i2c_address, (char*) buff, (int) buffSize);
    }

    return retVal;
}

uint8_t LIS2DH12::writeToReg(uint8_t regAddr, uint8_t *buff, size_t buffSize)
{
    // Most significant bit represents write from register, bit after it
    // represents address increment if multiple write, which is enabled.
    const uint8_t spiSetup = 0x40;

    uint8_t retVal = 0;

    if( spiInterface )
    {
        *cs = 0;

        spi->write(spiSetup | regAddr);

        while(buffSize--)
        {
            spi->write(*buff);
            buff++;
        }

        *cs = 1;
    }
    else
    {
        char i2c_buff[10];
        i2c_buff[0]=(char)regAddr;
        memcpy(i2c_buff+1, buff, buffSize);
        retVal = i2c->write(i2c_address, i2c_buff, (int) buffSize+1);
    }

    return retVal;
}
