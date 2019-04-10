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
static LCSFilterBlock _begin_filter_block = NULL;
static LCSFilterBlock _end_filter_block = NULL;

__unused static id (*orig_objc_msgSend)(id, SEL, ...);

typedef struct {
    void *pre;
    void *next;
    char *className;
    char *selName;
    uintptr_t lr;
} thread_lr_stack;

uintptr_t before_objc_msgSend(id self, uintptr_t lr, SEL sel) {
    
    char *className = (char *)object_getClassName(self);
    char *selName = (char *)sel;
    
    if (_begin_filter_block && _begin_filter_block(className, selName) > 0) {
        printf("start:class %s, selector %s\n", className, selName);
    }
    
    thread_lr_stack* ls = pthread_getspecific(_thread_key);
    if (ls == NULL) {
        ls = malloc(sizeof(thread_lr_stack));
        ls->pre = NULL;
        ls->next = NULL;
        ls->lr = lr;
        ls->className = className;
        ls->selName = selName;
        pthread_setspecific(_thread_key, (void *)ls);
    } else {
        thread_lr_stack* next = malloc(sizeof(thread_lr_stack));
        ls->next = next;
        next->pre = ls;
        next->next = NULL;
        next->lr = lr;
        next->className = className;
        next->selName = selName;
        pthread_setspecific(_thread_key, (void *)next);
    }
    return (uintptr_t)orig_objc_msgSend;
}

uintptr_t get_lr() {
    thread_lr_stack* ls = pthread_getspecific(_thread_key);
    pthread_setspecific(_thread_key, (void *)ls->pre);
    uintptr_t lr = ls->lr;
    char *className = ls->className;
    char *selName = ls->selName;
    if (_end_filter_block && _end_filter_block(className, selName) > 0) {
        printf("end:class %s, selector %s\n", ls->className, ls->selName);
    }
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

void lcs_start(LCSFilterBlock beginFilterBlock, LCSFilterBlock endFilterBlock) {
    
    _begin_filter_block = beginFilterBlock;
    _end_filter_block = endFilterBlock;
    
    pthread_key_create(&_thread_key, NULL);
    rebind_symbols((struct rebinding[1]){{"objc_msgSend", hook_objc_msgSend_x86, (void *)&orig_objc_msgSend}}, 1);
}

#endif



