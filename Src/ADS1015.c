#include "main.h"
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

uint8_t devAddr = 2;
void scanAddr()
{
		for(devAddr=2;devAddr < 0xFE;devAddr +=2)
		{
			if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1,devAddr,0,0,0xFFFF))
				break;
		}
}
uint16_t CrcCal(uint8_t *data,uint8_t num)
{
	uint8_t i,j,con1,con2;
	uint16_t CrcR=0xffff, con3=0x00;
	for(i=0;i<num;i++)
	{
		//�ѵ�һ��8λ���������ݣ���ͨѶ��Ϣ֡�ĵ�һ���ֽڣ���16λ��CRC�Ĵ����ĵ�
		//8λ����򣬰ѽ������CRC�Ĵ������߰�λ���ݲ��䣻
		con1=CrcR&0xff;
		con3=CrcR&0xff00;
		CrcR=con3+data[i]^con1;
		//��CRC�Ĵ�������������һλ������λ����0����λ����������ƺ���Ƴ�λ��
		for(j=0;j<8;j++)
		{
			con2=CrcR&0x0001;
			CrcR=CrcR>>1;
			if(con2==1)
				CrcR=CrcR^0xA001;
		}
	}
	con1=CrcR>>8;//���ֽ�
	con2=CrcR&0xff;//���ֽ�
	CrcR=con2;
	CrcR=(CrcR<<8)+con1;
	return CrcR;
}
uint8_t sumCal(uint8_t *dat,uint8_t len)
{
	uint8_t sum,i;
	for(sum=0,i=0;i<len;i++)
		sum += *dat++;
	return sum;
}
void show(uint16_t val1,uint16_t val2,uint16_t val3,uint16_t val4)
{
	uint8_t txData[10];
//	sprintf(txData,"val:%d  %d\n",val1,val2);
//	HAL_UART_Transmit(&huart1,txData,strlen(txData),0xFFFF);
	txData[0] = 0xFF;
	txData[1] = val1;txData[2] = val1>>8;
	txData[3] = val2;txData[4] = val2>>8;
	txData[5] = val3;txData[6] = val3>>8;
	txData[7] = val4;txData[8] = val4>>8;
	txData[9] = sumCal(txData,9);
	HAL_UART_Transmit(&huart1,txData,10,0xFFFF);
}
uint16_t getAds1015(uint8_t ch)
{
	uint16_t value;
	uint8_t buf[2];
	uint8_t config1015[]={0x01,0xC4,0x83};//write 0x8483->Config register(0x01)
	config1015[1] = ch; //C4--ch0,D4--ch1,E4--ch2,F4--ch3
	if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1,devAddr,config1015,sizeof(config1015),0xFFFF))
	{
		if(HAL_OK == HAL_I2C_Master_Transmit(&hi2c1,devAddr,"0x00",1,0xFFFF)) //point to conversion register
		{
			//while(1)
				{
				HAL_Delay(10);
				HAL_I2C_Master_Receive(&hi2c1,devAddr,buf,2,0xFFFF);
				*((uint8_t *)&value) = buf[1];
				*(((uint8_t *)&value)+1) = buf[0];
				value = value /16;
				return value;
			}
		}
	}
}

void ads1015()
{
	uint16_t val1,val2,val3,val4;
	val1 = getAds1015(0xC4);
	val2 = getAds1015(0xD4);
	val3 = getAds1015(0xE4);
	val4 = getAds1015(0xF4);
	show(val1,val2,val3,val4);
}

extern ADC_HandleTypeDef hadc1;

uint16_t ADC_GetValue(void)
{
	uint16_t _u16Value;
	if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) 
	{		
		_u16Value = (uint16_t)HAL_ADC_GetValue(&hadc1);			//��ȡADCת�����ݣ�����Ϊ12λ���鿴�����ֲ��֪���Ĵ���Ϊ16λ�洢ת�����ݣ�
																	//�����Ҷ��룬��ת�������ݷ�ΧΪ0~2^12-1,��0~4095.	
	}
	return _u16Value;
}
void stm32ADC()
{
		uint16_t val1,val2,val3,val4;
		HAL_ADC_Start(&hadc1);  
		HAL_ADC_PollForConversion(&hadc1,50);//��ʾ�ȴ�ת����ɣ��ڶ���������ʾ��ʱʱ�䣬��λms.    
		val1 = (float)ADC_GetValue();			//��ȡADC1��channel0������ֵ
	  val1 = val1 * 3300 / 4095;
 
		HAL_ADC_Start(&hadc1);  
		HAL_ADC_PollForConversion(&hadc1,50);   
		val2 = (float)ADC_GetValue();	
		val2 = val2 * 3300 / 4095;	
	
		HAL_ADC_Start(&hadc1);  
		HAL_ADC_PollForConversion(&hadc1,50);   
		val3 = (float)ADC_GetValue();
		val3 = val3 * 3300 / 4095;
	
		HAL_ADC_Start(&hadc1);  
		HAL_ADC_PollForConversion(&hadc1,50);   
		val4 = (float)ADC_GetValue();	
		val4 = val4 * 3300 / 4095;
		show(val1,val2,val3,val4);
}