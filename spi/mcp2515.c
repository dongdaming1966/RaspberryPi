#include	<stdio.h> 
#include	<fcntl.h> 
#include	<unistd.h>  
#include	<sys/ioctl.h>
#include	<stdint.h>  
#include	<linux/spi/spidev.h> 
#define 	SPI_PATH "/dev/spidev0.0"
#define 	dataconvert(x)	(x&0x2000)? x|0xc000:x&0x1fff 

uint8_t	received_data[12];

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

uint8_t mcp_transfer(int fd, uint8_t instruction, uint8_t addr, uint8_t data)
{
	uint8_t send[9]={0x03,0x30,0x7b};
	uint8_t receive[9];
	int i;
	send[0]=instruction;
	send[1]=addr;
	send[2]=data;
	
	transfer(fd,send,receive,3);	

	return receive[2];
}


int main()
{
	int fd,rev;
	
	fd=spi_init();

	mcp_transfer(fd,0x02,0x3f,0x87);	//CANSTAT:Configuration mode
	mcp_transfer(fd,0x02,0x2a,0x00);	//CNF1
	mcp_transfer(fd,0x02,0x29,0x82);	//CNF2
	mcp_transfer(fd,0x02,0x28,0x02);	//CNF3
	mcp_transfer(fd,0x02,0x31,0x01);	//SIDH
	mcp_transfer(fd,0x02,0x32,0x40);	//SIDL
	mcp_transfer(fd,0x02,0x35,0x01);	//DLC
	mcp_transfer(fd,0x02,0x36,0xff);	//D0
	mcp_transfer(fd,0x02,0x30,0x0b);	//TXB0CTRL
	mcp_transfer(fd,0x02,0x3f,0x07);	//CANSTAT


	rev=mcp_transfer(fd,0x03,0x29,0x00);
	

	printf("rev = %x\n",rev);

	return 0;
}
