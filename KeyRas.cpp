//----------------------------------------------------------------------------
#include "Support.hpp"
#define MenuK    5
#define UpK     6
#define DownK   13
#define SetK    19
//---------------------------------------------------------------------------
#define KLeft   81//2424832
#define KUp     82//2490368
#define KRight  83//2555904
#define KDown   84//2621440


//---------------------------------------------------------------------------
// /**
//  * @brief 保存配置到文件
//  */
void SaveConfig() {
    Conbuff[0x3D] = Tstyle;
    Conbuff[0x3E] = (PositionShift >> 8) & 0xFF;
    Conbuff[0x3F] = PositionShift & 0xFF;

    Conbuff[0x41] = Laseron & 0xFF;
    Conbuff[0x43] = Lighton & 0xFF;
    Conbuff[0x44] = (Contrast & 0xFF00) >> 8;
    Conbuff[0x45] = Contrast & 0xFF;

    unsigned char* tmpfloat = (unsigned char*)&Alpha;
    Conbuff[0x46] = tmpfloat[0];
    Conbuff[0x47] = tmpfloat[1];
    Conbuff[0x48] = tmpfloat[2];
    Conbuff[0x49] = tmpfloat[3];

    Conbuff[0x4A] = (CBeta & 0xFF00) >> 8;
    Conbuff[0x4B] = CBeta & 0xFF;
    Conbuff[0x4C] = IsHsv & 0xFF;
    Conbuff[0x4D] = GLv & 0xFF;
    Conbuff[0x4E] = (Thresh & 0xFF00) >> 8;
    Conbuff[0x4F] = Thresh & 0xFF;
    Conbuff[0x50] = (IMPSCa & 0xFF00) >> 8;
    Conbuff[0x51] = IMPSCa & 0xFF;
    Conbuff[0x52] = (CheckLine & 0xFF00) >> 8;
    Conbuff[0x53] = CheckLine & 0xFF;
    Conbuff[0x54] = (Hrange & 0xFF00) >> 8;
    Conbuff[0x55] = Hrange & 0xFF;
    Conbuff[0x56] = (BaseADJ & 0xFF00) >> 8;
    Conbuff[0x57] = BaseADJ & 0xFF;
    Conbuff[0x58] = (TopADJ & 0xFF00) >> 8;
    Conbuff[0x59] = TopADJ & 0xFF;
    Conbuff[0x5A] = SCanMode & 0xFF;
    Conbuff[0x5B] = Tstyle & 0xFF;
    Conbuff[0x5C] = (EnumX >> 8) & 0xFF;
    Conbuff[0x5D] = EnumX & 0xFF;

    Conbuff[0x60] = (UperLow >> 8) & 0xFF;
    Conbuff[0x61] = UperLow & 0xFF;
    Conbuff[0x62] = (UperHigh >> 8) & 0xFF;
    Conbuff[0x63] = UperHigh & 0xFF;
    Conbuff[0x64] = (DownLow >> 8) & 0xFF;
    Conbuff[0x65] = DownLow & 0xFF;
    Conbuff[0x66] = (DownHigh >> 8) & 0xFF;
    Conbuff[0x67] = DownHigh & 0xFF;

    Conbuff[0x70] = (EimData >> 8) & 0xFF;
    Conbuff[0x71] = EimData & 0xFF;

    Conbuff[0x72] = (EimDataL >> 8) & 0xFF;
    Conbuff[0x73] = EimDataL & 0xFF;

    Conbuff[0x75] = ADJMode & 0xFF;

    Conbuff[0x76] = (BADJ1 >> 8) & 0xFF;
    Conbuff[0x77] = BADJ1 & 0xFF;
    Conbuff[0x78] = (BADJ2 >> 8) & 0xFF;
    Conbuff[0x79] = BADJ2 & 0xFF;

    Conbuff[0x81] = SwitchData;
    Conbuff[0x83] = SwitchValue;

    Save2File(Conbuff, fname);
    Saved = 1;
}

//------------------------------------------------------------------------------
// 声明SCanK函数
unsigned char SCanK(void);
bool isAdjusting = false; // 定义isAdjusting并初始化
//---------------------------------------------------------------
int Threadkey(int key)
{
    
    //static int trimming = 0;    // 当前校准项目索引
    static bool isUpPressed = false;
    static bool isDownPressed = false;

    switch (key)
    {

    case 27:  // ESC键
        // 释放摄像头资源并关闭所有窗口，退出程序
        //Run=0;
        cap.release();
        destroyAllWindows();
        return 1;

    case KLeft: // 左方向键
        if (calibration && CurState == 0) {
            calibration = 0; // 退出校准模式
            isAdjusting = 0;
            trimming = 0;
        }
        else {
            if (CurState < 2) CurState++;
            else CurState = 0;
        }
        break;

    case KRight: // 右方向键
        if (calibration) {
            if (!isAdjusting) {
                isAdjusting = 1; // 进入调节模式
            }
            else {
                // 保存当前调节并退出调节模式
                SaveConfig();
                isAdjusting = 0;
            }
        }
        break;

    case KUp:    // 上方向键
        isUpPressed = true;
        // 仅在非校准模式时允许进入校准
        if (isDownPressed && (CurState != 0) && !calibration) {
            calibration = 1;
            isAdjusting = 0;
            trimming = 0;
        }
        else if (calibration) {
            if (isAdjusting) {
                switch (trimming)
                {
                case 0:
                    if (Laseron) {
                        Laseron = 0;
                        bcm2835_gpio_write(LaserP, LOW);
                    }
                    else {
                        Laseron = 1;
                        bcm2835_gpio_write(LaserP, HIGH);
                    }
                    break;
                case 1:
                    foam_on = (foam_on + 1) % 4; // 在0、1、2之间循环
                    break;
                case 2:
                    if (Contrast < 3) Contrast++;
                    break;
                case 3:
                    Alpha += 0.1;
                    break;
                case 4:
                    CBeta += 1;
                    break;
                case 5:
                    if (Thresh < 255) Thresh++;
                    break;
                case 6:
                    if (Tstyle == 0) Tstyle = 1;
                    else if (Tstyle == 1) Tstyle = 8;
                    else if (Tstyle == 8) Tstyle = 16;
                    break;
                case 7:
                    IMPSCa++;
                    break;
                case 8:
                    if (CheckLine < 600) CheckLine++;
                    break;
                case 9:
                    Hrange++;
                    break;
                case 10:
                    BaseADJ++;
                    break;
                case 11:
                    TopADJ++;
                    break;
                case 12:
                    if (SCanMode < 3) SCanMode++;
                    break;
                case 13:
                    if (EnumX < 1279) EnumX += 5;
                    break;
                case 14:
                    PositionShift++;
                    break;
                case 15:
                    EimData++;
                    break;
                case 16:
                    EimDataL++;
                    break;
                case 17:
                    if (ADJMode < 3) ADJMode++;
                    break;
                case 18:
                    BADJ1++;
                    break;
                case 19:
                    BADJ2++;
                    break;
                case 20:
                    SwitchData++;
                    break;
                case 21:
                    SwitchValue++;
                    break;
                case 22:
                    SaveConfig();
                    break;
                case 23:
                    cap.release();
                    destroyAllWindows();
                    system("sudo reboot");
                    return 1;
                }
            }
            else {
                trimming = (trimming - 1 + 24) % 24;
            }
        }
        break;

    case KDown:   // 下方向键
        isDownPressed = true;
        // 仅在非校准模式时允许进入校准
        if (isUpPressed && (CurState != 0) && !calibration) {
            calibration = 1;
            isAdjusting = 0;
            trimming = 0;
        }
        else if (calibration) {
            if (isAdjusting) {
                switch (trimming)
                {
                case 0:
                    if (Laseron) {
                        Laseron = 0;
                        bcm2835_gpio_write(LaserP, LOW);
                    }
                    else {
                        Laseron = 1;
                        bcm2835_gpio_write(LaserP, HIGH);
                    }
                    break;
                case 1:
                    foam_on = (foam_on - 1 + 4) % 4; // 确保在0、1、2之间循环
                    break;
                case 2:
                    if (Contrast > 0) Contrast--;
                    break;
                case 3:
                    Alpha -= 0.1;
                    break;
                case 4:
                    CBeta -= 1;
                    break;
                case 5:
                    if (Thresh > 0) Thresh--;
                    break;
                case 6:
                    if (Tstyle == 1) Tstyle = 0;
                    else if (Tstyle == 8) Tstyle = 1;
                    else if (Tstyle == 16) Tstyle = 8;
                    break;
                case 7:
                    if (IMPSCa > 1) IMPSCa--;
                    break;
                case 8:
                    if (CheckLine > 100) CheckLine--;
                    break;
                case 9:
                    Hrange--;
                    break;
                case 10:
                    BaseADJ--;
                    break;
                case 11:
                    TopADJ--;
                    break;
                case 12:
                    if (SCanMode > 0) SCanMode--;
                    break;
                case 13:
                    if (EnumX > 5) EnumX -= 5;
                    break;
                case 14:
                    PositionShift--;
                    break;
                case 15:
                    EimData--;
                    break;
                case 16:
                    EimDataL--;
                    break;
                case 17:
                    if (ADJMode > 0) ADJMode--;
                    break;
                case 18:
                    BADJ1--;
                    break;
                case 19:
                    BADJ2--;
                    break;
                case 20:
                    SwitchData--;
                    break;
                case 21:
                    SwitchValue--;
                    break;
                }
            }
            else {
                trimming = (trimming + 1) % 24;
            }
        }
        break;

    default:
        // 仅在无按键时重置状态
        if (key != KUp && key != KDown) {
            isUpPressed = false;
            isDownPressed = false;
        }
        break;
    }
    return 0; // 返回0，表示按键未被处理
}
//------------------------------------------------------------------------------
// /**
//  * @brief 读取硬件按键状态
//  * 
//  * @return unsigned char 返回按键值
//  */
//unsigned char SCanK(void)
//{
//    unsigned char GK = 0;
//
//    if (bcm2835_gpio_lev(MenuK) != 0) GK = 81;
//    if (bcm2835_gpio_lev(UpK) != 0) GK = 82;
//    if (bcm2835_gpio_lev(DownK) != 0) GK = 84;
//    if (bcm2835_gpio_lev(SetK) != 0) GK = 83;
//    //if(GK)system("sudo xset dtms force on");
//    return GK;
//}
// 
// 
// 
#include <chrono>

uint32_t millis() {
    using namespace std::chrono;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return now.count();
}

unsigned char SCanK(void) {
    unsigned char GK = 0;
    static bool prevMenu = false, prevUp = false, prevDown = false, prevSet = false;
    static uint32_t debounceTime = 0;

    bool currMenu = bcm2835_gpio_lev(MenuK) != 0;
    bool currUp = bcm2835_gpio_lev(UpK) != 0;
    bool currDown = bcm2835_gpio_lev(DownK) != 0;
    bool currSet = bcm2835_gpio_lev(SetK) != 0;

    uint32_t currentTime = millis(); // 获取当前时间（需要定义 millis 函数）

    // 防抖逻辑
    if (currentTime - debounceTime > 50) { // 50ms 防抖
        debounceTime = currentTime;

        // 检测按键释放
        if (!currMenu && prevMenu) GK = 81;
        if (!currUp && prevUp) GK = 82;
        if (!currDown && prevDown) GK = 84;
        if (!currSet && prevSet) GK = 83;

        // 更新按键状态
        prevMenu = currMenu;
        prevUp = currUp;
        prevDown = currDown;
        prevSet = currSet;
    }

    return GK;
}
//----------------------------------------------------------------------