#ifndef BUCKET_ARRAY_H
#define BUCKET_ARRAY_H
#include "../types.h"
#include "log.h"
/*
A bucket array is a dynamicly growing array type which is split up into sections or "buckets"
Each bucket contains an static array with size of BUCKET_SIZE and once a bucket is filled a new bucket is allocated.
This ensures that allocated items within a bucket stays at the same memory location at all times. 
*/
#define cws_bucket_array(type, bsize) struct { \
    struct {type data[bsize]; bool occupied[bsize]; } **buckets; \
    u32 buckets_count; \
    u32 bucket_index; \
}

#define cws_bucket_array_init(type,var, sz) \
do \
{ \
if(sz > 1) { \
    var.buckets = malloc(sizeof(*var.buckets)*sz); \
    for(u32 _var##counter_ = 0; _var##counter_ < sz; ++_var##counter_) \
    { \
        var.buckets[_var##counter_] = malloc(sizeof(**var.buckets)); \
            for(i32 i = 0; i < sizeof(var.buckets[_var##counter_]->occupied); ++i) \
            {\
                var.buckets[_var##counter_]->occupied[i] = false; \
            }\
    } \
    var.buckets_count = sz; \
    var.bucket_index = 0; \
} \
else \
    { \
        var.buckets = malloc(sizeof(*var.buckets)); \
            var.buckets[0] = malloc(sizeof(**var.buckets)); \
                for(i32 i = 0; i < sizeof(var.buckets[0]->occupied); ++i) \
            {\
                    var.buckets[0]->occupied[i] = false; \
            }\
            var.buckets_count = 1; \
            var.bucket_index = 0; \
} \
} while(0);

#define cws_bucket_array_free(arr) \
do { \
for(u32 _arr##counter_ = 0; _arr##counter_ < arr.buckets_count; ++_arr##counter_){\
    free(arr.buckets[_arr##counter_]);\
    arr.buckets_count = 0; \
    arr.bucket_index = 0; \
}; \
    free(arr.buckets); arr.buckets_count = 0; arr.bucket_index = 0; \
} while(0);

#define cws_bucket_array_push(arr, item) \
do { \
    if(arr.buckets == NULL) { \
        arr.buckets = malloc(sizeof(*arr.buckets)); \
        arr.buckets[0] = malloc(sizeof(**arr.buckets)); \
        for(i32 i = 0; i < sizeof(arr.buckets[0]->occupied); ++i) \
        {\
            arr.buckets[0]->occupied[i] = false; \
        }\
        arr.buckets_count = 1; \
        arr.bucket_index = 0; \
    } \
    bool found = false; \
    for(u32 _arr##counter_ = 0; _arr##counter_ < sizeof(arr.buckets[0]->occupied); ++_arr##counter_) \
    { \
        if(!(arr.buckets[arr.bucket_index]->occupied[_arr##counter_])) \
        {\
            found = true; \
                arr.buckets[arr.bucket_index]->occupied[_arr##counter_] = true; \
                arr.buckets[arr.bucket_index]->data[_arr##counter_] = item; \
                break; \
        } \
    }\
    if(!found) \
    { \
        void* X = realloc(arr.buckets, sizeof(*arr.buckets)  * (arr.buckets_count+1)); \
        if(X) {\
            arr.buckets = X; \
            arr.buckets_count += 1; \
            arr.bucket_index += 1; \
            arr.buckets[arr.bucket_index] = malloc(sizeof(**arr.buckets)); \
            arr.buckets[arr.bucket_index]->data[0] = item; \
            for(i32 __arr##i__ = 0; __arr##i__ < sizeof(arr.buckets[arr.bucket_index]->occupied); ++__arr##i__) \
            { \
                arr.buckets[arr.bucket_index]->occupied[__arr##i__] = false; \
            } \
            arr.buckets[arr.bucket_index]->occupied[0] = true; \
} \
        else {cws_log("Error allocating space!");} \
    } \
    } while(0);

#define cws_bucket_array_remove(arr, bucket_index, index) \
do { \
    if(bucket_index >= 0 && bucket_index < arr.buckets_count) {\
        if(index >= 0 && index < sizeof(arr.buckets[0].occupied)) { \
            arr.buckets[bucket_index]->occupied[index] = false; \
            if(arr.bucket_index > bucket_index) {\
                arr.bucket_index = bucket_index; \
            } \
        } \
    } \
} while(0);

#define cws_bucket_array_clear(arr) \
do { \
    for(u32 _arr##counter_ = 0; _arr##counter_ < arr.buckets_count; ++_arr##counter_) \
    { \
        arr.buckets[_arr##counter_]->occupied = 0; \
    }\
    arr.bucket_index = 0; \
} while(0);

#define cws_bucket_array_index(arr, index) (arr.buckets[(u32)index/sizeof(arr.buckets[0]->occupied)]->data[(u32)index%sizeof(arr.buckets[0]->occupied)]) \

#define cws_bucket_array_last(arr, ret) \
for(u32 _arr##counter__ = sizeof(arr.buckets[0]->occupied)-1; _arr##counter__ >= 0; --_arr##counter__){\
    if((arr.buckets[arr.bucket_index]->occupied[_arr##counter__])){\
        ret = arr.bucket_index*sizeof(arr.buckets[0]->occupied) + (_arr##counter__); \
        break; \
    } \
}

#define cws_bucket_array_occupied(arr,index) (arr.buckets[index/sizeof(arr.buckets[0]->occupied)]->occupied[index%sizeof(arr.buckets[0]->occupied)])
#define cws_bucket_array_item_count(arr) (arr.buckets_count*sizeof(arr.buckets[0]->occupied))
#endif
