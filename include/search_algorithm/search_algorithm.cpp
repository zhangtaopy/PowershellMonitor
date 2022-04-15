#include "search_algorithm.h"

int BM_SearchPackSign( unsigned char* pBuf, int nBufLen, unsigned char* pbySignBuf, int nSignLen, unsigned char Mask )
{
    int BM_count = 0;
    int BadCharater[256];

    for(int i = nSignLen-1; i>=0; i--)
    {
        if(pbySignBuf[i] == Mask)
            break;
        BM_count++;
    }

    for(int i=0; i<256; i++)
    {
        BadCharater[i] = BM_count;
    }

    for(int i=nSignLen-BM_count; i<nSignLen-1; i++)
    {
        BadCharater[pbySignBuf[i]] = nSignLen-i-1;
    }

    int p = 0;
    int nSkip = 0;
    while(p<=nBufLen-nSignLen)
    {
        for(int j=nSignLen-1; j>=0; j--)
        {
            if(pbySignBuf[j] == Mask)
            {
                if(j==0)
                    return p;
                continue;
            }

            if(pbySignBuf[j] != pBuf[p+j])
            {
                if(nSignLen-j-1 < BM_count)
                {
                    nSkip = BadCharater[pBuf[p+j]]-(nSignLen-j-1);
                    if(nSkip <= 0)
                        nSkip = 1;
                    p += nSkip;
                }
                else
                {
                    p += 1;
                }

                break;
            }

            if(j==0)
            {
                return p;
            }
        }
    }

    return -1;
}