#include "reg52.h"
#include "absacc.h"
#define uint unsigned int
#define uchar unsigned char
#define COM XBYTE[0XFF00]					//定义8155地址
#define PA XBYTE[0XFF01]
#define PB XBYTE[0XFF02]
#define PC XBYTE[0XFF03]

sbit lup1=P1^0;	      //L1	//定义led控制端口
sbit lup2=P1^1;       //L2
sbit ldown2=P1^2;       //L3
sbit lup3=P1^3;       //L4
sbit ldown3=P1^4;       //L5
sbit ldown4=P1^5;     //L6
sbit lopen=P1^7;      //L8

sbit lcd7=P2^3;
sbit Rebuild=P2^4;
sbit emergency=P3^2;
sbit Set=P2^3;//wifi配置air
sbit cc=P2^2; 
sbit aa=P3^3;        //motor
sbit bb=P3^4;
sbit buzzer=P3^5;
sbit sound=P1^6;

uchar test;      //维护状态标志位
uchar state;		//当前电梯状态
uchar statepre;		//之前电梯状态
uchar flag;		//电梯上下标志，0下，1上
bit flag1;		//定时时间到标志
uchar counter;		//计数器
bit in1=0, in2=0, in3=0, in4=0, up1=0, up2=0, up3=0, down2=0, down3=0, down4=0;	//KEY
uchar code table[]={0xf9, 0xa4,0xb0,0x99,  0xa1, 0xc1, 0xff};        //楼层显示码表，一共是四层 0xc1=u
uchar code stop[]={0x92,0x87,0xc0,0x8c};         //stop显示
sfr T2MOD = 0xC9;



uchar usartbuf[15]={0};//接收的数据
uchar Ulen=0,Tru,num_usart,usarrtflag;//Ulen接收的数据长度,Tru 接收0xff后的0x55
/*mcu发送至wifi数据，0x55表示由后方程序决定*/
uchar idata mcu_send1[]={
	0xff,0xff,0x00,0x6f,0x02,0x55,0x00,0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x34,
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x32,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
	0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x36,0x61,0x63,0x37,0x31,0x37,0x65,0x30,
	0x37,0x33,0x65,0x32,0x34,0x64,0x39,0x30,0x39,0x32,0x35,0x36,0x34,0x35,0x65,0x35,
	0x64,0x66,0x64,0x31,0x66,0x63,0x63,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x37,0x35,0x31,0x37,0x36,0x64,0x30,0x31,0x61,0x37,0x32,0x66,0x34,0x66,
	0x31,0x63,0x38,0x34,0x31,0x30,0x64,0x31,0x66,0x32,0x37,0x32,0x66,0x61,0x61,0x34,
	0x38,0x37,0x55  
};//设备基本信息回复
uchar mcu_send2[]={
	0xff,0xff,0x00,0x05,0x04,0x55,0x00,0x00,0x55
};//wifi模组控制设备，mcu回复
uchar mcu_send3[]={
	0xff,0xff,0x00,0x07,0x04,0x55,0x00,0x00,0x03,0x55,0x55
};//wifi模组读取当前状态，mcu回复
uchar mcu_send4[]={
	0xff,0xff,0x00,0x07,0x05,0x00,0x00,0x00,0x04,0x55,0x55
};//mcu上报当前状态 
uchar mcu_send5[]={
	0xff,0xff,0x00,0x05,0x08,0x55,0x00,0x00,0x55
};//心跳，mcu回复
uchar code mcu_send6[]={
	0xff,0xff,0x00,0x06,0x09,0x00,0x00,0x00,0x02,0x11
};//通知wifi进入配置模式
uchar mcu_send7[]={
	0xff,0xff,0x00,0x05,0x0e,0x55,0x00,0x00,0x55
};//推送wifi工作状态,mcu回复
uchar mcu_send8[]={
	0xff,0xff,0x00,0x06,0x12,0x55,0x00,0x00,0x55,0x55
};//非法数据包，mcu回复
//uchar code mcu_send9[]={
//	0xff,0xff,0x00,0x05,0x17,0x00,0x00,0x00,0x1c
//};//mcu请求网络时间
uchar mcu_send10[]={
	0xff,0xff,0x00,0x05,0x10,0x55,0x00,0x00,0x55
};//重启mcu，mcu应答
uchar code mcu_send11[]={
	0xff,0xff,0x00,0x05,0x0b,0x00,0x00,0x00,0x10
};//重置wifi模组


void delay(uint xms)	   //简单延时0.1ms
{
	uint o,j;
	for(o=xms;o>0;o--)
		for(j=11;j>0;j--);
}
/***********************************************************/

void Positive_Rotation()
{
	aa = 0;
	bb = 1;

}

void Reverse_Rotation()
{
	aa = 1;
	bb = 0;

}

void Stop_Rotation()
{
	aa = 1;
	bb = 1;
	sound=1;
	delay(500);

}


void display(){
		PB=0xff;
		PA=0xfe;
		PB=table[state-1];
                delay(2);

                PB=0xff;
                PA=0xfd;
                PB=table[flag+4];
                delay(2);
}

void display1(){
		PB=0xff;
		PA=0xfe;
		PB=stop[3];
		delay(2);
								PB=0xff;
                PA=0xfd;
                PB=stop[0];
                delay(2);
					PB=0xff;
          PA=0xfb;
          PB=stop[1];
          delay(2);
		PB=0xff;
    PA=0xf7;
    PB=stop[2];
    delay(2);
}


void keyscan(){					//扫描键盘
		uchar pctemp;
		PB=0xff;
		PA=~0x01;				//PA0口输出低电平
		pctemp=PC;
		switch(pctemp&0x0f){	//PC高位置0
			case(0x0e):{
				in1=1;			//被按下后标记，亮灯，下同
				

				break;
			}
			case(0x0d):{
				in2=1;
				

				break;
			}
			case(0x0b):{
				in3=1;
				

				break;
			}
			case(0x07):{
				in4=1;
				

				break;
			}
			default: break;
		}
		PA=~0x02;				 //PA1口低电平扫描第二行
		delay(1);
		pctemp=PC;
		switch(pctemp&0x0f){
			case(0x0e):{
				up1=1;
				lup1=0;

				break;
			}
			case(0x0d):{
				up2=1;
				lup2=0;

				break;
			}
			case(0x0b):{
				up3=1;
				lup3=0;
			  aa=0;

				break;
			}

			default: break;
		}
		PA=~0x04;
		delay(1);				//PA2口低电平扫描第三行
		pctemp=PC;
		switch(pctemp&0x0f){			

			case(0x0d):{
				down2=1;
				ldown2=0;

				break;
			}
			case(0x0b):{
				down3=1;
				ldown3=0;

				break;
			}
			case(0x07):{
				down4=1;
				ldown4=0;

				break;
			}
			default: break;
		}
}


void iniclock()      //start clock
{
		TH0=60;		//TH0=(65536-50000)/256 TL0=(65536-50000)%256   50ms初值
		TL0=176;
		ET0=1;	   //T0中断允许
		TR0=1;	  //启动定时器T0
}
void run(){
		iniclock();			//开定时
		sound=0;
		while(~flag1){		//时间未到扫描键盘
			keyscan();
			display();
		}
		flag1=0;			//时间到了，清标志位，关定时器
		TR0=0;
		ET0=0;
		
}

/*mcu发送数据程序*/
void Usart_SendArrang(uchar *dat,uchar len)
{
	uchar i;
	for(i=0;i<len;i++)
	{
			ES=0;//关闭串口中断
			SBUF=dat[i];
			while(!TI);
			TI=0;//发送标志位清零
			ES=1;//打开串口中断
	}
}



/*mcu改变设备状态*/
void Control_Mcu()
{
	switch(usartbuf[9])//标志位
	{
		case 0x01:
			if(usartbuf[10]==0x01)
			{
				up1=1;
				lup1=0;
			}
			else 
				
			break;
		case 0x02:
			if(usartbuf[10]==0x02){
				up2=1;
				lup2=0;
			}
			else
				
			break;
		case 0x04:
			if(usartbuf[10]==0x04)
				{
				down2=1;
				ldown2=0;
				}
			else 
			
			break;
		case 0x08:
			if(usartbuf[10]==0x08){
				
				up3=1;
				lup3=0;
			}
			else 
				
			break;
		case 0x10:
			if(usartbuf[10]==0x10)
			{
				down3=1;
				ldown3=0;
			}
			else 
				
			break;
		case 0x20:
			if(usartbuf[10]==0x20)
				{
				down4=1;
				ldown4=0;
				}
			else 
				
			break;
		case 0x40:
			if(usartbuf[10]==0x40)
				buzzer=1;
			else 
				buzzer=0;
			break;
		case 0x80:
			if(usartbuf[10]==0x80)
				test=1;
			else 
				test=0;
			break;
	}
}
/***********mcu接收数据处理***************/
void Usart_Communication()
{
	uchar i,checksum,sn,send_8_dev;//i循环，checksum校验和，sn包序号，send_8_dev，设备当前状态，send_7_error错误信息
	if(num_usart==10)
	{
		ET2=0;
		num_usart=0;
		usarrtflag=1;
		REN=0;
		
	}
	if(usarrtflag)
	{
		sn=usartbuf[5];//包序号
		checksum=0;
		switch(usartbuf[4])//命令
		{
			case 0x01://基本信息
				mcu_send1[5]=sn;
				for(i=2;i<114;i++)
				{
					checksum=checksum+mcu_send1[i];
				}
				checksum=checksum%256;
				mcu_send1[114]=checksum;
				Usart_SendArrang(mcu_send1,115);
				break;
				
			case 0x03://控制
				if(usartbuf[3]=0x06&&usartbuf[8]==0x02)
				{
					mcu_send3[5]=sn;
					send_8_dev=~P1;
					mcu_send3[9]=send_8_dev;
					for(i=2;i<10;i++)
					{
						checksum=checksum+mcu_send3[i];
					}
					checksum=checksum%256;
					mcu_send3[10]=checksum;
					Usart_SendArrang(mcu_send3,11);
					break;
				}
				if(usartbuf[3]=0x08&&usartbuf[8]==0x01)
				{
					Control_Mcu();								//收到数据后，mcu控制设备
					
					mcu_send2[5]=sn;
					for(i=2;i<8;i++)
					{
						checksum=checksum+mcu_send2[i];
					}
					checksum=checksum%256;
					mcu_send2[8]=checksum;
					Usart_SendArrang(mcu_send2,9);			//mcu回复

					mcu_send4[5]=sn;
					send_8_dev=~P1;
					mcu_send4[9]=send_8_dev;
					for(i=2;i<10;i++)
					{
						checksum=checksum+mcu_send4[i];
					}
					checksum=checksum%256;
					mcu_send4[10]=checksum;
					Usart_SendArrang(mcu_send4,11);			//mcu主动上报设备状态					
					break;	
				}
			case 0x06:
				break;
			case 0x07://心跳
				mcu_send5[5]=sn;
				for(i=2;i<8;i++)
				{
					checksum=checksum+mcu_send5[i];
				}
				checksum=checksum%256;
				mcu_send5[8]=checksum;
				Usart_SendArrang(mcu_send5,9);
				break;	

			case 0x0a://wifi回复进入配置模式
				break;

			case 0x0c://wifi回复重置wifi模组
				break;

			case 0x0d://推送wifi模组工作状态
				mcu_send7[5]=sn;
				for(i=2;i<8;i++)
				{
					checksum=checksum+mcu_send7[i];
				}
				checksum=checksum%256;
				mcu_send7[8]=checksum;
				Usart_SendArrang(mcu_send7,9);
				break;

			case 0x0f://重启mcu
				mcu_send10[5]=sn;
				for(i=2;i<8;i++)
				{
					checksum=checksum+mcu_send10[i];
				}
				checksum=checksum%256;
				mcu_send10[8]=checksum;
				Usart_SendArrang(mcu_send10,9);
				break;	

			case 0x11://非法数据包通知
				mcu_send8[5]=sn;
				send_8_dev=usartbuf[8];
				mcu_send8[8]=send_8_dev;
				for(i=2;i<9;i++)
				{
					checksum=checksum+mcu_send8[i];
				}
				checksum=checksum%256;
				mcu_send8[9]=checksum;
				Usart_SendArrang(mcu_send8,10);
				break;
			case 0x16://可绑定模式
				break;	
		}
		Ulen=0;
		usarrtflag=0;
		REN=1;
		
	}	
}
///********key_scan*****************/
//void key_scan()
//{
//		if(emergency==0)//判断S3是否被按下
//		{
//			delay(20);//按键消抖
//			if(emergency==0)
//			{
//				  buzzer=~buzzer;
////				Usart_SendArrang(mcu_send11,9);
//	//			Usart_SendArrang(mcu_send6,10);
//				
//				while(!emergency);//松手检测
//			}	
//		}
//		
//		
//	}	
		
void main()
{
		test=0;               
 		state=1;
		statepre=1;
		flag=1;
		flag1=0;
		COM=0x43;			//intialize 8155
		TMOD=0x21;		   //定义定时器0/1为定时器模式 定时器1工作方式2
		EA=1;				  //开总中断
		EX0=1;   //开启外部中断0
		IT0=1;
		buzzer=0;
		sound=0;
		
		ES=1;//打开串口中断
		SM0 = 0;	SM1 = 1;//串口工作方式1,8位UART波特率可变
		REN = 1;//串口允许接收
		TR1 = 1;//启动定时器1
	//	TMOD=0x20;					//定时器1工作方式2
		TH1 = 0xfd;
		TL1 = 0xfd;//设置比特率9600
		
		RCAP2H=(65536-10000)/256;
		RCAP2L=(65536-10000)%256;
		TH2=RCAP2H;
		TL2=RCAP2L;
		T2CON=0;			//配置定时器2控制寄存器，这里其实不用配置，T2CON上电默认就是0
		T2MOD=0;			//配置定时器2工作模式寄存器，这里其实不用配置，T2MOD上电默认就是0
		//IE=0xA0;			//1010 0000开总中断，开外定时器2中断，可按位操作：EA=1; ET2=1;
		TR2=1;         //启动定时器T2
		ET2=1;        //定时器T2中断允许
		

	//Usart_SendArrang(mcu_send6,10);
	
	
      while(1){
		Usart_Communication();
				
//		key_scan();
				
		if (!test){		
			switch(state){				//扫描电梯所在位置
				case(1):{
	
					if(state!=statepre){	//上次的状态和本次不一样说明电梯 需要开门
						Stop_Rotation();
						lopen=0;
						run();			//按键扫描三秒钟
						lopen=1;
						in1=0;		//如果in1和up1被按下则无效
					
						up1=0;
						lup1=1;
					}
                    else{
                        if(up1){
                                up1=0;
                                lup1=1;
                                lopen=0;
                                run();
                                lopen=1;
                                }
                         }
                      in1=0;
                                       
					if(up2|down2|up3|down3|down4|in2|in3|in4){
						Positive_Rotation();
						flag=1;			//说明上行
						run();				//按键扫描三秒钟
						
						state=2;			//更新状态
						statepre=1;
						
		
					}
					else{
						statepre=state;
                        flag=2;
						keyscan();
						display();
					}
					break;
				}
				case(2):{					  //电梯到2楼

					if(state!=statepre)
                    {                          //如果前后两次状态不一样则可能需要开门
                     	if(!(
							((flag==1)&&(in3|up3|down3)&&(~up2)&&(~in2))		   //去三楼
							||((flag==1)&&(in4|down4)&&(~up2)&&(~in2))		   //去四楼
                            ||((flag==0)&&(in1||up1)&&(~down2)&&(~in2))		   //去一楼
                            )
                          )
                           {			
								
								Stop_Rotation();
								lopen=0;
								run();				//按键扫描三秒钟
														 
								lopen=1;
								in2=0;				//如果in2被按下则无效
						
								if(flag==1){			//如果上行，电梯外上行指示灯按下无效
								up2=0;
								lup2=1;
								}
								else{				//如果下行，电梯外下行指示灯按下无效
								down2=0;
								ldown2=1;
								}
                         	}
					}
                                        else{									//在二楼停
                                        	if(down2|up2){
                                                	down2=0;
                                                        up2=0;
																												
                                                        lopen=0;
                                                        run();
                                                        lopen=1;
                                                        ldown2=1;
                                                        lup2=1;
                                                	}
                                        	}
                                        in2=0;
                                       
                                        if(flag)
                                        	up2=0;
                                        else
                                        	down2=0;
					if(flag==1){
						if(down3|in3|up3|down4|in4){
							Positive_Rotation();
							flag=1;
							run();
							state=3;
							statepre=2;
						}
						else if(in1|up1){
							Reverse_Rotation();
							flag=0;
							run();
							state=1;
							statepre=2;
						 }
						 else{
						 	statepre=state;
              flag=2;
							
						 	keyscan();
						 	display();
						 }
					}
					else {
						if(up1|in1){
							Reverse_Rotation();
							flag=0;
							run();
							state=1;
							statepre=2;
						}
						else if(in3|down3|up3|in4|down4){
							
							Positive_Rotation();
							flag=1;
							run();
							state=3;
							statepre=2;
						}
						else{
							statepre=state;
              flag=2;
							keyscan();
							display();
						}
					}
					break;
				}
				case(3):{				//电梯到3楼

					if(state!=statepre)
                    {                          //如果前后两次状态不一样则可能需要开门
                     	if(!(
							((flag==0)&&(in2|up2|down2)&&(~up3)&&(~in3))		   //去二楼
							||((flag==1)&&(in4|down4)&&(~up3)&&(~in3))		   //去四楼
                            ||((flag==0)&&(in1||up1)&&(~down3)&&(~in2))		   //去一楼
                            )
                          )
                           {			
								
								Stop_Rotation();						 
								lopen=0;
								run();				//按键扫描三秒钟
								lopen=1;
								in3=0;				//如果in3被按下则无效
						
								if(flag==1){			//如果上行，电梯外上行指示灯按下无效
								up3=0;
								lup3=1;
								}
								else{				//如果下行，电梯外下行指示灯按下无效
								down3=0;
								ldown2=1;
								}
                         	}
					}
                                        else{									//在三楼停
                                        	if(down3|up3){
                                                	down3=0;
                                                        up3=0;
                                                        lopen=0;
                                                        run();
                                                        lopen=1;
                                                        ldown3=1;
                                                        lup3=1;
                                                	}
                                        	}
                                        in3=0;
                                       
                                        if(flag)
                                        	up3=0;
                                        else
                                        	down3=0;
					if(flag==1){
						if(down4|in4){
							Positive_Rotation();
							flag=1;
							run();
							state=4;
							statepre=3;
						}
						else if(in1|up1|in2|up2|down2){
							Reverse_Rotation();
							flag=0;
							run();
							state=2;
							statepre=3;
						 }
						 else{
							Stop_Rotation();
						 	statepre=state;
              flag=2;
						 	keyscan();
						 	display();
						 }
					}
					else {
						if(up1|in1|up2|in2|down2){
							Reverse_Rotation();
							flag=0;
							run();
							state=2;
							statepre=3;
						}
						else if(in4|down4){
							
							Positive_Rotation();
							flag=1; 
							run();
							state=4;
							statepre=3;
						}
						else{
							statepre=state;
              flag=2;
							keyscan();
							display();
						}
					}
					break;
				}
				case(4):{					//case4和case1类似

					keyscan();
					display();
					if(state!=statepre){
						Stop_Rotation();
						lopen=0;
						run();
						lopen=1;
						in4=0;				//如果in4和down4被按下则无效
					
						down4=0;
						ldown4=1;
					}
                                        else{
                                        	if(down4){
                                                	down4=0;
                                                        ldown4=1;
                                                        lopen=0;
                                                        run();
                                                        lopen=1;
                                                }
                                        }
                                        in4=0;
                                      
					if(in1|up1|up2|down2|in2|in3|up3|down3){
						Reverse_Rotation();
						flag=0;				//说明下行
						run();
						state=3;			//更新状态
						statepre=4;
					}
					else{
						statepre=state;
            flag=2;
						keyscan();
						display();
					}
					break;
				}
				default: break;
			}
		}
		else{
		up1=0;up2=0;up3=0;down2=0;down3=0;down4=0;
		display1();
		
		}
	}
}

void exter0() interrupt 0
{
	buzzer = ~buzzer;
	
}

void T0_time()interrupt 1
{
	TH0=(65536-50000)/256; 	
	TL0=(65536-50000)%256; //重赋初值
	counter++;		   //定时器中断次数加1
	
	if(counter==60)	 //三秒钟执行一次
	{
        	counter=0;
			flag1=1;          //三秒钟定时到标志
        }
}


/*定时器2中断（判断串口接收完成）*/
void Timer2_Int() interrupt 5
{
	TF2=0;      //T2定时器中断请求标志位
	num_usart++;
	
}
//串口中断函数
void UART() interrupt 4
{
		num_usart=0;
		ET2=1;
		if(Ulen>2&&usartbuf[Ulen-1]==0xff)
		{
			Tru=SBUF;
		}
		usartbuf[Ulen++]= SBUF;//读SBUF，读出串口接收到的数据
		RI = 0;//软件清零接收标志位	
} 

