#include "rc522.h"


#define 	MAXRLEN 18
#define   RC522_DELAY()  delay_us(5)


void MFRC522_Init(void)
{
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
    CKCUClock.Bit.PB = 1;         
    CKCUClock.Bit.PA = 1;         
    CKCUClock.Bit.AFIO = 1;       
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    // SDA (PB0), SCK (PB1), MOSI (PB2), RST (PB6) 设为输出
    AFIO_GPxConfig(GPIO_PB, GPIO_PIN_0, AFIO_FUN_GPIO);
    AFIO_GPxConfig(GPIO_PB, GPIO_PIN_1, AFIO_FUN_GPIO);
    AFIO_GPxConfig(GPIO_PB, GPIO_PIN_2, AFIO_FUN_GPIO);
    AFIO_GPxConfig(GPIO_PB, GPIO_PIN_6, AFIO_FUN_GPIO);
    
    GPIO_DirectionConfig(HT_GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6, GPIO_DIR_OUT);

    // MISO (PA3) 设为输入
    AFIO_GPxConfig(GPIO_PA, GPIO_PIN_3, AFIO_FUN_GPIO);
    GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_3, GPIO_DIR_IN);
    GPIO_InputConfig(HT_GPIOA, GPIO_PIN_3, ENABLE);

    MFRC522_SDA_H;
    MFRC522_SCK_H;
}
	
// 软件模拟SPI读
u8 Read_MFRC522(u8 Address)
{
    u8 i, ucAddr, ucResult = 0;
    MFRC522_SCK_L;
    MFRC522_SDA_L;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    for(i=8; i>0; i--)
    {
        if(ucAddr & 0x80) MFRC522_MOSI_H; else MFRC522_MOSI_L;
        RC522_DELAY();
        MFRC522_SCK_H;
        RC522_DELAY();
        ucAddr <<= 1;
        MFRC522_SCK_L;
    }

    for(i=8; i>0; i--)
    {
        MFRC522_SCK_H;
        RC522_DELAY();
        ucResult <<= 1;
        if(MFRC522_MISO_READ) ucResult |= 1;
        MFRC522_SCK_L;
        RC522_DELAY();
    }
    MFRC522_SDA_H;
    MFRC522_SCK_H;
    return ucResult;
}
// 软件模拟SPI写
void Write_MFRC522(u8 Address, u8 value)
{  
    u8 i, ucAddr;
    MFRC522_SCK_L;
    MFRC522_SDA_L;
    ucAddr = (Address << 1) & 0x7E;

    for(i=8; i>0; i--)
    {
        if(ucAddr & 0x80) MFRC522_MOSI_H; else MFRC522_MOSI_L;
        RC522_DELAY();
        MFRC522_SCK_H;
        RC522_DELAY();
        ucAddr <<= 1;
        MFRC522_SCK_L;
    }

    for(i=8; i>0; i--)
    {
        if(value & 0x80) MFRC522_MOSI_H; else MFRC522_MOSI_L;
        RC522_DELAY();
        MFRC522_SCK_H;
        RC522_DELAY();
        value <<= 1;
        MFRC522_SCK_L;
    }
    MFRC522_SDA_H;
    MFRC522_SCK_H;
}

char MFRC522_Reset(void) 
{
    MFRC522_RST_H;
    delay_ms(1);
    MFRC522_RST_L;
    delay_ms(1);
    MFRC522_RST_H;
    delay_ms(1);
    Write_MFRC522(CommandReg, 0x0F);
    while(Read_MFRC522(CommandReg) & 0x10);
    
    Write_MFRC522(ModeReg, 0x3D);            
    Write_MFRC522(TReloadRegL, 30);           
    Write_MFRC522(TReloadRegH, 0);
    Write_MFRC522(TModeReg, 0x8D);
    Write_MFRC522(TPrescalerReg, 0x3E);
    Write_MFRC522(TxAutoReg, 0x40);
    return MI_OK;
}
/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg,tmp | mask);  // set bit mask
}



/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & ~mask);  // clear bit mask
} 


char MFRC522_ToCard(u8 Command, u8 *pInData, u8 InLenByte, u8 *pOutData, unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    u8 irqEn = 0x00, waitFor = 0x00, lastBits, n;
    unsigned int i;

    if (Command == PCD_AUTHENT) { irqEn = 0x12; waitFor = 0x10; }
    else if (Command == PCD_TRANSCEIVE) { irqEn = 0x77; waitFor = 0x30; }

    Write_MFRC522(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    Write_MFRC522(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i=0; i<InLenByte; i++) { Write_MFRC522(FIFODataReg, pInData[i]); }
    Write_MFRC522(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE) { SetBitMask(BitFramingReg, 0x80); }

    // 【关键修改】大幅增加超时等待时间
    i = 60000; 
    do {
        n = Read_MFRC522(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));

    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0) {
        if (!(Read_MFRC522(ErrorReg) & 0x1B)) {
            status = MI_OK;
            if (Command == PCD_TRANSCEIVE) {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits) *pOutLenBit = (n - 1) * 8 + lastBits;
                else *pOutLenBit = n * 8;
                if (n == 0) n = 1;
                if (n > MAXRLEN) n = MAXRLEN;
                for (i=0; i<n; i++) pOutData[i] = Read_MFRC522(FIFODataReg);
            }
        }
    }
    return status;
}

//开启天线  
//每次启动或关闭天线发射之间应至少有1ms的间隔
void MFRC522_AntennaOn(void)
{
    unsigned char i;
    i = Read_MFRC522(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


//关闭天线
void MFRC522_AntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}


//功能描述用MF522计算CRC
//输入参数pIndata--要读数CRC的数据len--数据长度pOutData--计算的CRC结果
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    Write_MFRC522(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   Write_MFRC522(FIFODataReg, *(pIndata+i));   }
    Write_MFRC522(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = Read_MFRC522(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}


char MFRC522_Request(u8 req_code, u8 *pTagType)
{
   char status;  
   unsigned int unLen;
   u8 ucComMF522Buf[MAXRLEN]; 

   ClearBitMask(Status2Reg, 0x08);
   Write_MFRC522(BitFramingReg, 0x07);
   
   ucComMF522Buf[0] = req_code;
   status = MFRC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
   if ((status == MI_OK) && (unLen == 0x10)) {
       *pTagType = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   } else { status = MI_ERR; }
   return status;
}

//功    能：防冲突检测读取选中卡片的卡序列号
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK 
char MFRC522_Anticoll(u8 *pSnr)
{
    char status;
    u8 i, snr_check = 0;
    unsigned int unLen;
    u8 ucComMF522Buf[MAXRLEN]; 

    ClearBitMask(Status2Reg, 0x08);
    Write_MFRC522(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = MFRC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);
    if (status == MI_OK) {
    	 for (i=0; i<4; i++) {
             *(pSnr+i) = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i]) status = MI_ERR;
    }
    SetBitMask(CollReg, 0x80);
    return status;
}
/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char MFRC522_SelectTag(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = MFRC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char MFRC522_AuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = MFRC522_ToCard(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char MFRC522_Read(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = MFRC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}


/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char MFRC522_Write(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = MFRC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = MFRC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}


