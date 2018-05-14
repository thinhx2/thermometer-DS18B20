#include"temperature.h"
#include"stdio.h"


/*******************************************************************************
* 函数名         : Delay1ms
* 函数功能		   : 延时函数
* 输入           : 延时时长，单位ms
* 输出         	 : 无
*******************************************************************************/

void Delay1ms(uint c)
{
	uchar a,b;
	for (; c>0; c--)
	{
		 for (b=199;b>0;b--)
		 {
		  	for(a=1;a>0;a--);
		 }      
	}
}
/*******************************************************************************
* 函数名         : DS18B20Init
* 函数功能		   : 初始化
* 输入           : 无
* 输出         	 : 初始化成功返回1，失败返回0
*******************************************************************************/

uchar DS18B20Init()
{
	uint i;
	DSPORT=0;			 //将总线拉低480us~960us
	i=70;	
	while(i--);//延时642us
	DSPORT=1;			//然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线拉低
	i=0;
	while(DSPORT)	//等待DS18B20拉低总线
	{
		i++;
		if(i>5000)//等待>5MS
			return 0;//初始化失败	
	}
	return 1;//初始化成功
}

/*******************************************************************************
* 函数名         : DS18B20WriteByte
* 函数功能		   : 向18B20写入一个字节
* 输入           : dat
* 输出         	 : 无
*******************************************************************************/

void DS18B20WriteByte(uchar dat)
{
	uint i,j;
	for(j=0;j<8;j++)
	{
		DSPORT=0;			//每写入一位数据之前先把总线拉低1us
		i++;
		DSPORT=dat&0x01; //然后写入一个数据，从最低位开始
		i=6;
		while(i--); //延时68us，持续时间最少60us
		DSPORT=1;	//然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		dat>>=1;
	}
}
/*******************************************************************************
* 函数名         : DS18B20ReadByte
* 函数功能		 : 读取一个字节
* 输入           : 无
* 输出         	 : 1个字节数据
*******************************************************************************/


uchar DS18B20ReadByte()
{
	uchar byte,bi;
	uint i,j;	
	for(j=8;j>0;j--)
	{
		DSPORT=0;//先将总线拉低1us
		i++;
		DSPORT=1;//然后释放总线
		i++;
		i++;//延时6us等待数据稳定
		bi=DSPORT;	 //读取数据，从最低位开始读取
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte=(byte>>1)|(bi<<7);						  
		i=4;		//读取完之后等待48us再接着读取下一个数
		while(i--);
	}				
	return byte;
}
/*******************************************************************************
* 函数名         : DS18B20TransfTemp
* 函数功能		   : 让18b20开始转换温度
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/

void  DS18B20TransfTemp()
{
	DS18B20Init();
	Delay1ms(1);
	DS18B20WriteByte(0xcc);		//跳过ROM操作命令		 
	DS18B20WriteByte(0x44);	    //温度转换命令
	Delay1ms(100);	//等待转换成功
   
}
/*******************************************************************************
* 函数名         : DS18B20ReadTempCom
* 函数功能		   : 发送读取温度命令
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/

void  DS18B20ReadTempCom()
{	

	DS18B20Init();
	Delay1ms(1);
	DS18B20WriteByte(0xcc);	 //跳过ROM操作命令
	DS18B20WriteByte(0xbe);	 //发送读取温度命令
}
/*******************************************************************************
* 函数名         : DS18B20ReadTemp
* 函数功能		 : 读取温度
* 输入           : 无
* 输出         	 : 温度值数字量（实际温度的补码）
*******************************************************************************/

int DS18B20ReadTemp()
{
	int temp=0;
	uchar tmh,tml;
	DS18B20TransfTemp();		//先写入转换命令
	DS18B20ReadTempCom();		//然后等待转换完后发送读取温度命令
	tml=DS18B20ReadByte();		//读取温度值共16位，先读低字节
	tmh=DS18B20ReadByte();		//再读高字节
	temp=tmh;
	temp<<=8;
	temp|=tml;
	return temp;
}

/*******************************************************************************
* 函数名         : DS18B20toString
* 函数功能		 : 读取温度值并转化为浮点数（模拟量，保留两位小数）
* 输入           : 无
* 输出         	 : float 温度 （温度值模拟量，保留两位小数)
*******************************************************************************/
float DS18B20NumTemp()
{
	float Atemp;
	int Dtemp=DS18B20ReadTemp();
	if(Dtemp< 0)				//当温度值为负数
  	{
		//因为读取的温度是实际温度的补码，所以减1，再取反求出原码
		Dtemp=Dtemp-1;
		Dtemp=~Dtemp;
		Atemp=Dtemp*0.0625;
  	}
 	else
  	{			
		//如果温度是正的那么，那么正数的原码就是补码它本身
		Atemp=Dtemp*0.0625;
	}
	return 	 Atemp;
}

/*******************************************************************************
* 函数名         : DS18B20toString
* 函数功能		 : 读取温度值并转化为字符串（模拟量，保留两位小数）
* 输入           : float 温度值
* 输出         	 : 字符串指针 （温度值模拟量，保留两位小数)
*******************************************************************************/
uchar* DS18B20toString(float numtemp)
{
	int i=0,j=0;
	uchar temp_str[7]={'\0'};
	uchar test[7];
	int itemp= numtemp*100+0.5;
	if(itemp< 0)				//当温度值为负数
		temp_str[i]='-';
	if(itemp>=10000)
		temp_str[i++] = itemp / 10000+'0';		 //百位
	if(itemp>=1000)
		temp_str[i++] =itemp % 10000 / 1000+'0';	 //十位
	temp_str[i++] =	 itemp % 1000 / 100+'0';
	temp_str[i++] =	 '.';
	temp_str[i++] =	 itemp % 100 / 10+'0';
	temp_str[i++] =	 itemp % 10+'0';
	temp_str[i] = '\0';
	do							 //严格C语法不需要这部分，但keil好像存在一些问题
	{
		test[j]=temp_str[j];
		j++;	
	}while(temp_str[j]!='\0');
	test[j]='\0';
	return test;
}

