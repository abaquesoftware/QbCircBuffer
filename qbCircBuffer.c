#include "qbCircBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#define TLEVEL if
#define QBTRACE printf

char *QBCIRCBUFFER_RC_STRING[] = 
{ 
	"OK",                             // 0 
	"Unknwown error",                 // 1
	"Buffer overflow",                // 2
	"Memory allocation failed" ,      // 3
	"Mutex initialization failed" ,   // 4
	"Buffer closed"                   // 5
};

// **** private functions ****

char * _qbCircBuffer_move_pointer( qbCircBuffer_t *qbCircBuffer , char *start, long offset )
{
	char *ret=start + offset;
	while(ret < qbCircBuffer->buffer                         ) ret+=qbCircBuffer->buffer_size;
	while(ret > qbCircBuffer->buffer+qbCircBuffer->buffer_size ) ret-=qbCircBuffer->buffer_size;
	return ret;
}

long qbCircBuffer_getUsedSpaceSizeWithNoLock( qbCircBuffer_t *qbCircBuffer )
{
	long ret=(long)(qbCircBuffer->storage_pointer - qbCircBuffer->reading_pointer);
	if( qbCircBuffer->reading_pointer > qbCircBuffer->storage_pointer )
		ret+=qbCircBuffer->buffer_size;
	return ret;
}

// **** For debug

void qbCircBuffer_dump( qbCircBuffer_t *qbCircBuffer, const char *message )
{
	QBTRACE("%s : ------------------------- qbCircBuffer dump - %s -----------------------\n" , qbCircBuffer->log_prefix, message );
	QBTRACE("%s : Buffer size: %ld\n", qbCircBuffer->log_prefix, qbCircBuffer->buffer_size );
	QBTRACE("%s : pointer 'storage' : %ld\n" , qbCircBuffer->log_prefix, (qbCircBuffer->storage_pointer - qbCircBuffer->buffer) );
	QBTRACE("%s : pointer 'reading' : %ld\n" , qbCircBuffer->log_prefix, (qbCircBuffer->reading_pointer - qbCircBuffer->buffer) );
	QBTRACE("%s : ------------------------- qbCircBuffer dump - %s -----------------------\n" , qbCircBuffer->log_prefix, message );
}

void qbCircBuffer_hexDump( qbCircBuffer_t *qbCircBuffer, const char *message )
{
        QBTRACE("%s : ------------------------- qbCircBuffer hexDump - %s -----------------------\n" , qbCircBuffer->log_prefix, message );
        QBTRACE("%s : Buffer size: %ld\n", qbCircBuffer->log_prefix, qbCircBuffer->buffer_size );
        QBTRACE("%s : pointer 'storage' : %ld\n" , qbCircBuffer->log_prefix, (qbCircBuffer->storage_pointer - qbCircBuffer->buffer) );
        QBTRACE("%s : pointer 'reading' : %ld\n" , qbCircBuffer->log_prefix, (qbCircBuffer->reading_pointer - qbCircBuffer->buffer) );
        QBTRACE("%s : \n" , qbCircBuffer->log_prefix );
	char hexString[64];
	char asciiString[64];
	char tmp[8];
	int pos=0;
	hexString[0]='\0';
	asciiString[0]='\0';
	for(pos=0; pos < qbCircBuffer->buffer_size; pos++)
	{
		sprintf(tmp,"%02x.",qbCircBuffer->buffer[pos]);
		strcat(hexString,tmp);
		tmp[0]='?';
		if(qbCircBuffer->buffer[pos]>31 && qbCircBuffer->buffer[pos]<128)
			tmp[0]=qbCircBuffer->buffer[pos];
		tmp[1]='.';
		tmp[2]='\0';
		strcat(asciiString,tmp);
		if((pos+1)%16==0)
		{
			QBTRACE("%s : %-51s | %s\n", qbCircBuffer->log_prefix, hexString, asciiString );
			hexString[0]='\0';
		        asciiString[0]='\0';
		}
		else if((pos+1)%8==0)
		{
			strcat(hexString,"   ");
			strcat(asciiString,"   ");
		}
	}
	if(hexString[0]) QBTRACE("%s : %-51s | %s\n", qbCircBuffer->log_prefix, hexString, asciiString );

        QBTRACE("%s : ------------------------- qbCircBuffer hexDump - %s -----------------------\n" , qbCircBuffer->log_prefix, message );
}


// **** public functions ****

int qbCircBuffer_init( unsigned long buffer_size, const char *log_prefix, qbCircBuffer_t **new )
{
	*new = (qbCircBuffer_t*)malloc(sizeof(qbCircBuffer_t));
        if(*new==NULL)
        {
                TLEVEL(1) QBTRACE("%s : Cannot allocate CircBuffer itself: not enought memory\n",log_prefix);
                return QBCIRCBUFFER_RC_MEMALLOC;
        }
	(*new)->buffer_size=buffer_size;
	if( log_prefix )
	{
		strncpy( (*new)->log_prefix, log_prefix, QBCIRCBUFFER_LOG_PREFIX_MAX_SIZE );
		(*new)->log_prefix[QBCIRCBUFFER_LOG_PREFIX_MAX_SIZE-1]='\0';
	}
	else
		(*new)->log_prefix[0]='\0';

        (*new)->buffer=(char*)malloc(sizeof(char)*buffer_size);
	if((*new)->buffer==NULL)
	{
		TLEVEL(1) QBTRACE("%s : Cannot allocate CircBuffer: not enought memory\n",log_prefix);
		free(*new);
		(*new)=NULL;
		return QBCIRCBUFFER_RC_MEMALLOC;
	}
        (*new)->storage_pointer=(*new)->buffer;
	(*new)->reading_pointer=(*new)->buffer;

        if(pthread_mutex_init(&(*new)->pointer_move_mutex, NULL)!=0)
        {
                TLEVEL(1) QBTRACE("%s : Cannot initialize mutex (pointer_move_mutex)\n",log_prefix);
                free(*new);
                (*new)=NULL;
                return QBCIRCBUFFER_RC_MUTEXINIT;
        }
       if(pthread_mutex_init(&(*new)->storage_mutex, NULL)!=0)
        {
                TLEVEL(1) QBTRACE("%s : Cannot initialize mutex (storage_mutex)\n",log_prefix);
                free(*new);
                (*new)=NULL;
                return QBCIRCBUFFER_RC_MUTEXINIT;
        }
       if(pthread_mutex_init(&(*new)->reading_mutex, NULL)!=0)
        {
                TLEVEL(1) QBTRACE("%s : Cannot initialize mutex (reading_mutex)\n",log_prefix);
                free(*new);
                (*new)=NULL;
                return QBCIRCBUFFER_RC_MUTEXINIT;
        }
        (*new)->isClosed=0;

	return QBCIRCBUFFER_RC_OK;
}

int qbCircBuffer_storeData( qbCircBuffer_t *qbCircBuffer, const char *input, unsigned long input_len )
{
	int rc=QBCIRCBUFFER_RC_OK;
	if(qbCircBuffer->isClosed) return QBCIRCBUFFER_RC_CLOSED;
	if( input_len == 0 ) return QBCIRCBUFFER_RC_OK;
	if( input_len >= qbCircBuffer->buffer_size ) return QBCIRCBUFFER_RC_OVERFLOW;
	
	pthread_mutex_lock(&qbCircBuffer->storage_mutex);
	
	pthread_mutex_lock(&qbCircBuffer->pointer_move_mutex);
	char *storage_beginning=qbCircBuffer->storage_pointer;
	char *storage_end=_qbCircBuffer_move_pointer( qbCircBuffer , qbCircBuffer->storage_pointer , input_len -1 );
	char *reading_pointer=qbCircBuffer->reading_pointer;
	long freeSpace=qbCircBuffer->buffer_size-qbCircBuffer_getUsedSpaceSizeWithNoLock(qbCircBuffer);
	pthread_mutex_unlock(&qbCircBuffer->pointer_move_mutex);

	// ------------------------------------+-------------------------------------
	// if storage_beginning <= storage_end | if storage_beginning > storage_end
	// ------------------------------------+-------------------------------------
	//   stor.area      qbCircBuffer         |   stor.area      qbCircBuffer         
	//  +---------+-------------------+    | +---------+-------------------+    
	//  |         |                   |    | |   xx    |                   |    
	//  |...xx....|<-storage_beginning|    | |...xx....|<-storage_end      |    
	//  |   xx    |                   |    | |         |                   |
	//  |   xx    |                   |    | |         |                   |
	//  |...xx....|<-storage_end      |    | |...xx....|<-storage_beginning|
	//  |         |                   |    | |   xx    |                   |
	//  +---------+-------------------+    | +---------+-------------------+
	//                                     |
	// ------------------------------------+-------------------------------------
	
	// check buffer overflow
	if( input_len >= freeSpace )
		rc=QBCIRCBUFFER_RC_OVERFLOW;

	if(rc==QBCIRCBUFFER_RC_OK)
	{
		// store data
		if( storage_beginning <= storage_end )
		{
			memcpy(storage_beginning, input, input_len);
		}
		else // storage_beginning > storage_end
		{
			long input_len_1=qbCircBuffer->buffer + qbCircBuffer->buffer_size - storage_beginning;
			memcpy(storage_beginning, input, input_len_1);
			long input_len_2=input_len-input_len_1;
			memcpy(qbCircBuffer->buffer, &(input[input_len_1]), input_len_2);
		}
		// update storage_pointer position
		pthread_mutex_lock(&qbCircBuffer->pointer_move_mutex);
		qbCircBuffer->storage_pointer=_qbCircBuffer_move_pointer( qbCircBuffer , storage_end , 1 );
		pthread_mutex_unlock(&qbCircBuffer->pointer_move_mutex);
	}
	pthread_mutex_unlock(&qbCircBuffer->storage_mutex);
	return rc;
}

int qbCircBuffer_readData( qbCircBuffer_t *qbCircBuffer, char **output1, unsigned long *output1_len, char **output2, unsigned long *output2_len , char **read_id )
{
	*output1=NULL;
	*output1_len=0;
	*output2=NULL;
	*output2_len=0;

	if(qbCircBuffer->isClosed) return QBCIRCBUFFER_RC_CLOSED;

        pthread_mutex_lock(&qbCircBuffer->pointer_move_mutex);
        char *storage_pointer=qbCircBuffer->storage_pointer;
        char *reading_pointer=qbCircBuffer->reading_pointer;
        pthread_mutex_unlock(&qbCircBuffer->pointer_move_mutex);

	if( reading_pointer <= storage_pointer )
	{
		*output1=reading_pointer;
		*output1_len=storage_pointer-reading_pointer;
		*output2=NULL;
	        *output2_len=0;
	}
	else // reading_pointer > storage_pointer
	{
		*output1=reading_pointer;
		*output1_len=qbCircBuffer->buffer + qbCircBuffer->buffer_size - reading_pointer;
		*output2=qbCircBuffer->buffer;
		*output2_len=storage_pointer - qbCircBuffer->buffer;
	}
	*read_id=storage_pointer;

	return QBCIRCBUFFER_RC_OK;
}

int qbCircBuffer_releaseReadData( qbCircBuffer_t *qbCircBuffer, char *read_id )
{
	if(qbCircBuffer->isClosed) return QBCIRCBUFFER_RC_CLOSED;

	// update storage_pointer position
        pthread_mutex_lock(&qbCircBuffer->pointer_move_mutex);
        qbCircBuffer->reading_pointer=read_id;
        pthread_mutex_unlock(&qbCircBuffer->pointer_move_mutex);

        return QBCIRCBUFFER_RC_OK;
}

int qbCircBuffer_free( qbCircBuffer_t *qbCircBuffer )
{
	free( qbCircBuffer->buffer );
	qbCircBuffer->buffer=NULL;
	pthread_mutex_destroy(&qbCircBuffer->pointer_move_mutex);
	pthread_mutex_destroy(&qbCircBuffer->storage_mutex);
	pthread_mutex_destroy(&qbCircBuffer->reading_mutex);
	qbCircBuffer->isClosed=1;
	return QBCIRCBUFFER_RC_OK;
}

long qbCircBuffer_getUsedSpaceSize( qbCircBuffer_t *qbCircBuffer )
{
	pthread_mutex_lock(&qbCircBuffer->pointer_move_mutex);
    long ret = qbCircBuffer_getUsedSpaceSizeWithNoLock( qbCircBuffer );
	pthread_mutex_unlock(&qbCircBuffer->pointer_move_mutex);
	return ret;
}

