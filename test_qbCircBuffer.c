#include "qbCircBuffer.h"

#include <stdio.h>
#include <string.h>

void readQbCircBuffer( qbCircBuffer_t *qbCircBuffer )
{
	int iRc;
        char*output1, *output2;
        long output1_len, output2_len;
        qbCircBuffer_read_pointer_t read_pt;
        char output[4096];

        iRc=qbCircBuffer_readData(qbCircBuffer, &output1, &output1_len, &output2, &output2_len, &read_pt);
        printf( "qbCircBuffer_readData - Result = %s - output1=%p, output1_len=%ld, output2=%p, output2_len=%ld, read_pt=%p\n" , QBCIRCBUFFER_RC_STRING[iRc] , output1, output1_len, output2, output2_len, read_pt);
        *output='\0';
        if(output1_len > 0 ) strncat(output,output1,output1_len);
        if(output2_len > 0 ) strncat(output,output2,output2_len);
        printf( "Read Data :'%s' (%ld)\n" , output , strlen(output) );
	iRc = qbCircBuffer_releaseReadData(qbCircBuffer, read_pt);
	printf( "qbCircBuffer_releaseReadData - Result = %s\n" , QBCIRCBUFFER_RC_STRING[iRc] );
}


int main( int nb_arg, char **arg_list )
{
	printf( "================ test QbCircBuffer ==========\n" );

	qbCircBuffer_t *qbCircBuffer;
	int iRc;

	iRc = qbCircBuffer_init( 100 , "_LOG_" , &qbCircBuffer );
	printf( "qbCircBuffer_init - Result = %s\n" , QBCIRCBUFFER_RC_STRING[iRc] );
	qbCircBuffer_hexDump( qbCircBuffer, "After Init");
	
	int i;
	for( i = 0 ; i < 10 ; i++)
	{
	iRc = qbCircBuffer_storeData(qbCircBuffer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",30);
	printf( "qbCircBuffer_storeData - add A - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	iRc = qbCircBuffer_storeData(qbCircBuffer, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",30);
	printf( "qbCircBuffer_storeData - add B - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	iRc = qbCircBuffer_storeData(qbCircBuffer, "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",30);
	printf( "qbCircBuffer_storeData - add C - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	//qbCircBuffer_hexDump( qbCircBuffer, "After ABC");

	iRc = qbCircBuffer_storeData(qbCircBuffer, "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",30);
	printf( "qbCircBuffer_storeData - add E - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	//qbCircBuffer_hexDump( qbCircBuffer, "After E");

	readQbCircBuffer( qbCircBuffer );

	iRc = qbCircBuffer_storeData(qbCircBuffer, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",30);
	printf( "qbCircBuffer_storeData - add F - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	iRc = qbCircBuffer_storeData(qbCircBuffer, "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",30);
	printf( "qbCircBuffer_storeData - add G - Result = %s - usage : %.2f\n" , QBCIRCBUFFER_RC_STRING[iRc] , (float)qbCircBuffer_getUsedSpaceSize(qbCircBuffer)*100./(float)qbCircBuffer->buffer_size );
	//qbCircBuffer_hexDump( qbCircBuffer, "After FG");

	readQbCircBuffer( qbCircBuffer );
	}

	return 0;
}
