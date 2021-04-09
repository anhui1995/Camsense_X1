### 某宝25元激光雷达的数据帧解析 Camsense_X1

#### 串口配置为：

波特率115200，8数据位，一停止位，无校验位。

#### Camsense X1 单个数据帧：

03 08 79 4E F3 B5 0B 02 3B 03 02 59 F7 01 B4 F6 01 CC F0 01 01 E8 01 B7 DC 01 2C 00 80 00 89 B7 81 3E 55 AA 

说一下，有的人可能以55为开头，这样就有四个固定的头，但这样没有尾部，感觉不太可能，
也可能是三个固定头，一个固定尾，但这都不是重点，这个只是划分数据帧的一个依据。头尾对半分，谁都不吃亏。

#### 数据帧解析：

##### 总体描述

每个数据帧包含8个距离信息，每个距离所对应的角度为 angle = startAngle+stepAngle*N； 

其中（N=0,1，……，7）；stepAngle = (endAngle - startAngle)/8.0;（if(endAngle<startAngle)  endAngle += 360.0;）
每个距离信息包括距离和可信度两个数据。

每一个数据帧共36字节，包含的信息依次为：

数据头两字节，固定值：0x03，0x08。 buffer[0] == 0x03 ，buffer[1] == 0x08；

转速信息一字节：buffer[2]；

未知信息一字节，固定值：0x4E。buffer[3] == 0x4E；根据淘宝的一位买家的疑问，这里可能不是固定的0X4E，这里感谢这位买家（ID:西伯****狗）。
然后解释一下，我这里是根据，正常运行、全部遮挡运行，卡住不让转，加阻力使其转速过低，这几种情况下，这一字节都为）0x4E,所以我才说的是固定值。

##### 数组分析

此数据帧的起始角度两字节：startAngle = (buffer[5]<<8 | buffer[4])/64.0 - 640.0；


距离信息一 distance = buffer[7]<<8 | buffer[6];  quality = buffer[8];

距离信息二 distance = buffer[10]<<8 | buffer[9];  quality = buffer[11];

距离信息三 distance = buffer[13]<<8 | buffer[12];  quality = buffer[14];

距离信息四 distance = buffer[16]<<8 | buffer[15];  quality = buffer[17];

距离信息五 distance = buffer[19]<<8 | buffer[18];  quality = buffer[20];

距离信息六 distance = buffer[22]<<8 | buffer[21];  quality = buffer[23];

距离信息七 distance = buffer[25]<<8 | buffer[24];  quality = buffer[26];

距离信息八 distance = buffer[28]<<8 | buffer[27];  quality = buffer[29];


此数据帧的结束角度两字节：endAngle =  (buffer[31]<<8 | buffer[30])/64.0 - 640.0 ；
校验位两字节：buffer[32]，buffer[33]；
数据尾两字节，固定值：0x55，0xAA。 buffer[34] == 0x55 ，buffer[35] == 0xAA。


具体代码也有，另一个文件就是我的数据接收以及处理函数，C语言，STM32F103ZET6单片
