/* File - i2c_master.h
   Last Modified by - Sarvesh Nagarajan
   Date modified - 10/14/2008
   Reference : Embedded Microcomputer Systems by Dr. Valvano
*/

#ifndef _I2C_MASTER_INTR_H
#define _I2C_MASTER_INTR_H 1

#define I2C_BUFFER_SIZE 32

#define ERR_OK 0
#define ERR_BUSOFF 1
#define ERR_SPEED 2
#define ERR_DISABLED 3
#define ERR_BUSY 4
#define ERR_TXFULL 5
#define ERR_ARBITR 6

#define I2C1_BUSY  1                   /* Start condition detected on the I2C */
#define I2C1_IDLE  0                   /* Stop condition detected on the I2C */


/*
** ===================================================================
**     Method      :  I2C1_Enable (bean InternalI2C)
**
**     Description :
**         Enables I2C bean. Events may be generated
**         ("DisableEvent"/"EnableEvent").
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
unsigned char I2C1_Enable(void);

/*
** ===================================================================
**     Method      :  I2C1_Disable (bean InternalI2C)
**
**     Description :
**         Disables I2C bean. No events will be generated.
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
unsigned char I2C1_Disable(void);

/*
** ===================================================================
**     Method      :  I2C1_EnableEvent (bean InternalI2C)
**
**     Description :
**         Enables the events.
**         This method is enabled only if any event is generated.
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
/*
byte I2C1_EnableEvent(void)

**  This method is implemented as a macro. See I2C1.h file.  **
*/

/*
** ===================================================================
**     Method      :  I2C1_DisableEvent (bean InternalI2C)
**
**     Description :
**         Disables the events.
**         This method is enabled only if any event is generated.
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
/*
byte I2C1_DisableEvent(void)

**  This method is implemented as a macro. See I2C1.h file.  **
*/

/*
** ===================================================================
**     Method      :  I2C1_SendChar (bean InternalI2C)
**
**     Description :
**         When working as a MASTER, this method writes one (7-bit
**         addressing) or two (10-bit addressing) slave address
**         bytes inclusive of R/W bit = 0 to the I2C bus and then
**         writes one character (byte) to the bus. The slave address
**         must be specified before, by the "SelectSlave" or
**         "SelectSlave10" method or in the bean initialization
**         section, "Target slave address init" property. If
**         interrupt service is enabled and the method returns
**         ERR_OK, it doesn't mean that transmission was successful.
**         The state of transmission is obtainable from
**         (OnTransmitData, OnError or OnArbitLost) events. 
**         When working as a SLAVE, this method writes a character
**         to the internal output slave buffer and, after the master
**         starts the communication, to the I2C bus. If no character
**         is ready for a transmission (internal output slave buffer
**         is empty), the Empty character will be sent (see "Empty
**         character" property).
**     Parameters  :
**         NAME            - DESCRIPTION
**         Chr             - Character to send.
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED -  Device is disabled
**                           ERR_BUSY - The slave device is busy, it
**                           does not respond by an acknowledge (only
**                           in master mode and when interrupt
**                           service is disabled)
**                           ERR_BUSOFF - Clock timeout elapsed or
**                           device cannot transmit data
**                           ERR_TXFULL - Transmitter is full (slave
**                           mode only)
**                           ERR_ARBITR - Arbitration lost (only when
**                           interrupt service is disabled and in
**                           master mode)
** ===================================================================
*/
unsigned char I2C1_SendChar(unsigned char Chr);

/*
** ===================================================================
**     Method      :  I2C1_RecvChar (bean InternalI2C)
**
**     Description :
**         When working as a MASTER, this method writes one (7-bit
**         addressing) or two (10-bit addressing) slave address
**         bytes inclusive of R/W bit = 1 to the I2C bus, then reads
**         one character (byte) from the bus and then sends the stop
**         condition. The slave address must be specified before, by
**         the "SelectSlave" or "SelectSlave10" method or in bean
**         initialization section, property "Target slave address
**         init". If interrupt service is enabled and the method
**         returns ERR_OK, it doesn't mean that transmission was
**         finished successfully. The state of transmission must be
**         tested by means of events (OnReceiveData, OnError or
**         OnArbitLost). In case of successful transmission,
**         received data is ready after OnReceiveData event is
**         called. 
**         When working as a SLAVE, this method reads a character
**         from the input slave buffer.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Chr             - Received character.
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED -  Device is disabled
**                           ERR_BUSY - The slave device is busy, it
**                           does not respond by the acknowledge
**                           (only in master mode and when interrupt
**                           service is disabled)
**                           ERR_BUSOFF - Clock timeout elapsed or
**                           device cannot receive data
**                           ERR_RXEMPTY - No data in receiver (slave
**                           mode only)
**                           ERR_OVERRUN - Overrun error was detected
**                           from the last character or block
**                           received (slave mode only)
**                           ERR_ARBITR - Arbitration lost (only when
**                           interrupt service is disabled and in
**                           master mode)
**                           ERR_NOTAVAIL - Method is not available
**                           in current mode - see generated code
**                           comment
** ===================================================================
*/
#define I2C1_RecvChar(Chr) I2C1_RecvBlock((Chr), 1, (unsigned short *)&I2C1_SndRcvTemp)

/*
** ===================================================================
**     Method      :  I2C1_SendBlock (bean InternalI2C)
**
**     Description :
**         When working as a MASTER, this method writes one (7-bit
**         addressing) or two (10-bit addressing) slave address
**         bytes inclusive of R/W bit = 0 to the I2C bus and then
**         writes the block of characters to the bus. The slave
**         address must be specified before, by the "SelectSlave" or
**         "SlaveSelect10" method or in bean initialization section,
**         "Target slave address init" property. If interrupt
**         service is enabled and the method returns ERR_OK, it
**         doesn't mean that transmission was successful. The state
**         of transmission is detectable by means of events
**         (OnTransmitData, OnError or OnArbitLost). Data to be send
**         is not copied to an internal buffer and remains in the
**         original location. Therefore the content of the buffer
**         should not be changed until the transmission is complete.
**         Event OnTransmitData can be used to detect the end of the
**         transmission.
**         When working as a SLAVE, this method writes a block of
**         characters to the internal output slave buffer and then,
**         after the master starts the communication, to the I2C bus.
**         If no character is ready for a transmission (internal
**         output slave buffer is empty), the "Empty character" will
**         be sent (see "Empty character" property). In SLAVE mode
**         the data are copied to an internal buffer, if specified
**         by "Output buffer size" property.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - Pointer to the block of data to send.
**         Siz             - Size of the block.
**       * Snt             - Amount of data sent (moved to a buffer).
**                           In master mode, if interrupt support is
**                           enabled, the parameter always returns
**                           the same value as the parameter 'Siz' of
**                           this method.
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED -  Device is disabled
**                           ERR_BUSY - The slave device is busy, it
**                           does not respond by the acknowledge
**                           (only in master mode and when interrupt
**                           service is disabled)
**                           ERR_BUSOFF - Clock timeout elapsed or
**                           device cannot transmit data
**                           ERR_TXFULL - Transmitter is full. Some
**                           data has not been sent. (slave mode only)
**                           ERR_ARBITR - Arbitration lost (only when
**                           interrupt service is disabled and in
**                           master mode)
** ===================================================================
*/
unsigned char I2C1_SendBlock(void* Ptr,unsigned short Siz,unsigned short *Snt);

/*
** ===================================================================
**     Method      :  I2C1_RecvBlock (bean InternalI2C)
**
**     Description :
**         When working as a MASTER, this method writes one (7-bit
**         addressing) or two (10-bit addressing) slave address
**         bytes inclusive of R/W bit = 1 to the I2C bus, then reads
**         the block of characters from the bus and then sends the
**         stop condition. The slave address must be specified
**         before, by the "SelectSlave" or "SelectSlave10" method or
**         in bean initialization section, "Target slave address
**         init" property. If interrupt service is enabled and the
**         method returns ERR_OK, it doesn't mean that transmission
**         was finished successfully. The state of transmission must
**         be tested by means of events (OnReceiveData, OnError or
**         OnArbitLost). In case of successful transmission,
**         received data is ready after OnReceiveData event is
**         called. 
**         When working as a SLAVE, this method reads a block of
**         characters from the input slave buffer.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Ptr             - A pointer to the block space for
**                           received data.
**         Siz             - The size of the block.
**       * Rcv             - Amount of received data. In master mode,
**                           if interrupt support is enabled, the
**                           parameter always returns the same value
**                           as the parameter 'Siz' of this method.
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED -  Device is disabled
**                           ERR_BUSY - The slave device is busy, it
**                           does not respond by an acknowledge (only
**                           in master mode and when interrupt
**                           service is disabled)
**                           ERR_BUSOFF - Clock timeout elapsed or
**                           device cannot receive data
**                           ERR_RXEMPTY - The receive buffer didn't
**                           contain the requested number of data.
**                           Only available data (or no data) has
**                           been returned  (slave mode only).
**                           ERR_OVERRUN - Overrun error was detected
**                           from last character or block receiving
**                           (slave mode only)
**                           ERR_ARBITR - Arbitration lost (only when
**                           interrupt service is disabled and in
**                           master mode)
** ===================================================================
*/
unsigned char I2C1_RecvBlock(void* Ptr,unsigned short Siz,unsigned short *Rcv);

/*
** ===================================================================
**     Method      :  I2C1_GetCharsInTxBuf (bean InternalI2C)
**
**     Description :
**         Returns number of characters in the output buffer. In
**         SLAVE mode returns the number of characters in the
**         internal slave output buffer. In MASTER mode returns
**         number of characters to be sent from the user buffer
**         (passed by SendBlock method).
**         This method is not supported in polling mode.
**     Parameters  : None
**     Returns     :
**         ---             - Number of characters in the output
**                           buffer.
** ===================================================================
*/
unsigned short I2C1_GetCharsInTxBuf(void);

/*
** ===================================================================
**     Method      :  I2C1_GetCharsInRxBuf (bean InternalI2C)
**
**     Description :
**         Returns number of characters in the input buffer. In
**         SLAVE mode returns the number of characters in the
**         internal slave input buffer. In MASTER mode returns
**         number of characters to be received into a user buffer
**         (passed by RecvChar or RecvBlock method).
**         This method is not supported in polling mode.
**     Parameters  : None
**     Returns     :
**         ---             - Number of characters in the input
**                           buffer.
** ===================================================================
*/
unsigned short I2C1_GetCharsInRxBuf(void);

/*
** ===================================================================
**     Method      :  I2C1_SelectSlave (bean InternalI2C)
**
**     Description :
**         This method selects a new slave for communication by its
**         7-bit slave address value. Any send or receive method
**         directs to or from selected device, until a new slave
**         device is selected by this method. This method is not
**         available for the SLAVE mode.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Slv             - 7-bit slave address value.
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_BUSY - The device is busy, wait
**                           until the current operation is finished.
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED -  The device is disabled
** ===================================================================
*/
unsigned char I2C1_SelectSlave(unsigned char Slv);

/*
** ===================================================================
**     Method      :  I2C1_GetMode (bean InternalI2C)
**
**     Description :
**         This method returns the actual operating mode of this
**         bean.
**     Parameters  : None
**     Returns     :
**         ---             - Actual operating mode value
**                           TRUE - Master
**                           FALSE - Slave
** ===================================================================
*/
/*
bool I2C1_GetMode(void)

**  This method is implemented as a macro. See I2C1.h file.  **
*/

/*
** ===================================================================
**     Method      :  I2C1_Init (bean InternalI2C)
**
**     Description :
**         Initializes the associated peripheral(s) and the beans 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void I2C1_Init(void);

#endif /* _I2C_MASTER_INTR_H */