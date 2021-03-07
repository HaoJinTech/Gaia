#include <stdio.h>
#include "CRC.h"

int crc_CCITT_Send(int data_crc)  
{
    int data = 0, ax = 0, bx = 0, cx = 0;
    int px = 0x11021; //CRC_CCITT的生成多项式
    data = data_crc;  //data_crc为信息内容


    data <<= 16;        //信息左移16位
    ax = data >> 15;  //将前17位存在ax中，准备与多项式px做异或操作
    data <<= 17;   //除了前17位后剩下的15位


    for (cx = 15; cx > 0; cx--)
    {        
        if (((ax >> 16) & 0x1) == 0x1)
        {
            ax = ax ^ px;
        }
        ax <<= 1;        
        bx = data >> 31;        
        ax = ax + bx;        
        data <<= 1;
    }

    if (((ax >> 16) & 0x1) == 0x1)  //最后一位的异或操作
    {
        ax = ax ^ px;        
    }

    crc = ax; //crc 为根据信息内容data_crc和CRC_CCITT的生成多项式px求出的CRC码。

    printf("\n\n原始信息:%x\n", data_crc);
    data_crc <<= 16;
    data_crc += crc;
    printf("多项式信息:%x\n", px);
    printf("生成CRC校验码:%x\n", crc);
    printf("最后生成信息:%x\n", data_crc);
    return data_crc;
    
}

/* int main()
{
    FILE *fp_data, *fp_code;
    int crcnum, ch;
    fp_code = fopen("./code.txt", "w"); //加密文档
    fp_data = fopen("./data.txt", "w"); //原始文档

    printf("*****************************************************\n");
    printf("请输入原始信息,输入-1退出:");
    while (1)
    {
        scanf("%x", &ch);
        if (ch == -1)
        {
            break;
        }
        crcnum = crc_CCITT_Send(ch); //求出经过CRC校验后加密信息

        fprintf(fp_code, "%x ", crcnum);
        fprintf(fp_data, "%x ", ch);
    }
    
    printf("*****************************************************\n");
    fclose(fp_code);
    fclose(fp_data);

    system("pause");

    return 0;
} */