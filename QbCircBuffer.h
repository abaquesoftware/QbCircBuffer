#ifndef __QBCIRCBUFFER_H_
#define __QBCIRCBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

// Return codes
#define QBCIRCBUFFER_RC_OK        0
#define QBCIRCBUFFER_RC_UNKNOWN   1
#define QBCIRCBUFFER_RC_OVERFLOW  2
#define QBCIRCBUFFER_RC_MEMALLOC  3
#define QBCIRCBUFFER_RC_MUTEXINIT 4
#define QBCIRCBUFFER_RC_CLOSED    5

//const char* CIRCBUFFER_RC_STRING[] = { "OK", "Unknwown error", "Buffer overflow", "Memory allocation failed" , "Mutex initialization failed" , "Buffer closed" };
extern char* QBCIRCBUFFER_RC_STRING[];

#define CIRCBUFFER_LOG_PREFIX_MAX_SIZE 1024

typedef struct qbCircBuffer_ {
        long buffer_size;
	char log_prefix[CIRCBUFFER_LOG_PREFIX_MAX_SIZE];
        char *buffer;
        char *storage_pointer;
        char *reading_pointer;
        pthread_mutex_t pointer_move_mutex;
        pthread_mutex_t storage_mutex;
        pthread_mutex_t reading_mutex;
	char isClosed;
} qbCircBuffer_t;

typedef char* qbCircBuffer_read_pointer_t;

int qbCircBuffer_init( unsigned long buffer_size, const char *log_prefix, qbCircBuffer_t **new );
int qbCircBuffer_storeData( qbCircBuffer_t *qbCircBuffer, const char *input, unsigned long input_len );
int qbCircBuffer_readData( qbCircBuffer_t *qbCircBuffer, char **output1, unsigned long *output1_len, char **output2, unsigned long *output2_len , qbCircBuffer_read_pointer_t *read_pt );
int qbCircBuffer_releaseReadData( qbCircBuffer_t *qbCircBuffer, qbCircBuffer_read_pointer_t read_pt );
int qBircBuffer_free( qbCircBuffer_t *qbCircBuffer );

// Get
long qbCircBuffer_getUsedSpaceSize( qbCircBuffer_t *qbCircBuffer );

// Debug
void qbCircBuffer_dump( qbCircBuffer_t *qbCircBuffer, const char *message );
void qbCircBuffer_hexDump( qbCircBuffer_t *qbCircBuffer, const char *message );

#ifdef __cplusplus
}
#endif

#endif // __QBCIRCBUFFER_H_

