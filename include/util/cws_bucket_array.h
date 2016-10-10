#ifndef BUCKET_ARRAY_H
#define BUCKET_ARRAY_H
#include "../types.h"
#include "log.h"
/*
A bucket array is a dynamicly growing array type which is split up into sections or "buckets"
Each bucket contains an static array with size of BUCKET_SIZE and once a bucket is filled a new bucket is allocated.
This ensures that allocated items within a bucket stays at the same memory location at all times. 
*/
#define BUCKET_SIZE 32
#define cws_bucket_array(type, var) struct { \
    struct {type data[BUCKET_SIZE]; u32 occupied; } **buckets; \
    u32 buckets_count; \
    u32 bucket_index; \
    u32 type_size; \
} var;

#define cws_bucket_array_init(type,var,sz) \
if(sz > 0) { \
    var.buckets = malloc(sizeof(var.buckets)*sz); \
    for(u32 _var##counter_ = 0; _var##counter_ < sz; ++_var##counter_) \
    { \
        var.buckets[_var##counter_] = malloc(sizeof(**var.buckets)); \
        var.buckets[_var##counter_]->occupied = 0; \
    } \
    var.buckets_count = sz; \
    var.bucket_index = 0; \
    var.type_size = sizeof(type); \
} \
else { \
    var.buckets = NULL; \
    var.buckets_count = 0; \
    var.bucket_index = 0; \
    var.type_size = sizeof(type); \
} \

#define cws_bucket_array_free(arr) \
for(u32 _arr##counter_ = 0; _arr##counter_ < arr.buckets_count; ++_arr##counter_){\
    free(arr.buckets[_arr##counter_]);\
}; \
free(arr.buckets); arr.buckets_count = 0; arr.bucket_index = 0; arr.type_size = 0

#define cws_bucket_array_push(arr, item) ({ \
    if(arr.buckets == NULL) { \
        arr.buckets = malloc(sizeof(arr.buckets)); \
        arr.buckets[0] = malloc(sizeof(**arr.buckets)); \
        arr.buckets[0]->occupied = 0; \
        arr.buckets_count = 1; \
        arr.bucket_index = 0; \
    } \
    if (arr.buckets[arr.bucket_index]->occupied<UINT_MAX) { \
        for(u32 _arr##counter_ = 0; _arr##counter_ < BUCKET_SIZE; ++_arr##counter_) { \
            if(((arr.buckets[arr.bucket_index]->occupied)&(1<<_arr##counter_)) == 0){\
                arr.buckets[arr.bucket_index]->occupied |= (1<<_arr##counter_); \
                arr.buckets[arr.bucket_index]->data[_arr##counter_] = item; \
                break; \
            }\
        } \
    } \
else { \
        void* X = realloc(arr.buckets, sizeof(arr.buckets)  * arr.buckets_count+1); \
        if(X) {\
            arr.buckets = X; \
            arr.buckets_count += 1; \
            arr.bucket_index += 1; \
            arr.buckets[arr.bucket_index] = malloc(sizeof(**arr.buckets)); \
            arr.buckets[arr.bucket_index]->data[0] = item; \
            arr.buckets[arr.bucket_index]->occupied = 1; \
        } \
        else {cws_log("Error allocating space!");} }})

#define cws_bucket_array_remove(arr, bucket_index, index){ \
    (if(bucket_index >= 0 && bucket_index < arr.buckets_count) {\
        if(index >= 0 && index < BUCKET_SIZE) { \
            arr.buckets[bucket_index]->occupied &= ~(1<<index); \
            if(arr.bucket_index > bucket_index) {\
                arr.bucket_index = bucket_index; \
            } \
        } \
    } \
)}
#define cws_bucket_array_clear(arr) {(\
    for(u32 _arr##counter_ = 0; _arr##counter_ < arr.buckets_count; ++_arr##counter_) \
    { \
        arr.buckets[_arr##counter_]->occupied = 0; \
    }\
    arr.bucket_index = 0;)}

#define cws_bucket_array_index(arr, index)(arr.buckets[(u32)index/BUCKET_SIZE]->data[(u32)index%BUCKET_SIZE])
#define cws_bucket_array_last(arr, ret) \
for(u32 _arr##counter__ = BUCKET_SIZE-1; _arr##counter__ >= 0; --_arr##counter__){\
    if((arr.buckets[arr.bucket_index]->occupied&(1<<_arr##counter__))){\
        ret = arr.bucket_index*BUCKET_SIZE + (_arr##counter__); \
        break; \
    } \
} \


#define cws_bucket_array_occupied(arr,index)((arr.buckets[(u32)index/BUCKET_SIZE]->occupied&(1<<(index%BUCKET_SIZE))))
#define cws_bucket_array_item_count(arr) (arr.buckets_count*BUCKET_SIZE)
#endif