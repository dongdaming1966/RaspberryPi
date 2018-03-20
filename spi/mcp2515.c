#include	<stdio.h> 
#include	<fcntl.h> 
#include	<unistd.h>  
#include	<sys/ioctl.h>
#include	<stdint.h>  
#include	<linux/spi/spidev.h>
#include	<stdarg.h>		//Variable Arguments

#define 	SPI_PATH "/dev/spidev0.0"
#define 	dataconvert(x)	(x&0x2000)? x|0xc000:x&0x1fff 

#define		INST_RESET 0xC0
#define		INST_READ 0x03
#define		INST_RXBUFF 0x90
#define		INST_WRITE 0x02
#define		INST_TXBUFF 0x40

#define		ADDR_TXB0CTRL 0x30
#define		ADDR_TXB0SIDH 0x31
#define		ADDR_TXB0SIDL 0x32
#define		ADDR_TXB0DLC 0x35
#define		ADDR_TXB0D0 0x36

#define		SET_TXBUFF(num,id,len) 6,INST_TXBUFF+num*2,(id&0x7f8)>>3,(id&0x07)<<5,0x00,0x00,len
#define		SET_TXDATA(num,len) len+1,INST_TXBUFF+1+num*2
#define		SET_TXSEND(num) 3,INST_WRITE,ADDR_TXB0CTRL+num*16,0x0b

int spi_init(void)
{
	int	fd; 
       	uint8_t  mode   = 3; 
       	uint8_t  bits   = 8; 
       	uint32_t speed  = 1000000; 
 
       	if ((fd = open(SPI_PATH, O_RDWR))<0) 
       	{ 
        	perror("SPI Error: Can't open device."); 
               	return -1; 
       	} 

       	if((ioctl(fd, SPI_IOC_WR_MODE,             &mode )<0)| 
          (ioctl(fd, SPI_IOC_RD_MODE,             &mode )<0)| 
          (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD,&bits )<0)| 
          (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD,&bits )<0)| 
          (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)| 
          (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)<0)) 
       	{ 
               	perror("SPI: Can't be initialized."); 
               	return -1; 
       	} 
 

	return fd; 
}

int transfer(int fd, uint8_t* send, uint8_t* receive,uint8_t len)

{      

	struct spi_ioc_transfer trans=         
	{
		trans.tx_buf 		= (unsigned long) send,    
		trans.rx_buf 		= (unsigned long) receive,   
		trans.len 		= len,                     	     
		trans.delay_usecs 	= 0,                     
	};

	if (ioctl(fd, SPI_IOC_MESSAGE(1), &trans)< 0)
	{       
		perror("SPI: SPI_IOC_MESSAGE Failed");        
		return -1;     
	}     
	return 0;  

} 

uint8_t mcp_transfer(int fd, uint8_t len,...)
{
	va_list valist;
	uint8_t send[99];
	uint8_t receive[99];
	int i;

	va_start(valist,len);

//	printf("send: ");
	for(i=0;i<len;i++)
//	{
		send[i]=va_arg(valist,int);
//		printf("%x ",send[i]);
//	}
//	printf("\n");

	transfer(fd,send,receive,len);

//	printf("get: %x\n",receive[len-1]);

	return receive[len-1];
}

int mcp_init(void)
{
	int fd;

	fd=spi_init();

	mcp_transfer(fd,3,INST_WRITE,0x3f,0x87);	//CANSTAT:Configuration mode
	mcp_transfer(fd,3,INST_WRITE,0x2a,0x00);	//CNF1
	mcp_transfer(fd,3,INST_WRITE,0x29,0x82);	//CNF2
	mcp_transfer(fd,3,INST_WRITE,0x28,0x02);	//CNF3

	mcp_transfer(fd,3,INST_WRITE,0x3f,0x07);	//CANSTAT

	return fd;
}
int mcp_print(int fd, int addr, int len)
{
	int rev,i;

	for(i=0;i<len;i++)
	{
		rev=mcp_transfer(fd,3,INST_READ,addr+i,0x00);
		printf("%x = %x\n",addr+i,rev);
	}

	return 0;
}

int mcp_setdata(int fd,int addr,int len,...)
{
	va_list valist;
	int i,p;

	va_start(valist,len);

	mcp_transfer(fd,3,INST_WRITE,ADDR_TXB0SIDH,0x01);	//SIDH
	mcp_transfer(fd,3,INST_WRITE,ADDR_TXB0SIDL,0x40);	//SIDL
	mcp_transfer(fd,3,INST_WRITE,ADDR_TXB0DLC,0x01);	//DLC
	mcp_transfer(fd,3,INST_WRITE,ADDR_TXB0D0,0xff);		//D0
	mcp_transfer(fd,3,INST_WRITE,ADDR_TXB0CTRL,0x0b);	//TXB0CTRL

//	printf("addr:%d\nlen: %d\nprint: ",addr,len);
	for(i=0;i<len;i++)
	{
		p=va_arg(valist,int);
	//	printf("%d ",p);
	}
//	printf("\n");
	return 0;
}

int main()
{
	int fd,rev;
	
	fd=mcp_init();	

	mcp_transfer(fd,SET_TXBUFF(0,0x301,5));
	mcp_transfer(fd,SET_TXDATA(0,5),0x0f,0x00,0x00,0x00,0x00);
	mcp_transfer(fd,SET_TXSEND(0));

	mcp_print(fd,0x30,16);

	return 0;
}
