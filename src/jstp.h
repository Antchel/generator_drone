#ifndef __JSTP_H_
#define __JSTP_H_

#include "jsmn.h"
#include "jstp_gen.h"


#ifdef __cplusplus
extern "C" {
#endif



// size of input character buffer
#define JSTP_RX_BUFFER_SIZE 128
// size of outpuy character buffer
#define JSTP_TX_BUFFER_SIZE 128
// size of JSON tokens array (one token size - sizof(int)*4+1)
#define JSTP_JS_TOKENS_SIZE 10

#define     null    0
#define     false   0
#define     true    1

typedef enum {

    // no error
    JSTP_NO_ERROR = 0,
/*#------------------------------------------------------------------------#
  #                            4x   Request Error                          #
  #------------------------------------------------------------------------#*/
    // Bad request: not JSON
    JSTP_FAILED_PARSE_JSON = 40,
    // Bad request: args error
    JSTP_ARGS_ERROR = 41,
    // Cannot process the request due to large message and rx buffer overflow
    JSTP_RX_OVERFLOW = 42,
    // The requested path could not be found.
    JSTP_PATH_NOT_FOUND = 44,
    
/*#------------------------------------------------------------------------#
  #                            5x   Processing Error                       #
  #------------------------------------------------------------------------#*/
    // Does not recognize the request method, or it lacks the ability to fulfill the request.
    // Usually this implies future availability.
    JSTP_ITEM_NOT_IMPLEMENTED = 51,
    // Response too large and internal response buffer overflowed.
    JSTP_TX_OVERFLOW = 57,


} jstp_error_t;

/**
 * API
 */

// init jstp args (call one time)
void jstp_init();

// put char to rx buffer. Light function, can call from interrupt.
void jstp_rx_push_char(char c);

// check rx-buffer, execute tasks, write to tx buffer (call as many as possible)
void jstp_tick();

// get one char from tx buffer. return bool - is tx buffer contain data to read
int jstp_tx_pop_char(char* c);







/**
 * Args methods
 */
// compare two null-terminal string
int jstp_str_compare(char *x, const char *y);

// check that starts integer value
int jstp_rx_long(const char *src, jsmntok_t *t, long* out_value);

// check that starts array
int jstp_rx_array(const char *src, jsmntok_t *t, size_t* out_array_size);

// compare token string and null-terminal string
int jstp_rx_cmp_str(const char *src, jsmntok_t *t,const char* comapareWith);
// 
int jstp_rx_str(char *src, jsmntok_t *t,char** out_str, size_t * out_str_len);
// 
int jstp_rx_bool(char *src, jsmntok_t *t, int * out_bool);
//
int jstp_rx_obj(const char *src, jsmntok_t *t, size_t* out_size);
//
int jstp_rx_float(const char *src, jsmntok_t *t, float* value);

/**
 * Result methods
 */

// push to tx buffer success start: {"OK":
void jstp_tx_begin();
// push to tx buffer end: }\n
void jstp_tx_end();

// push to tx buffer array begin: [
void jstp_tx_array_begin();
// push to tx buffer array item separator: ,
void jstp_tx_array_end_item();
// push to tx buffer array end: ]
void jstp_tx_array_end();

//begin object
void jstp_tx_obj_begin();
//write string and ':'
void jstp_tx_obj_key(const char * key);
// push to tx buffer item separator: ,
void jstp_tx_obj_end_item();
//end object
void jstp_tx_obj_end();

// push to tx buffer integer value
void jstp_tx_long(long v);
// push to tx buffer string value with quotes: "<str>"
void jstp_tx_str(const char* str);
// push to tx buffer one char
void jstp_tx_char(char c);
// push tx buffer 'true', if (boolValue != 0);  'false' otherwise
void jstp_tx_bool(int boolValue);
// push tx 'null'
void jstp_tx_null();
// push float value
void jstp_tx_float(float value);










#ifdef __cplusplus
}
#endif

#endif /* __JSTP_H_ */
