#include "stm32f4xx.h"                  // Device header
#include "AT24C02.h"
#include "IIC.h"

int8_t AT24C02_pagewrite(uint8_t slave,uint8_t address,uint8_t *buff,uint8_t length)
{
	IIC_start();
	IIC_sendbyte(slave);
	if(0  == IIC_waitack())
	{
		IIC_stop();
	  return -1;
	}
	
	IIC_sendbyte(address);
	if(0  == IIC_waitack())
	{
		IIC_stop();
	  return -2;
	}
	
	while(length--)
	{
		IIC_sendbyte(*buff++);
		if(0  == IIC_waitack())
		{
			IIC_stop();
			return -3;
		}
	}
	IIC_stop();
	return 0;
}

int8_t AT24C02_randomread(uint8_t slave,uint8_t address,uint8_t *buff,uint8_t length)
{
	IIC_start();
	IIC_sendbyte(slave);
	if(0  == IIC_waitack())
	{
		IIC_stop();
	  return -4;
	}
	IIC_sendbyte(address);
	if(0  == IIC_waitack())
	{
		IIC_stop();
	  return -5;
	}
	
	
	IIC_start();
	IIC_sendbyte(slave | 0x01);
	if(0  == IIC_waitack())
	{
		IIC_stop();
	  return -6;
	}
	length = length -1;
	while(length--)
	{
	  *buff++ =  IIC_revicebyte();
		IIC_sendack(1);
	}
	
	*buff++ =  IIC_revicebyte();
	IIC_sendack(0);
	IIC_stop();
	return 0;
}






