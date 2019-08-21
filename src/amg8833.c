#include "amg8833.h"

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);
uint16_t Calculate_color(int16_t value);

// pamet pro pixelova data
int16_t irarray[IRARRAY];
int32_t irarray1024[1024];
int16_t mintemp;
int16_t maxtemp;
static uint8_t interpolationmode = INTMODE0;

void Amg8833_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Turn on peripherals.
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	// Configure I2C SCL and SDA pins.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Configure INT pin.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

	// Reset I2C.
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

	// Configure I2C.
	uint32_t pclk1 = 42000000;

	I2C1 ->CR2 = pclk1 / 1000000; // Configure frequency and disable interrupts and DMA.
	I2C1 ->OAR1 = I2C_OAR1_ADDMODE | 0x33;

	// Configure I2C speed in standard mode.
	const uint32_t i2c_speed = 100000;
	int ccrspeed = pclk1 / (i2c_speed * 2);
	if (ccrspeed < 4)
	{
		ccrspeed = 4;
	}
	I2C1 ->CCR = ccrspeed;
	I2C1 ->TRISE = pclk1 / 1000000 + 1;

	I2C1 ->CR1 = I2C_CR1_ACK | I2C_CR1_PE; // Enable and configure the I2C peripheral.


	// zaloz sluzbu vycitani teplotni mapy
	if(Scheduler_Add_Task(Temperaturemap_service, 0, TEMPERATUREMAP_PERIOD) == SCH_MAX_TASKS)// periodicka, 100ms
	{
		// chyba pri zalozeni service
	}

	// zaloz sluzbu vykresleni skaly
	if(Scheduler_Add_Task(Scale_service, 0, SCALE_PERIOD) == SCH_MAX_TASKS)// periodicka, 100ms
	{
		// chyba pri zalozeni service
	}

	// zapni vyssi kmitocet obrazu na 10Hz
	Amg8833_WriteCMD(FPSCREG, FPS10);

	// zapni plovouci prumer
	Amg8833_WriteCMD(RESMAMOD, VAL1);
	Amg8833_WriteCMD(RESMAMOD, VAL2);
	Amg8833_WriteCMD(RESMAMOD, VAL3);
	Amg8833_WriteCMD(MAMOD,    FLOATAVR);
	Amg8833_WriteCMD(RESMAMOD, VAL4);

	Amg8833_Scale_init();
	Amg8833_Interpolation_init();
	laser_init();										// inicializace laseroveho ukazovatka
}

/* This function issues a start condition and
 * transmits the slave address + R/W bit
 *
 * Parameters:
 * 		I2Cx --> the I2C peripheral e.g. I2C1
 * 		address --> the 7 bit slave address
 * 		direction --> the tranmission direction can be:
 * 						I2C_Direction_Tranmitter for Master transmitter mode
 * 						I2C_Direction_Receiver for Master receiver
 */
void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
	// wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Send I2C1 START condition
	I2C_GenerateSTART(I2Cx, ENABLE);

	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send slave Address for write
	I2C_Send7bitAddress(I2Cx, address, direction);

	/* wait for I2C1 EV6, check if
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */
	if(direction == I2C_Direction_Transmitter){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if(direction == I2C_Direction_Receiver){
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/* This function transmits one byte to the slave device
 * Parameters:
 *		I2Cx --> the I2C peripheral e.g. I2C1
 *		data --> the data byte to be transmitted
 */
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device
 * and acknowledges the byte (requests another byte)
 */
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
	// enable acknowledge of recieved data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the recieved data
 */
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
	// disabe acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void I2C_stop(I2C_TypeDef* I2Cx){
	// Send I2C1 STOP Condition
	I2C_GenerateSTOP(I2Cx, ENABLE);
}

void Amg8833_Scan(uint16_t size)
{
	mintemp = 0x7FFF;
	maxtemp = 0x8000;

	// analyzuje nejnizsi a nejvyssi teplotu v mape a dle toho rozlozi stupnici

	if (size == IRDIMX*IRDIMY)
	{
		for (uint8_t i = 0; i < (size-1); i++)
		{
			if (irarray[i] > maxtemp) { maxtemp = irarray[i]; }
			if (irarray[i] < mintemp) { mintemp = irarray[i]; }
		}
	}
	else if (size == IRDIMX1024*IRDIMY1024)
	{
		for (uint16_t i = 0; i < (size-1); i++)
		{
			if (irarray1024[i] > maxtemp) { maxtemp = irarray1024[i]; }
			if (irarray1024[i] < mintemp) { mintemp = irarray1024[i]; }
		}
	}
}

void Amg8833_WriteCMD(uint8_t addr, uint8_t byte)
{
	I2C_Cmd(I2C1, DISABLE);
	I2C_Cmd(I2C1, ENABLE);

	I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); 	// start a transmission in Master transmitter mode
	I2C_write(I2C1, addr); 										  	// write addr to the slave
	I2C_write(I2C1, byte); 										  	// write byte to the slave
	I2C_stop(I2C1); 											  	// stop the transmission
}

void Amg8833_Process256(void)
{
	uint8_t received_data[2*IRARRAY];
	int16_t temperature;
	uint8_t position;
	uint8_t mpos;
	uint8_t npos;

	I2C_Cmd(I2C1, DISABLE);
	I2C_Cmd(I2C1, ENABLE);

	// random access reading = write addr + read only one data byte
	for (uint8_t byte = 0; byte < 2*MAXPIXREAD; byte++)
	{
		I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); 	// start a transmission in Master transmitter mode
		I2C_write(I2C1, 0x80+byte); 								  	// write one byte to the slave
		I2C_stop(I2C1); 											  	// stop the transmission

		I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver);		// start a transmission in Master receiver mode
		received_data[byte] = I2C_read_nack(I2C1); // read one byte and do not request another byte
	}

	mpos = 0; npos = 0;

	for (uint8_t byte = 0; byte < MAXPIXREAD; byte++)					// prochazi nacteny buffer
	{
		position = byte << 1;

		// temperature je 12 bitova hodnota s rozlisenim na 0,25st. C
		temperature = received_data[position+1] << 8 | received_data[position];
		//irarray[byte] = temperature >> 2;		// prevod na 8 bitove cislo

		// musime ukladat na pozice 0,2,4,...14, 32,34,36,...46, 64,66...
		//irarray[byte] = temperature;			// prevod na 16 bitove cislo

		// vzorec na umisteni prvku pres jednu pozici v sloupcich i radcich
		irarray[32*mpos+2*npos] = temperature;
		npos++;		// obnov pozici pro sloupce
		if (npos > 7)	// pokud jsme na poslednim sloupci, nastav zase sloupec 0
		{
			mpos++;		// pro novy sloupec 0 nastav dalsi radek
			npos = 0;
		}
	}
}

void Amg8833_Process(uint8_t mode)
{
	uint8_t received_data[2*IRARRAY];
	int16_t temperature;
	uint8_t position;
	uint8_t mpos;
	uint8_t npos;

	I2C_Cmd(I2C1, DISABLE);
	I2C_Cmd(I2C1, ENABLE);

	// random access reading = write addr + read only one data byte
	for (uint8_t byte = 0; byte < 2*MAXPIXREAD; byte++)
	{
		I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter); 	// start a transmission in Master transmitter mode
		I2C_write(I2C1, 0x80+byte); 								  	// write one byte to the slave
		I2C_stop(I2C1); 											  	// stop the transmission

		I2C_start(I2C1, SLAVE_ADDRESS<<1, I2C_Direction_Receiver);		// start a transmission in Master receiver mode
		received_data[byte] = I2C_read_nack(I2C1); // read one byte and do not request another byte
	}

	mpos = 0; npos = 0;

	for (uint8_t byte = 0; byte < MAXPIXREAD; byte++)					// prochazi nacteny buffer
	{
		position = byte << 1;

		// temperature je 12 bitova hodnota s rozlisenim na 0,25st. C
		temperature = received_data[position+1] << 8 | received_data[position];
		//irarray[byte] = temperature >> 2;		// prevod na 8 bitove cislo

		// musime ukladat na pozice 0,2,4,...14, 32,34,36,...46, 64,66...
		//irarray[byte] = temperature;			// prevod na 16 bitove cislo

		if (mode == MODE256)
		{
			// vzorec na umisteni prvku pres jednu pozici v sloupcich i radcich
			irarray[32*mpos+2*npos] = temperature;
		}
		else if (mode == MODE1024)
		{
			// vzorec na umisteni prvku pres ctyri pozice v sloupcich i radcich
			irarray1024[116*mpos+4*npos] = temperature;
		}
		npos++;		// obnov pozici pro sloupce
		if (npos > 7)	// pokud jsme na poslednim sloupci, nastav zase sloupec 0
		{
			mpos++;		// pro novy sloupec 0 nastav dalsi radek
			npos = 0;
		}
	}
}


void Amg8833_Interpolate256(void)
{
	// projdi celou mapu a dopocitej chybejici policka
	uint8_t mpos;		// ukazatel na jednotlive radky
	uint8_t npos;		// ukazatel v ramci jednoho radku

	// zacneme s dopocitanim hodnot v radcich
	for (mpos = 0; mpos < 8; mpos++)
	{
		for (npos = 0; npos < 8; npos++)
		{
			if (npos == 7)
			{
				irarray[32*mpos+2*npos+1] = irarray[32*mpos+2*npos];
			}
			else
			{
				irarray[32*mpos+2*npos+1] = (irarray[32*mpos+2*npos] + irarray[32*mpos+2*npos+2])/2;
			}
		}
	}

	// pokracujeme s dopocitanim hodnot ve sloupcich
	for (mpos = 1; mpos < 16; mpos += 2)
	{
		for (npos = 0; npos < 8; npos++)
		{
			if (mpos == 15)
			{
				irarray[16*mpos+2*npos] = irarray[16*(mpos-1)+2*npos];
			}
			else
			{
				irarray[16*mpos+2*npos] = (irarray[16*(mpos-1)+2*npos] + irarray[16*(mpos+1)+2*npos])/2;
			}
		}
	}

	// nyni prvky v krizovych vazbach
	for (mpos = 1; mpos < 16; mpos += 2)
	{
		for (npos = 0; npos < 8; npos++)
		{
			if (mpos == 15)
			{
				if (npos == 7)
				{
					// uplne posledni bod je kopie sousedniho bodu
					irarray[16*mpos+2*npos+1] = irarray[16*(mpos-1)+2*npos];
				}
				else
				{
					// posledni radek se sestavuje jen ze dvou prvku
					irarray[16*mpos+2*npos+1] = (irarray[16*(mpos-1)+2*npos] + irarray[16*(mpos-1)+2*(npos+1)])/2;
				}
			}
			else if (npos == 7)
			{
				// posledni sloupec se sestavuje jen ze dvou prvku
				irarray[16*mpos+2*npos+1] = (irarray[16*(mpos-1)+2*npos] + irarray[16*(mpos+1)+2*npos])/2;
			}
			else
			{
				irarray[16*mpos+2*npos+1] = (irarray[16*(mpos-1)+2*npos] + irarray[16*(mpos-1)+2*(npos+1)] + irarray[16*(mpos+1)+2*npos] + irarray[16*(mpos+1)+2*(npos+1)])/4;
			}
		}
	}
}

void Amg8833_Interpolate1024(void)
{
	// projdi celou mapu a dopocitej chybejici policka
	uint8_t mpos;		// ukazatel na jednotlive radky
	uint8_t npos;		// ukazatel v ramci jednoho radku
	uint16_t index;		// ukazatel na aktualni policko

	// budeme skakat po nactenych hodnotach a dopocitame vzdy vsechny hodnoty uvnitr pole 4x4
	// sloupce 0-6, radky 0-6
	for (mpos = 0; mpos < 7; mpos++)
	{
		for (npos = 0; npos < 7; npos++)
		{
				index = 116*mpos+4*npos;		// vychozi bod prochazeni, nacteny bod z HW			// VAL0
				// index+4, index+116, index+120 jsou take nactene body z HW
				// kazdy vnitrni bod je ovlivnen dalsimi HW body a cisla jsou predpocitane vahy vlivu

				switch (get_interpolation_mode())
				{
					default:
					case INTMODE0:
						// postupny prumerovaci postup, stred, stred, stred
						// nejprve prostredni bod [1]
						irarray1024[index+60] = (25*irarray1024[index] + 25*(irarray1024[index+4]) + 25*(irarray1024[index+116]) + 25*(irarray1024[index+120]))/100;// VAL10
						// pak nove utvorene stredy urovne [2]
						irarray1024[index+2] = (33*irarray1024[index] + 34*(irarray1024[index+4]) + 33*(irarray1024[index+60]))/100;	// VAL2
						irarray1024[index+58] = (33*irarray1024[index] + 34*(irarray1024[index+116]) + 33*(irarray1024[index+60]))/100;// VAL8
						irarray1024[index+89] = (33*irarray1024[index+116] + 34*(irarray1024[index+60]) + 33*(irarray1024[index+120]))/100;// VAL14
						// a dale prvky mezi na urovni [3]
						irarray1024[index+30] = (25*irarray1024[index] + 25*(irarray1024[index+2]) + 25*(irarray1024[index+58]) + 25*(irarray1024[index+60]))/100;// VAL5
						irarray1024[index+32] = (33*irarray1024[index+2] + 33*(irarray1024[index+4]) + 33*(irarray1024[index+60]))/100;// VAL7
						irarray1024[index+88] = (25*irarray1024[index+58] + 25*(irarray1024[index+60]) + 25*(irarray1024[index+116]) + 26*(irarray1024[index+120]))/100;// VAL13
						irarray1024[index+90] = (33*irarray1024[index+60] + 33*(irarray1024[index+89]) + 33*(irarray1024[index+120]))/100;// VAL15
						// body na urovni [4]
						irarray1024[index+1] = (33*irarray1024[index] + 33*(irarray1024[index+2]) + 33*(irarray1024[index+30]))/100;	// VAL1
						irarray1024[index+3] = (33*irarray1024[index+2] + 33*(irarray1024[index+4]) + 33*(irarray1024[index+32]))/100;	// VAL3
						irarray1024[index+29] = (33*irarray1024[index] + 33*(irarray1024[index+30]) + 33*(irarray1024[index+58]))/100;	// VAL4
						irarray1024[index+31] = (25*irarray1024[index+2] + 25*(irarray1024[index+30]) + 25*(irarray1024[index+32]) + 25*(irarray1024[index+60]))/100;// VAL6
						irarray1024[index+59] = (25*irarray1024[index+30] + 25*(irarray1024[index+58]) + 25*(irarray1024[index+60]) + 25*(irarray1024[index+88]))/100;// VAL9
						irarray1024[index+61] = (33*irarray1024[index+60] + 33*(irarray1024[index+32]) + 33*(irarray1024[index+90]))/100;// VAL11
						irarray1024[index+87] = (33*irarray1024[index+58] + 33*(irarray1024[index+88]) + 33*(irarray1024[index+116]))/100;	// VAL12
					break;

					case INTMODE1:
						// artefakty 4x4 body
						irarray1024[index+1] = (38*irarray1024[index] + 31*(irarray1024[index+4]) + 23*(irarray1024[index+116]) + 8*(irarray1024[index+120]))/100;	// VAL1
						irarray1024[index+2] = (35*irarray1024[index] + 35*(irarray1024[index+4]) + 15*(irarray1024[index+116]) + 15*(irarray1024[index+120]))/100;	// VAL2
						irarray1024[index+3] = (31*irarray1024[index] + 38*(irarray1024[index+4]) + 8*(irarray1024[index+116]) + 23*(irarray1024[index+120]))/100;	// VAL3
						irarray1024[index+29] = (38*irarray1024[index] + 23*(irarray1024[index+4]) + 31*(irarray1024[index+116]) + 8*(irarray1024[index+120]))/100;	// VAL4
						irarray1024[index+30] = (36*irarray1024[index] + 26*(irarray1024[index+4]) + 26*(irarray1024[index+116]) + 12*(irarray1024[index+120]))/100;// VAL5
						irarray1024[index+31] = (31*irarray1024[index] + 31*(irarray1024[index+4]) + 19*(irarray1024[index+116]) + 19*(irarray1024[index+120]))/100;// VAL6
						irarray1024[index+32] = (26*irarray1024[index] + 36*(irarray1024[index+4]) + 12*(irarray1024[index+116]) + 26*(irarray1024[index+120]))/100;// VAL7
						irarray1024[index+58] = (35*irarray1024[index] + 15*(irarray1024[index+4]) + 35*(irarray1024[index+116]) + 15*(irarray1024[index+120]))/100;// VAL8
						irarray1024[index+59] = (31*irarray1024[index] + 19*(irarray1024[index+4]) + 31*(irarray1024[index+116]) + 19*(irarray1024[index+120]))/100;// VAL9
						irarray1024[index+60] = (25*irarray1024[index] + 25*(irarray1024[index+4]) + 25*(irarray1024[index+116]) + 25*(irarray1024[index+120]))/100;// VAL10
						irarray1024[index+61] = (19*irarray1024[index] + 31*(irarray1024[index+4]) + 19*(irarray1024[index+116]) + 31*(irarray1024[index+120]))/100;// VAL11
						irarray1024[index+87] = (31*irarray1024[index] + 8*(irarray1024[index+4]) + 38*(irarray1024[index+116]) + 23*(irarray1024[index+120]))/100;	// VAL12
						irarray1024[index+88] = (26*irarray1024[index] + 12*(irarray1024[index+4]) + 36*(irarray1024[index+116]) + 26*(irarray1024[index+120]))/100;// VAL13
						irarray1024[index+89] = (19*irarray1024[index] + 19*(irarray1024[index+4]) + 31*(irarray1024[index+116]) + 31*(irarray1024[index+120]))/100;// VAL14
						irarray1024[index+90] = (12*irarray1024[index] + 26*(irarray1024[index+4]) + 26*(irarray1024[index+116]) + 36*(irarray1024[index+120]))/100;// VAL15
					break;

					case INTMODE2:
						// vice rozpite, vyhlazene artefakty
						irarray1024[index+1] = (48*irarray1024[index] + 33*(irarray1024[index+4]) + 17*(irarray1024[index+116]) + 2*(irarray1024[index+120]))/100;	// VAL1
						irarray1024[index+2] = (42*irarray1024[index] + 42*(irarray1024[index+4]) + 8*(irarray1024[index+116]) + 8*(irarray1024[index+120]))/100;	// VAL2
						irarray1024[index+3] = (33*irarray1024[index] + 48*(irarray1024[index+4]) + 2*(irarray1024[index+116]) + 17*(irarray1024[index+120]))/100;	// VAL3
						irarray1024[index+29] = (48*irarray1024[index] + 17*(irarray1024[index+4]) + 33*(irarray1024[index+116]) + 2*(irarray1024[index+120]))/100;	// VAL4
						irarray1024[index+30] = (47*irarray1024[index] + 24*(irarray1024[index+4]) + 24*(irarray1024[index+116]) + 5*(irarray1024[index+120]))/100;// VAL5
						irarray1024[index+31] = (36*irarray1024[index] + 36*(irarray1024[index+4]) + 14*(irarray1024[index+116]) + 14*(irarray1024[index+120]))/100;// VAL6
						irarray1024[index+32] = (24*irarray1024[index] + 47*(irarray1024[index+4]) + 5*(irarray1024[index+116]) + 24*(irarray1024[index+120]))/100;// VAL7
						irarray1024[index+58] = (42*irarray1024[index] + 8*(irarray1024[index+4]) + 42*(irarray1024[index+116]) + 8*(irarray1024[index+120]))/100;// VAL8
						irarray1024[index+59] = (36*irarray1024[index] + 14*(irarray1024[index+4]) + 36*(irarray1024[index+116]) + 14*(irarray1024[index+120]))/100;// VAL9
						irarray1024[index+60] = (25*irarray1024[index] + 25*(irarray1024[index+4]) + 25*(irarray1024[index+116]) + 25*(irarray1024[index+120]))/100;// VAL10
						irarray1024[index+61] = (14*irarray1024[index] + 36*(irarray1024[index+4]) + 14*(irarray1024[index+116]) + 36*(irarray1024[index+120]))/100;// VAL11
						irarray1024[index+87] = (33*irarray1024[index] + 2*(irarray1024[index+4]) + 48*(irarray1024[index+116]) + 17*(irarray1024[index+120]))/100;	// VAL12
						irarray1024[index+88] = (24*irarray1024[index] + 5*(irarray1024[index+4]) + 47*(irarray1024[index+116]) + 24*(irarray1024[index+120]))/100;// VAL13
						irarray1024[index+89] = (14*irarray1024[index] + 14*(irarray1024[index+4]) + 36*(irarray1024[index+116]) + 36*(irarray1024[index+120]))/100;// VAL14
						irarray1024[index+90] = (5*irarray1024[index] + 24*(irarray1024[index+4]) + 24*(irarray1024[index+116]) + 47*(irarray1024[index+120]))/100;// VAL15
					break;
				}
		}
	}
	// posledni radek
	mpos = 7;
	for (npos = 0; npos < 8; npos++)
	{
			index = 116*mpos+4*npos;		// vychozi bod prochazeni, nacteny bod z HW			// VAL0
			// index+4, index+116, index+120 jsou take nactene body z HW
			// kazdy vnitrni bod je ovlivnen dalsimi HW body a cisla jsou predpocitane vahy vlivu

			irarray1024[index+1] = (75*irarray1024[index] + 25*(irarray1024[index+4]))/100;	// VAL1
			irarray1024[index+2] = (50*irarray1024[index] + 50*(irarray1024[index+4]))/100;	// VAL2
			irarray1024[index+3] = (25*irarray1024[index] + 75*(irarray1024[index+4]))/100;	// VAL3
	}

	// posledni sloupec
	npos = 7;
	for (mpos = 0; mpos < 8; mpos++)
	{
			index = 116*mpos+4*npos;		// vychozi bod prochazeni, nacteny bod z HW			// VAL0
			// index+4, index+116, index+120 jsou take nactene body z HW
			// kazdy vnitrni bod je ovlivnen dalsimi HW body a cisla jsou predpocitane vahy vlivu

			irarray1024[index+29] = (75*irarray1024[index] + 25*(irarray1024[index+116]))/100;	// VAL1
			irarray1024[index+58] = (50*irarray1024[index] + 50*(irarray1024[index+116]))/100;	// VAL2
			irarray1024[index+87] = (25*irarray1024[index] + 75*(irarray1024[index+116]))/100;	// VAL3
	}
}

void set_interpolation_mode(uint8_t mode)
{
	interpolationmode = mode % INTMODEMAX;

	uint8_t text[2];	// jeden znak a nula na konci

	// vymaz stare cislo
	Draw_Rectangle(INTERPOSX, INTERPOSY-8, 8, 8, LCD_RED);

	text[0] = 0x30+interpolationmode;
	text[1] = 0;
	Display_String(INTERPOSX, INTERPOSY, &text[0], LCD_WHITE);
}

uint8_t get_interpolation_mode(void)
{
	return interpolationmode;
}

void Amg8833_Draw_screen256(void)
{
	uint8_t index = 0;

	for (uint16_t posy = 0; posy < IRDIMY; posy++)	// ukazatel do pole ctvercu na obrazovce
	{// kreslime postupne linky v ose y
		for (uint16_t posx = 0; posx < IRDIMX; posx++) // ukazatel na pole ctvercu na obrazovce
		{// kreslime linku v sirce obdelniku
			//Draw_Rectangle(posy*RECTSIZEX, DISPSIZEY-(posx+1)*RECTSIZEY, RECTSIZEX, RECTSIZEY, Calculate_color(irarray[index++]));
			Draw_Rectangle(posx*RECTSIZEX, DISPSIZEY-(posy+1)*RECTSIZEY, RECTSIZEX, RECTSIZEY, Calculate_color(irarray[index++]));
		}
	}

	// vykresli skalu
	//Amg8833_Draw_scale();
}

void Amg8833_Draw_screen1024(void)
{
	uint16_t index = 0;

	for (uint16_t posy = 0; posy < IRDIMY1024; posy++)	// ukazatel do pole ctvercu na obrazovce
	{// kreslime postupne linky v ose y
		for (uint16_t posx = 0; posx < IRDIMX1024; posx++) // ukazatel na pole ctvercu na obrazovce
		{// kreslime linku v sirce obdelniku
		// ruzne orientace kresleni na displej:
			//Draw_Rectangle(posy*RECTSIZEX, DISPSIZEY-(posx+1)*RECTSIZEY, RECTSIZEX, RECTSIZEY, Calculate_color(irarray[index++]));
			//Draw_Rectangle(posx*RECTSIZEX1024, DISPSIZEY-(posy+1)*RECTSIZEY1024, RECTSIZEX1024, RECTSIZEY1024, Calculate_color(irarray1024[index++]));
			Draw_Rectangle(DISPSIZEX-(posx+1)*RECTSIZEX1024, DISPSIZEY-(posy+1)*RECTSIZEY1024, RECTSIZEX1024, RECTSIZEY1024, Calculate_color(irarray1024[index++]));
		}
	}
}


uint16_t Calculate_color(int16_t value)
{
	// vstupem je hodnota -1024 az +1023, nutno predelat na pomer 0-100 v aktivnim pasmu - danem maxtemp a mintemp

	uint32_t hodnota;
	int16_t range;

	range = maxtemp - mintemp;
	hodnota = (value - mintemp);	// hodnota je nyni vzdy kladna a je 0 az range

	if (range > MINDEGREES)
	{
		// musime zvetsit na range 100
		// snizit rozsah na 0-100 vydelenim pomerem range/MAXDEGREES neboli nasobit MAXDEGREES/range
		hodnota *= MAXDEGREES;
		hodnota /= range;
		hodnota--;					// korekce, hodnota muze byt jen 0-99
	}
	return Get_FLIR_color((uint8_t)hodnota);
}

void Amg8833_Scale_init(void)
{
	// vykresli stupnici
	for (uint16_t posx = 0; posx < MAXDEGREES; posx++) // ukazatel carku skaly
	{// kreslime linku v sirce skaly, tloustka linky 2 body
		Draw_Rectangle(DISPSIZEX-SCALEOFFSETX-posx*SCALESIZEX, SCALEOFFSETY, SCALESIZEX, SCALESIZEY, Get_FLIR_color(posx));
	}
}

void Amg8833_Draw_scale_values(void)
{
	uint8_t text[6];	// -SDJoC
	int16_t temperature;
	uint8_t digit;

	// popis nejvyssi a nejnizsi teplotu
	// vymaz pole pro text
	Draw_Rectangle(SCALETEMPSIZEX-1, 0, SCALETEMPSIZEX+1, SCALETEMPSIZEY, LCD_RED);
	Draw_Rectangle(SCALETEMPMINPOSX-1, 0, SCALETEMPSIZEX+1, SCALETEMPSIZEY, LCD_RED);

	// vypis texty
	Set_Font(&FONT_8);							// nastavi font

	// priprav text maxima
	temperature = maxtemp;
	temperature /= 4;

	// znamenko
	if (temperature < 0)
	{
		text[0] = '-';
	}
	else
	{
		text[0] = '+';
	}

	// preved na cislo ASCII
	digit = temperature / 100;
	text[1] = digit + '0';
	temperature -= digit * 100;
	digit = temperature / 10;
	text[2] = digit + '0';
	temperature -= digit * 10;
	text[3] = temperature + '0';
	text[4] = 0x7F;//'°';
	text[5] = 'C';
	text[6] = 0;
	Display_String(SCALETEMPMAXPOSX, SCALETEMPMAXPOSY, &text[0], LCD_WHITE);

	// priprav text minima
	temperature = mintemp;
	temperature /= 4;

	// znamenko
	if (temperature < 0)
	{
		text[0] = '-';
	}
	else
	{
		text[0] = '+';
	}

	// preved na cislo ASCII
	digit = temperature / 100;
	text[1] = digit + '0';
	temperature -= digit * 100;
	digit = temperature / 10;
	text[2] = digit + '0';
	temperature -= digit * 10;
	text[3] = temperature + '0';
	text[4] = 0x7F;//'°';
	text[5] = 'C';
	text[6] = 0;
	Display_String(SCALETEMPMINPOSX, SCALETEMPSIZEY, &text[0], LCD_WHITE);
}

void Amg8833_CalculateCOM(uint8_t mode)
{
	uint8_t mpos;		// ukazatel na jednotlive radky
	uint8_t npos;		// ukazatel v ramci jednoho radku
	int32_t sum;		// celkova vaha obrazu
	int32_t sumx;		// cast vahy v x
	int32_t sumy;		// cast vahy v y
	uint32_t comx;
	uint32_t comy;
	uint16_t temp;
	uint16_t length;
	uint8_t limit;

	sum = 0;
	sumx = 0;
	sumy = 0;

	if (mode == MODE256)
	{
		length = (IRDIMX*IRDIMY)-1;
		limit = IRDIMX;
	}
	else if (mode == MODE1024)
	{
		length = IRDIMX1024*IRDIMX1024-1;
		limit = IRDIMX1024;
	}

	// pokud je neco v obrazu, zpracuj to
	if ((maxtemp - mintemp) > COMTHRESHOLD)
	{
		// prahovani, budeme uvazovat jen plochy s maximalni teplotou, jine budou vynulovany
		for (uint16_t index = 0; index <= length; index++)
		{
			if (irarray1024[index] < (maxtemp-COMMAXLIMIT))
			{
				irarray1024[index] = 0;
			}
		}

		// projedeme puvodni hodnoty v celem poli
		for (mpos = 0; mpos < (limit-1); mpos++)  // radky
		{
			for (npos = 0; npos < (limit-1); npos++)  // sloupce
			{
				//temp = irarray[32*mpos+2*npos]-mintemp;
				temp = irarray1024[IRDIMX1024*mpos+npos];		// zde uz neni mintemp, protoze mame naprahovano
				sumx += (temp)*(npos+1);
				sumy += (temp)*(mpos+1);
				sum += (temp);
			}
		}
		comx = (sumx) / sum;		// 2*sumx
		comy = (sumy) / sum;		// 2*sumy
		Draw_Rectangle(comx*RECTSIZEX+5, DISPSIZEY-(comy+1)*RECTSIZEY+5, COMSIZEX, COMSIZEY, LCD_RED);
	}
}

void laser_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Laser led pin, normally PB8, active in high
	GPIO_InitStructure.GPIO_Pin = LASER_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LASER_PORT, &GPIO_InitStructure);

	uint8_t text[2];	// jeden znak a nula na konci
	Set_Font(&FONT_8);	// nastavi font

	text[0] = 'L';
	text[1] = 0;
	Display_String(LASERPOSX-14, LASERPOSY+16, &text[0], LCD_WHITE);
	text[0] = 'A';
	text[1] = 0;
	Display_String(LASERPOSX-5, LASERPOSY+16, &text[0], LCD_WHITE);
	text[0] = 'S';
	text[1] = 0;
	Display_String(LASERPOSX+4, LASERPOSY+16, &text[0], LCD_WHITE);
	text[0] = 'E';
	text[1] = 0;
	Display_String(LASERPOSX+13, LASERPOSY+16, &text[0], LCD_WHITE);
	text[0] = 'R';
	text[1] = 0;
	Display_String(LASERPOSX+22, LASERPOSY+16, &text[0], LCD_WHITE);
}

void laser_control(uint8_t mode)
{
	if (mode == LASERACTIVE)
	{
		GPIO_SetBits(LASER_PORT, LASER_PIN);		// activate Gate of switching FET
	}
	else
	{
		GPIO_ResetBits(LASER_PORT, LASER_PIN);		// deactivate Gate of switching FET
	}
}

void laser_icon_Draw()
{
	uint8_t text[3];	// dva znaky a nula na konci

	// vymaz starou ikonu
	//Draw_Rectangle(LASERPOSX, LASERPOSY-16, 16, 16, LCD_BLACK);

	if (get_laser_active() == LASERACTIVE)
	{
		// nakresli ikonu
		text[0] = 0x80;	// 1/4 ikony
		text[1] = 0x81; // 2/4 ikony
		text[2] = 0;
		Display_String(LASERPOSX, LASERPOSY, &text[0], LCD_RED);
		text[0] = 0x82; // 3/4 ikony
		text[1] = 0x83; // 4/4 ikony
		text[2] = 0;
		Display_String(LASERPOSX+8, LASERPOSY, &text[0], LCD_RED);
	}
}

void laser_icon_Clear()
{
	uint8_t text[3];	// dva znaky a nula na konci

	// vymaz starou ikonu
	Draw_Rectangle(LASERPOSX, LASERPOSY-16, 16, 16, LCD_BLACK);
}

void Amg8833_Interpolation_init(void)
{
	uint8_t text[2];	// jeden znak a nula na konci

	// vypis legendu
	Set_Font(&FONT_8);							// nastavi font
	text[0] = 'I';
	text[1] = 0;
	Display_String(INTERTEXTPOSX-1, INTERTEXTPOSY, &text[0], LCD_WHITE);
	text[0] = 'N';
	text[1] = 0;
	Display_String(INTERTEXTPOSX+8, INTERTEXTPOSY, &text[0], LCD_WHITE);
	text[0] = 'T';
	text[1] = 0;
	Display_String(INTERTEXTPOSX+17, INTERTEXTPOSY, &text[0], LCD_WHITE);

	// inicializuj a vypis cislo modu
	set_interpolation_mode(INTMODE0);
}

