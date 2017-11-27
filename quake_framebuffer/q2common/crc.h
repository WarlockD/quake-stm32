/* crc.h */
#ifndef _QUAKE2_CRC_H_
#define _QUAKE2_CRC_H_
void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, unsigned char data);
unsigned short CRC_Value(unsigned short crcvalue);
unsigned short CRC_Block (unsigned char *start, int count);

#endif
