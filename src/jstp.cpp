

#include "jstp.h"



jsmn_parser p;
char jstp_rx_buff[JSTP_RX_BUFFER_SIZE];
char jstp_tx_buff[JSTP_TX_BUFFER_SIZE];
jsmntok_t t[JSTP_JS_TOKENS_SIZE];

// jstp state
typedef enum
{
    JSTP_MODE_NOT_INIT = 0,
    JSTP_MODE_READ = 1,
    JSTP_MODE_WRITE = 2,
} jstp_mode;


typedef struct
{
    char* buff;
    size_t r_inx;
    size_t w_inx;
    size_t size;
} jstp_stream_t;

typedef struct
{
    jstp_mode mode;
    jstp_stream_t rx;
    jstp_stream_t tx;
} jstp_engn_t;

jstp_engn_t jstp;



void jstp_init()
{

    jstp.mode = JSTP_MODE_READ;

    jstp.rx.buff = jstp_rx_buff;
    jstp.rx.r_inx = 0;
    jstp.rx.w_inx = 0;
    jstp.rx.size = JSTP_RX_BUFFER_SIZE;

    jstp.tx.buff = jstp_tx_buff;
    jstp.tx.r_inx = 0;
    jstp.tx.w_inx = 0;
    jstp.tx.size = JSTP_TX_BUFFER_SIZE;
}

void jstp_rx_push_char(char c)
{
    // read mode and no rx overflow
    if (jstp.mode == JSTP_MODE_READ && jstp.rx.w_inx < jstp.rx.size)
    {

        jstp.rx.buff[jstp.rx.w_inx++] = c;
    }
    // rx overflow will be handled at next tick
}

int jstp_tx_pop_char(char* c)
{

    // write mode and exit chars to write and no tx overflow
    if (jstp.mode == JSTP_MODE_WRITE && jstp.tx.r_inx < jstp.tx.w_inx && jstp.tx.r_inx < jstp.tx.size)
    {
        *c = jstp.tx.buff[jstp.tx.r_inx++];
        return true;
    }
    return false;
}

void jstp_tx_begin()
{
	jstp_tx_char(0x0A);
    jstp_tx_char('{');
    jstp_tx_str("OK");
    jstp_tx_char(':');
}

void jstp_tx_end()
{
    jstp_tx_char('}');
    jstp_tx_char(0x0D);
	jstp_tx_char(0x0A);
}


void jstp_tx_array_begin()
{
    jstp_tx_char('[');
}

void jstp_tx_array_end_item()
{
    jstp_tx_char(',');
}

void jstp_tx_array_end()
{
    jstp_tx_char(']');
}

void jstp_tx_obj_begin()
{
    jstp_tx_char('{');
}

void jstp_tx_obj_end()
{
    jstp_tx_char('}');
}

void jstp_tx_obj_end_item()
{
    jstp_tx_char(',');
}

void jstp_tx_obj_key(const char * key)
{
    jstp_tx_str(key);
    jstp_tx_char(':');
}


void jstp_tx_char(char c)
{
    if (jstp.tx.w_inx < jstp.tx.size)
    {
        jstp.tx.buff[jstp.tx.w_inx++] = c;
    }
    // tx overflow will be handled at next tick
}

void jstp_tx_str(const char* str)
{
    const char * c = str;
    jstp_tx_char('\"');
    while(*c)
    {
        jstp_tx_char(*c);
        c++;
    }
    jstp_tx_char('\"');
}

void jstp_tx_long(long v)
{
    char buff[16];
    size_t inx = 16-1;
    size_t i;

    if(v == 0)
    {
        jstp_tx_char('0');
		return;
    }

    if(v < 0)
    {
        jstp_tx_char('-');
        v = -v;
    }
    //calc
    do
    {
        long iv = v / 10;
        buff[inx--] = (v - iv * 10) + '0';
        v = iv;
    }
    while(v && inx>=0);
    // write

    for (i = inx + 1; i < 16; ++i)
    {
        jstp_tx_char(buff[i]);
    }
}

void jstp_tx_float(float v)
{
    unsigned long tmp;
    int digit;
    int i;

    if (v < 0)
	{
		jstp_tx_char('-');
		v=-v;
	}
    tmp = (unsigned long)v;
    jstp_tx_long(tmp);

    v -= (float)tmp;

	if (v==0) return;

    jstp_tx_char('.');

    i=0;
    while(v > 0 && i < 6)
    {
        digit = v * 10;
        jstp_tx_char('0' + digit);
        v = (float)v*10.0F - (float)digit;
        i++;
    }




}

void jstp_tx_error(jstp_error_t err)
{
    //remove old tx data
    jstp.tx.r_inx = 0;
    jstp.tx.w_inx = 0;

	jstp_tx_char(0x0A);
    jstp_tx_char('{');
    jstp_tx_str("ERR");
    jstp_tx_char(':');
    jstp_tx_long(err);
    jstp_tx_char(',');
    switch(err)
    {
        case JSTP_ITEM_NOT_IMPLEMENTED:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Not implemented");
            break;
        case JSTP_RX_OVERFLOW:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Bad request: too large");
            break;
        case JSTP_TX_OVERFLOW:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Insufficient Storage");
            break;
        case JSTP_FAILED_PARSE_JSON:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Bad request: not JSON");
            break;
        case JSTP_PATH_NOT_FOUND:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Not Found");
            break;
        case JSTP_ARGS_ERROR:
            jstp_tx_str("MSG");
            jstp_tx_char(':');
            jstp_tx_str("Bad request: args error");
            break;
    }

    jstp_tx_char('}');
	jstp_tx_char(0x0D);
	jstp_tx_char(0x0A);
}



void jstp_tx_bool(int boolValue)
{
    if (boolValue)
    {
        jstp_tx_char('t');
        jstp_tx_char('r');
        jstp_tx_char('u');
        jstp_tx_char('e');
    }
    else
    {
        jstp_tx_char('f');
        jstp_tx_char('a');
        jstp_tx_char('l');
        jstp_tx_char('s');
        jstp_tx_char('e');
    }

}

size_t jstp_strlen(const char* p)
{
    size_t len;
    len = 0;
    if(p == 0) return 0;
    while(*p)
    {
        len++;
        p++;
    }
    return len;
}

int jstp_str_compare(char *x, const char *y)
{
    size_t len = jstp_strlen(x);
    if (len != jstp_strlen(y))
    {
        return false;
    }
    while(len--)
    {
        if (*x++ != *y++) return false;
    }
    return true;

}

int jstp_rx_obj(const char *src, jsmntok_t *t, size_t* out_size)
{
   if (t->type != JSMN_OBJECT)  return false;
   *out_size = t->size;
   return true;
}

int jstp_rx_cmp_str(const char *src, jsmntok_t *t,const char* comapareWith)
{
    size_t srcLen;
    size_t cmpLen;
    size_t minLen;

    // if token not string -> return false
    if (t->type != JSMN_STRING) return false;

    srcLen = t->end - t->start;
    cmpLen = jstp_strlen(comapareWith);

    // if token length != comparable str len -> return false
    if (srcLen != cmpLen) return false;

    // comapre as simple string
    src = &src[t->start];
    while(cmpLen--)
    {
        if (*src++ != *comapareWith++) return false;
    }
    return true;
}


void jstp_split_path(char* txt, char** data)
{
    char c;

    // replace first separtaor
    if (*txt == '/') *txt = ' ';
    while(c = *txt)
    {
        if (c == '.' || c == '/')
        {
            *txt = '.';
        }
        // is separator
        else if ((c >= 'A' && c <= 'Z'))
        {
            // to lower case
            *txt = *txt + 32;
        }
        // is text char
        else if ((c >= 'a' && c <= 'z') || c == '_')
        {
            // skip
        }
        else if (c == ':' || c == '=' || c == ' ')
        {
            *txt = 0;
            *data = ++txt;
            return;
        }
        ++txt;
    }
    // has no data
    *data = txt;
}

void jstp_execute(char* txt)
{
    char * data = 0;
    int r;
    size_t data_length;

    jstp_split_path(txt,&data);

    data_length = jstp_strlen(data);
    jsmn_init(&p);

    r = jsmn_parse(&p, data, data_length, t, JSTP_JS_TOKENS_SIZE);

    if (r < 0)
    {
        jstp_tx_error(JSTP_FAILED_PARSE_JSON);
        return;
    }
    r = jstp_execute_gen(txt,data,t,JSTP_JS_TOKENS_SIZE);
    if (r != 0)
    {
        jstp_tx_error(r);
    }
}

int jstp_parse_dec_digit(char c)
{
    int v = c;
    v -= '0';
    if(v < 0 || v > 9) return -1;
    return v;
}

int jstp_rx_bool(char *src, jsmntok_t *t, int * out_bool)
{
    size_t src_length;
    if (t->type != JSMN_PRIMITIVE) return false;
    src_length = t->end - t->start;

    if (src_length == 4 && src[t->start] == 't')
    {
        *out_bool = true;
        return true;
    }
    if (src_length == 5 && src[t->start] == 'f')
    {
        *out_bool = false;
        return true;
    }

    return false;

}


void jstp_tx_null()
{
    jstp_tx_char('n');
    jstp_tx_char('u');
    jstp_tx_char('l');
    jstp_tx_char('l');

}

int jstp_rx_long(const char *src, jsmntok_t *t, long* out_value)
{
    size_t src_length;
    size_t  length = 0;
    long     acc = 0;
    long     minus = false;
    size_t  inx = t->start;

    if (t->type != JSMN_PRIMITIVE) return false;

    src_length = t->end - t->start;

    // empty string
    if (src_length == 0) return false;



    while(src[inx] == ' ')
    {
        inx++;
    }

    if(src[inx] == '-')
    {
        inx++;
        minus = true;
    }

    while(inx < t->end)
    {
        char c = src[inx++];
        long digit = jstp_parse_dec_digit(c);
        if(digit < 0)
        {
            return false;
        }
        acc = acc * 10 + digit;
    }

    if(minus) acc *= -1;

    *out_value = acc;
    return true;
}

int jstp_rx_float(const char *src, jsmntok_t *t, float* value)
{
    size_t inx;
    int sign;
    char c;
    int digit;
    float fraction = 0;
    int i;
    unsigned int fraction_weight;

    fraction = 0.0F;
    inx = t->start;
    *value = 0;




    while(src[inx] == ' ')
    {
        inx++;
    }
    if(src[inx] == '-')
    {
        inx++;
        sign = -1;
    }
    else
    {
        sign = 1;
    }


    while(inx < t->end)
    {
        c = src[inx++];
        if (c == '.') break;

        digit = jstp_parse_dec_digit(c);
        if(digit < 0)
        {
            return false;
        }
        *value = *value * 10 + digit;
    }
    fraction_weight = 10;
    for(i=0; i<8; i++)
    {
        if (inx >= t->end) break;
        c = src[inx++];
        digit = jstp_parse_dec_digit(c);
        if(digit < 0)
        {
            return false;
        }
        fraction = fraction + (float)digit / (float)fraction_weight;
        fraction_weight *=10;
    }
	*value += fraction;
    *value *= sign;

    return true;
}

int jstp_rx_str(char *src, jsmntok_t *t,char** out_str, size_t * out_str_len)
{
    // if token not string -> return false
    if (t->type != JSMN_STRING) return false;

    *out_str_len = t->end - t->start;
    src +=t->start;
    *out_str = src;


    return true;
}

int jstp_rx_array(const char *src, jsmntok_t *t, size_t* out_array_size)
{
    if (t->type != JSMN_ARRAY) return false;
    *out_array_size = t->size;
    return true;
}



void jstp_tick(){

    while(1)
    {
        if (jstp.mode == JSTP_MODE_READ)
        {
            char c;
            // check rx overflow
            if (jstp.rx.w_inx >= jstp.rx.size || jstp.rx.r_inx >= jstp.rx.size)
            {
                // write to tx error
                jstp_tx_error(JSTP_RX_OVERFLOW);
                // switch to write mode
                jstp.mode = JSTP_MODE_WRITE;

                // clear rx stream
                jstp.rx.r_inx = 0;
                jstp.rx.w_inx = 0;
                return;
            }
            // contain new data?
            if (jstp.rx.w_inx == 0 || jstp.rx.r_inx >= jstp.rx.w_inx)
            {
                // nothing to read
                return;
            }

            // read one byte
            c = jstp.rx.buff[jstp.rx.r_inx];

            switch (c)
            {
                // LF -> clear buffer
                case 0x0a:
                    jstp.rx.r_inx = 0;
                    jstp.rx.w_inx = 0;
                    break;
                // backspace
                case 0x08:
                    if (jstp.rx.r_inx > 0) --jstp.rx.r_inx;
                    break;
                // CR -> execute
                case 0x0d:
                    // replace last symbol by null
                    jstp.rx.buff[jstp.rx.r_inx] = 0;

                    //execute request
                    jstp_execute(jstp.rx.buff);

                    // switch to write mode
                    jstp.mode = JSTP_MODE_WRITE;

                    // clear rx stream
                    jstp.rx.r_inx = 0;
                    jstp.rx.w_inx = 0;
                    break;
                default:
                    ++jstp.rx.r_inx;
            }
        }

        if (jstp.mode == JSTP_MODE_WRITE)
        {
            if (jstp.tx.r_inx >= jstp.tx.w_inx)
            {
                // all message sended -> switch to read mode
                jstp.mode = JSTP_MODE_READ;
                jstp.tx.r_inx = 0;
                jstp.tx.w_inx = 0;
            }
            return;
        }
    }
}

