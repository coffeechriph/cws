#ifndef ARRAY_H
#define ARRAY_H
#include "../types.h"
#include "log.h"
#define cws_array(type) \
struct {\
    type *data; \
    u32 size; \
    u32 length; \
}

#define cws_array_init(type,var,sz) \
do { \
if(sz > 0) { \
    var.data = malloc(sizeof(type)*sz); \
} \
else {\
    var.data = NULL;\
} \
var.length = 0; \
var.size = sz; \
}while(0);

#define cws_array_free(arr) free(arr.data); arr.size = 0; arr.length = 0
#define cws_array_push(arr, item) \
do { \
if(arr.data == NULL) { \
    arr.data = malloc(sizeof(*arr.data));\
    arr.size = 1; \
    arr.length = 0; \
} \
if (arr.length < arr.size) {\
    arr.data[arr.length++] = item;\
}\
else { \
    void* X = realloc(arr.data, sizeof(*arr.data) * arr.size*2);\
    if(X) {\
        arr.data = X; arr.data[arr.length++] = item; arr.size *= 2;\
    }\
    else {cws_log("Error allocating space!");}\
} \
} while(0);

#define cws_array_pop(arr) do { if (arr.length > 0) arr.length -= 1 } while(0);
#define cws_array_remove(arr,index) \
do { \
    if(index >= 0 && index <= arr.length) { \
        for(u32 _arr##counter_ = index; _arr##counter_ < arr.length-1; ++_arr##counter_) { \
            arr.data[_arr##counter_] = arr.data[_arr##counter_+1];\
        } \
        arr.data[arr.length-1] = 0; arr.length--;\
    } \
} while(0);
#define cws_array_clear(arr) (arr.length = 0)
#endif
