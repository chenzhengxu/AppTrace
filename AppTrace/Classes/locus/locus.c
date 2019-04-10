//
//  hookObjcSend.c
//  fishhookdemo
//
//  Created by FanFamily on 2018/7/21.
//  Copyright © 2018年 Family Fan. All rights reserved.
//

#include "locus.h"



#include "fishhook.h"
#include <pthread.h>
#import <objc/message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dispatch/dispatch.h>
#import <objc/runtime.h>

#include "hook_objc_msgSend.h"
#include "hook_objc_msgSend_x86.h"

static pthread_key_t _thread_key;
static LCSFilterBlock _filter_block = NULL;

__unused static id (*orig_objc_msgSend)(id, SEL, ...);

typedef struct {
    void *pre;
    void *next;
    uintptr_t lr;
} thread_lr_stack;

uintptr_t before_objc_msgSend(id self, uintptr_t lr, SEL sel) {
    
    if (_filter_block && _filter_block((char *)object_getClassName(self), (char *)sel) > 0) {
        printf("class %s, selector %s\n", (char *)object_getClassName(self), (char *)sel);
    }
    
    thread_lr_stack* ls = pthread_getspecific(_thread_key);
    if (ls == NULL) {
        ls = malloc(sizeof(thread_lr_stack));
        ls->pre = NULL;
        ls->next = NULL;
        ls->lr = lr;
        pthread_setspecific(_thread_key, (void *)ls);
    } else {
        thread_lr_stack* next = malloc(sizeof(thread_lr_stack));
        ls->next = next;
        next->pre = ls;
        next->next = NULL;
        next->lr = lr;
        pthread_setspecific(_thread_key, (void *)next);
    }
    return (uintptr_t)orig_objc_msgSend;
}

uintptr_t get_lr() {
    thread_lr_stack* ls = pthread_getspecific(_thread_key);
    pthread_setspecific(_thread_key, (void *)ls->pre);
    uintptr_t lr = ls->lr;
    free(ls);
    return lr;
}

#ifdef __arm64__

void lcs_start(LCSFilterBlock filter) {
    
    _filter_block = filter;
    
    pthread_key_create(&_thread_key, NULL);
    rebind_symbols((struct rebinding[1]){{"objc_msgSend", hook_objc_msgSend, (void *)&orig_objc_msgSend}}, 1);
}

#else

void lcs_start(LCSFilterBlock filter) {
    
    _filter_block = filter;
    
    pthread_key_create(&_thread_key, NULL);
    rebind_symbols((struct rebinding[1]){{"objc_msgSend", hook_objc_msgSend_x86, (void *)&orig_objc_msgSend}}, 1);
}

#endif



