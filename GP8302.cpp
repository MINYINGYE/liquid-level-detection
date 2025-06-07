//----------------------------------------------------------------------
#include "Support.hpp"
//----------------------------------------------------------------------
#define SDAP 2
#define SCLP 3
#define LaserP 23
#define LightP 24

#define SDAL() {bcm2835_gpio_write(SDAP,LOW);}
#define SDAH() {bcm2835_gpio_write(SDAP,HIGH);}
#define SCLL() {bcm2835_gpio_write(SCLP,LOW);}
#define SCLH() {bcm2835_gpio_write(SCLP,HIGH);}

// 用于启动I2C通信，确保从设备准备好接收数据。
//----------------------------------------------------------------------
void I2cStart(void)
{
    SCLH(); // 将SCL拉高
    SDAH(); // 将SDA拉高
    usleep(4); // 短暂延时
    SDAL(); // 将SDA拉低
    SCLL(); // 将SCL拉低
    usleep(4); // 短暂延时
}
// I2cStop()：用于结束I2C通信，确保从设备知道数据传输已完成。
//----------------------------------------------------------------------
void I2cStop(void)
{
    SCLH(); // 将SCL拉高
    SDAL(); // 将SDA拉低
    usleep(4); // 短暂延时
    SDAH(); // 将SDA拉高
    usleep(4); // 短暂延时
}
// I2cWaitAck()：用于等待从设备的应答信号，确保数据被成功接收。
//---------------------------------------------------------------------
int I2cWaitAck(void)
{
    bcm2835_gpio_fsel(SDAP, BCM2835_GPIO_FSEL_INPT); // 将SDA设置为输入模式
    SCLH(); // 将SCL拉高
    usleep(4); // 短暂延时
    if (bcm2835_gpio_lev(SDAP) != 0) { // 检查SDA电平
        return 1; // 未收到应答
    }
    SCLL(); // 将SCL拉低
    bcm2835_gpio_fsel(SDAP, BCM2835_GPIO_FSEL_OUTP); // 将SDA恢复为输出模式
    return 0; // 收到应答
}
//----------------------------------------------------------------------
void I2cWrByte(unsigned char Wbyte)
{
    unsigned char i;
    SCLL();
    for(i=0;i<8;i++)
    {
        if(Wbyte&0x80){SDAH();}
        else SDAL();
        SCLH();
        usleep(2);
        SCLL();
        Wbyte<<=1;
        usleep(2);
    }
}
// 写入数据到GP8302
//----------------------------------------------------------------------
int WrGp(int data)
{
    unsigned char Dat;
     // 发送I2C启动信号
    I2cStart();
     // 发送设备地址和写命令
    I2cWrByte(0xB0);
    if(I2cWaitAck()){return 1;}
    I2cWrByte(0x02);// 寄存器地址
    I2cWaitAck();
    
    //Dat=(data<<4)&0xF0;
     // 将12位数据分为高4位和低8位
    Dat=(data<<4)&0xF0;
    I2cWrByte(Dat);
    //printf("%02X",Dat);
    I2cWaitAck();
    
    Dat=(data>>4)&0x0FF;
    //Dat=data&0x0FF;
    I2cWrByte(Dat);
    //printf("%02X ",Dat);
    I2cWaitAck();
    //printf("\n",Dat);
    //bcm2835_delay(100);
    usleep(100);
    I2cStop();
    return 0;
}

// 初始化GP8302
//----------------------------------------------------------------------
void init8302(void)
{
// 初始化I2C引脚
 bcm2835_gpio_fsel(SDAP,BCM2835_GPIO_FSEL_OUTP);
 bcm2835_gpio_fsel(SCLP,BCM2835_GPIO_FSEL_OUTP);
 bcm2835_gpio_fsel(LaserP,BCM2835_GPIO_FSEL_OUTP);
 bcm2835_gpio_fsel(LightP,BCM2835_GPIO_FSEL_OUTP);
 
 //bcm2835_gpio_write(LaserP,HIGH);
 //bcm2835_gpio_write(LightP,HIGH);
 
 bcm2835_gpio_fsel(MenuK,BCM2835_GPIO_FSEL_INPT);
 bcm2835_gpio_fsel(UpK,BCM2835_GPIO_FSEL_INPT);
 bcm2835_gpio_fsel(DownK,BCM2835_GPIO_FSEL_INPT);
 bcm2835_gpio_fsel(SetK,BCM2835_GPIO_FSEL_INPT);
}
//----------------------------------------------------------------------------
