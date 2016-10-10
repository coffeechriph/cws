#ifndef ARRAY_H
#define ARRAY_H
#include "../types.h"
#include "log.h"
#define cws_array(type, var) \
struct {\
    type *data; \
    u32 size; \
    u32 length; \
    u32 type_size;\
} var;

#define cws_array_init(type,var,sz) \
if(sz > 0) { \
    var.data = malloc(sizeof(type)*sz); \
} \
else {\
    var.data = NULL;\
} \
var.length = 0; \
var.size = sz; \
var.type_size = sizeof(type)

#define cws_array_free(arr) free(arr.data); arr.size = 0; arr.length = 0; arr.type_size = 0
#define cws_array_push(arr, item) \
if(arr.data == NULL) { \
    arr.data = malloc(arr.type_size);\
    arr.size = 1; \
} \
if (arr.length < arr.size) {\
    arr.data[arr.length++] = item;\
}\
else { \
    void* X = realloc(arr.data, arr.type_size * arr.size*2);\
    if(X) {\
        arr.data = X; arr.data[arr.length++] = item; arr.size *= 2;\
    }\
    else {cws_log("Error allocating space!");}\
}
#define cws_array_pop(arr) (if (arr.length > 0) arr.length -= 1)
#define cws_array_remove(arr,index) if(index >= 0 && index <= arr.length) {for(u32 _arr##counter_ = index; _arr##counter_ < arr.length-1; ++_arr##counter_) {arr.data[_arr##counter_] = arr.data[_arr##counter_+1];} arr.data[arr.length-1] = 0; arr.length--;}
#define cws_array_clear(arr) (arr.length = 0)

#endif