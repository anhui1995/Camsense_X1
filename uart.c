/*
使用 STM32F103ZET6 单片机解析
*/


u8 Res2;

u8 buffer[36]={0};
u16 bufferNum=0;

u8 get=0;

double startAngle = 0;
double stepAngle = 0;

typedef struct {
	double angle;   //对应角度
	u16 distance;   //对应距离
	u8 quality;     //对应数据可信度
} dataBuffer;

dataBuffer   bufferPlus[260];
u16 bufferPlusNum=0;

u16 angleBuffer[370]={0};
u16 angleBufferNum=0;

u16 resultBuffer[40]={0};
u8 resultBufferNum=0;





//只初始化RX
void InitUsart2Gpio(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}



void InitUsart2(void)
{
  USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
//   //Usart1 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	
	USART_Cmd(USART2, ENABLE);
}




//  两个数据帧的时间间隔大约为3.53ms
void USART2_IRQHandler(void)                	//串口2中断服务程序
	{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		Res2 =USART_ReceiveData(USART2);
		
		switch(bufferNum){
		
			case 0 : if(Res2==0x03){ //数据头1

					bufferNum++;
				} 
				break;
				
			case 1 : if(Res2==0x08){//数据头2 
					bufferNum++;
				}  else {
					bufferNum = 0;
				} 
				break;
				
			case 2 :  //转速信息，一般用不到
				buffer[bufferNum++] = Res2;
				break;
				
			case 3 : if(Res2==0x4E){
					bufferNum++;
				}  else {
					bufferNum = 0;
				} 
				break;

			case 31: //如需接收完整的数据帧则此处应为35，最后四字节数据为校验和数据尾，为节省时间，所以就不接收了。
					 //我这个程序运行在STM32F103ZET6上，如果读取到35，会出现丢帧的现象。
				buffer[bufferNum] = Res2;
				//开始数据转换，关中断，防止中断套娃。只要在下一个数据帧到来之前打开就不会漏掉数据。
				USART_Cmd(USART2, DISABLE);
				
			
				/*  这俩玩意不固定，，是解析这个模块的数据中最大的坑。。。
				起始角度 startAngle = (buffer[5]<<8 | buffer[4])/64.0 - 640.0 ;
				结束角度 endAngle =  (buffer[31]<<8 | buffer[30])/64.0 - 640.0 ;
				*/
				
				startAngle = (buffer[5]<<8 | buffer[4])/64.0 - 640.0 ;  //起始角度
				stepAngle = (buffer[31]<<8 | buffer[30])/64.0 - 640.0 ; //结束角度
				if(stepAngle<startAngle)  stepAngle += 360.0;
				
				stepAngle = (stepAngle - startAngle)/8.0;     //角度步进，不固定！！！！！！！！也就是说，每一个数据帧横跨的角度不固定。。。。。
				
				if(get==0){  //这里是根据我的需要只读取角度值为80°-272°，角度大于80时开始记录
					if(startAngle>80 && startAngle<180){
							get=1;
							bufferPlusNum=0;
						}
				}
				else { //解析此帧数据的八个距离信息
				
					bufferPlus[bufferPlusNum].angle = startAngle;
					bufferPlus[bufferPlusNum].distance = buffer[7]<<8 | buffer[6];
					bufferPlus[bufferPlusNum++].quality = buffer[8];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle;
					bufferPlus[bufferPlusNum].distance = buffer[10]<<8 | buffer[9];
					bufferPlus[bufferPlusNum++].quality = buffer[11];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*2;
					bufferPlus[bufferPlusNum].distance = buffer[13]<<8 | buffer[12];
					bufferPlus[bufferPlusNum++].quality = buffer[14];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*3;
					bufferPlus[bufferPlusNum].distance = buffer[16]<<8 | buffer[15];
					bufferPlus[bufferPlusNum++].quality = buffer[17];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*4;
					bufferPlus[bufferPlusNum].distance = buffer[19]<<8 | buffer[18];
					bufferPlus[bufferPlusNum++].quality = buffer[20];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*5;
					bufferPlus[bufferPlusNum].distance = buffer[22]<<8 | buffer[21];
					bufferPlus[bufferPlusNum++].quality = buffer[23];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*6;
					bufferPlus[bufferPlusNum].distance = buffer[25]<<8 | buffer[24];
					bufferPlus[bufferPlusNum++].quality = buffer[26];
					
					bufferPlus[bufferPlusNum].angle = startAngle+stepAngle*7;
					bufferPlus[bufferPlusNum].distance = buffer[28]<<8 | buffer[27];
					bufferPlus[bufferPlusNum++].quality = buffer[29];
					
					//printf("-%d",bufferPlusNum);
					
					if(startAngle>272){ //角度大于272时结束记录，然后这里面就是我自己的二次数据整理，可忽略

						for(j=0;j<bufferPlusNum;j++){ //将距离信息保存到数组angleBuffer中，下标即角度，数据即距离，方便调用
							
							if(bufferPlus[j].angle>=360) angleBufferNum = (u16)(bufferPlus[j].angle) - 360;
							// 这里注意下，一圈的最后一帧数据的结束角度可能小于起始角度，也就是跨过了0°，在上面的处理中，
							// 这一帧得到的数据中，角度就会出现大于360°的情况，所以要减去360°。
							else angleBufferNum = (u16)(bufferPlus[j].angle);
							
							if(bufferPlus[j].quality == 0) angleBuffer[angleBufferNum] = 9999;
							else angleBuffer[angleBufferNum] = bufferPlus[j].distance;

							delay_us(50);
						}
						resultBufferNum=0;
						
						
						
						for(j=88;j<273;j+=5){ //这是我自己的一个数据合并，
							if(angleBuffer[j] < angleBuffer[j+1]) resultBuffer[resultBufferNum] = angleBuffer[j];
							else resultBuffer[resultBufferNum] = angleBuffer[j+1];
							if(angleBuffer[j+2] < resultBuffer[resultBufferNum]) resultBuffer[resultBufferNum] = angleBuffer[j+2];
							if(angleBuffer[j+3] < resultBuffer[resultBufferNum]) resultBuffer[resultBufferNum] = angleBuffer[j+3];
							if(angleBuffer[j+4] < resultBuffer[resultBufferNum]) resultBuffer[resultBufferNum] = angleBuffer[j+4];
							resultBufferNum++; //最终是38
							
						}

						
						get=0;
				//		printf("\n%4d - %4d - %4d - %4d",angleBuffer[0],angleBuffer[1],angleBuffer[2],angleBuffer[3]);
					}
					
				}

				USART_Cmd(USART2, ENABLE);
			
				bufferNum = 0;  //此帧数据处理结束，bufferNum要归零。。。
				break;
				
			default: //这里缓存数据，用default节省时间，
				buffer[bufferNum++] = Res2;
			break;
				
		}
	
		
		
	 } 

} 


