#ifndef _CRC_32_
#define _CRC_32_

//////////////////////////////////////////////////////////////////////////
extern "C"
{
	unsigned int CRC32(unsigned int CRC, void const *pvBuf, unsigned int uLen);
};

#endif  // _CRC_32