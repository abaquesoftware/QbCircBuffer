#include "qbQbCircBuffer.h"

#include <stdio.h>
#include <string.h>

void readQbCircBuffer( qbQbCircBuffer_t *qbQbCircBuffer )
{
	int iRc;
        char*output1, *output2;
        long output1_len, output2_len;
        qbQbCircBuffer_read_pointer_t read_pt;
        char output[4096];

        iRc=qbQbCircBuffer_readData(qbQbCircBuffer, &output1, &output1_len, &output2, &output2_len, &read_pt);
        printf( "qbQbCircBuffer_readData - Result = %s - output1=%p, output1_len=%ld, output2=%p, output2_len=%ld, read_pt=%p\n" , CIRCBUFFER_RC_STRING[iRc] , output1, output1_len, output2, output2_len, read_pt);
        *output='\0';
        if(output1_len > 0 ) strncat(output,output1,output1_len);
        if(output2_len > 0 ) strncat(output,output2,output2_len);
        printf( "Read Data :'%s' (%ld)\n" , output , strlen(output) );
	iRc = qbQbCircBuffer_releaseReadData(qbQbCircBuffer, read_pt);
	printf( "qbQbCircBuffer_releaseReadData - Result = %s\n" , CIRCBUFFER_RC_STRING[iRc] );
}


int main( int nb_arg, char **arg_list )
{
	printf( "================ test QbCircBuffer ==========\n" );

	qbQbCircBuffer_t *qbQbCircBuffer;
	int iRc;

	iRc = qbQbCircBuffer_init( 100 , "_LOG_" , &qbQbCircBuffer );
	printf( "qbQbCircBuffer_init - Result = %s\n" , CIRCBUFFER_RC_STRING[iRc] );
	qbQbCircBuffer_hexDump( qbQbCircBuffer, "After Init");
	
	int i;
	for( i = 0 ; i < 10 ; i++)
	{
	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",30);
	printf( "qbQbCircBuffer_storeData - add A - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",30);
	printf( "qbQbCircBuffer_storeData - add B - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",30);
	printf( "qbQbCircBuffer_storeData - add C - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	//qbQbCircBuffer_hexDump( qbQbCircBuffer, "After ABC");

	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",30);
	printf( "qbQbCircBuffer_storeData - add E - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	//qbQbCircBuffer_hexDump( qbQbCircBuffer, "After E");

	readQbCircBuffer( qbQbCircBuffer );

	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",30);
	printf( "qbQbCircBuffer_storeData - add F - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	iRc = qbQbCircBuffer_storeData(qbQbCircBuffer, "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",30);
	printf( "qbQbCircBuffer_storeData - add G - Result = %s - usage : %.2f\n" , CIRCBUFFER_RC_STRING[iRc] , (float)qbQbCircBuffer_getUsedSpaceSize(qbQbCircBuffer)*100./(float)qbQbCircBuffer->buffer_size );
	//qbQbCircBuffer_hexDump( qbQbCircBuffer, "After FG");

	readQbCircBuffer( qbQbCircBuffer );
	}

	return 0;
}
