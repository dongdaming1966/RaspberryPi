#include	<stdio.h> 
#include	<fcntl.h> 
#include	<unistd.h>  
#include	<sys/ioctl.h>
#include	<stdint.h>  
#include	<linux/spi/spidev.h> 
#define 	SPI_PATH "/dev/spidev0.0"
#define 	dataconvert(x)	(x&0x2000)? x|0xc000:x&0x1fff 

struct temp 
{
	short  SUPPLY_OUT;
	short  XGYRO_OUT;
	short  YGYRO_OUT;
	short  ZGYRO_OUT;
	short  XACCL_OUT;
	short  YACCL_OUT;
	short  ZACCL_OUT;
	short  XMAGN_OUT;
	short  YMAGN_OUT;
	short  ZMAGN_OUT;
	short  TEMP_OUT;
	short  AUX_OUT;
}burst_data;

unsigned int	fd; 
uint16_t		received_data[12];

int spi_init(void)
{
       uint8_t  mode   = 3; 
       uint8_t  bits   = 8; 
       uint32_t speed  = 500000; 
 
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
 

	return 0; 
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


void imu_read(int16_t data[2]) 
{ 
       	uint8_t burst[24]={0x36,0x08}; 
       	uint8_t  buff[24]; 
	uint16_t temp; 
	int16_t result;
	int i;

	transfer(fd,burst,buff,2);
	usleep(10000);

    	
	for(i=0;i<2;i++)	printf("%x\t",buff[i]);
	
	printf("\n");

}


int main()
{
	uint8_t burst[]={0xb4,0x02};
       	uint8_t  buff[24]; 
	int16_t data[2];
	
//	struct temp burst_data;
	
//	wiringPiSetup();
//	pinMode(0,OUTPUT);
//	digitalWrite(0,HIGH);
	spi_init();
	
//	transfer(fd,burst,buff,4);
	usleep(10000);
	while(1) 
		imu_read(data);
}
