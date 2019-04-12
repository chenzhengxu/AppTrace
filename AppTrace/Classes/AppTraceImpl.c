//
//  hookObjcSend.c
//  fishhookdemo
//
//  Created by FanFamily on 2018/7/21.
//  Copyright © 2018年 Family Fan. All rights reserved.
//

#include "AppTraceImpl.h"

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
#include <mach/mach_time.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

static pthread_t _main_ptread;

static int lcs_print = 0;
static pthread_key_t _thread_lr_stack_key;
static char _log_path[1024];
static FILE * _log_file = NULL;
static long long begin_;
static __uint64_t main_thread_id=0;
__unused static id (*orig_objc_msgSend)(id, SEL, ...);
dispatch_queue_t queue_;

typedef struct {
    void *pre;
    void *next;
    uintptr_t lr;
    char* obj;
    char* sel;
    uint64_t begin_elapsed;
} thread_lr_stack;

long long lcs_getCurrentTime() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long microseconds = te.tv_sec*1000*1000LL + te.tv_usec;
//    printf("%lld\n", microseconds);
    return microseconds;
}

void write_method_log(char* obj, char* sel, uint64_t beginElapsed, uint64_t endElapsed) {
    // thread_id
    pthread_t thread = pthread_self();
    __uint64_t thread_id=0;
    pthread_threadid_np(thread,&thread_id);
    char threadName[32];
    if (main_thread_id == thread_id) {
        sprintf(threadName, "%s", "Main");
    } else {
        sprintf(threadName, "Sub[%llu]", thread_id);
    }
    // [class]sel
    unsigned long repl_len = strlen(obj) + strlen(sel) + 10;
    char *repl_name = malloc(repl_len);
    snprintf(repl_name, repl_len, "[%s]%s", obj, sel);
    // print
    char begin_str[512];
    char end_str[512];
    sprintf(begin_str, "{\"name\":\"%s\",\"cat\":\"catname\",\"ph\":\"%s\",\"pid\":666,\"tid\":\"%s\",\"ts\":%llu},\n", repl_name, "B", threadName, beginElapsed);
    sprintf(end_str, "{\"name\":\"%s\",\"cat\":\"catname\",\"ph\":\"%s\",\"pid\":666,\"tid\":\"%s\",\"ts\":%llu},\n", repl_name, "E", threadName, endElapsed);
    unsigned long begin_length = strlen(begin_str);
    unsigned long end_length = strlen(end_str);
    unsigned long length = begin_length + end_length + 10;
    char *log = malloc(length);
    snprintf(log, length, "%s%s", begin_str, end_str);

//    printf("%s", log);
    free(repl_name);
    dispatch_async(queue_, ^{
        fputs(log, _log_file);
        free(log);
    });
}

void before_objc_msgSend(id self, SEL sel, ...) {}

uintptr_t save_lr(id self, SEL sel, uintptr_t lr)
{
    thread_lr_stack* ls = pthread_getspecific(_thread_lr_stack_key);
    uint64_t time = lcs_getCurrentTime();
    uint64_t elapsed = time - begin_;
    if (ls == NULL) {
        ls = malloc(sizeof(thread_lr_stack));
        ls->pre = NULL;
        ls->next = NULL;
        ls->lr = lr;
        ls->obj = (char *)object_getClassName(self);
        ls->sel = (char *)sel;
        ls->begin_elapsed = elapsed;
        pthread_setspecific(_thread_lr_stack_key, (void *)ls);
    } else {
        thread_lr_stack* next = NULL;

        pthread_t thread = pthread_self();
        __uint64_t thread_id=0;
        pthread_threadid_np(thread,&thread_id);
        // 主线程的栈可以复用
        if (main_thread_id == thread_id) {
            if (ls->next == NULL) {
                next = malloc(sizeof(thread_lr_stack));
                ls->next = next;
                next->next = NULL;
                next->pre = ls;
            } else {
                next = ls->next;
            }
        } else {
            next = malloc(sizeof(thread_lr_stack));
            ls->next = next;
            next->next = NULL;
            next->pre = ls;
        }
        next->lr = lr;
        next->obj = (char *)object_getClassName(self);
        next->sel = (char *)sel;
        next->begin_elapsed = elapsed;
        pthread_setspecific(_thread_lr_stack_key, (void *)next);
    }
    return (uintptr_t)orig_objc_msgSend;
}

uintptr_t get_lr() {
    thread_lr_stack* ls = pthread_getspecific(_thread_lr_stack_key);
    pthread_setspecific(_thread_lr_stack_key, (void *)ls->pre);
    uintptr_t lr = ls->lr;
    // log
    if (lcs_print > 0) {
        uint64_t time = lcs_getCurrentTime();
        uint64_t elapsed = time - begin_;
        uint64_t interval = elapsed - ls->begin_elapsed;
//        printf("%s %s %llu %llu\n", (char *)ls->obj, (char *)ls->sel, ls->begin_elapsed, elapsed);
        // 时间限制 为了减少log数据量
        if (interval > method_min_duration) {
            write_method_log((char *)ls->obj, (char *)ls->sel, ls->begin_elapsed, elapsed);
        }
    }
    pthread_t thread = pthread_self();
    __uint64_t thread_id=0;
    pthread_threadid_np(thread,&thread_id);
    if (main_thread_id != thread_id) {
        // 非主线程的应当释放
        free(ls);
    }
    return lr;
}

void lcs_stop_print() {
    lcs_print = 0;
}

void lcs_resume_print() {
    lcs_print = 1;
}

extern id hook_objc_msgSend(id, SEL, ...);

#ifdef __arm64__

__asm__  (
          ".text\n"
          ".global _hook_objc_msgSend\n"
          
          "_hook_objc_msgSend:\n"
          
          "stp q6, q7, [sp, #-32]!\n"
          "stp q4, q5, [sp, #-32]!\n"
          "stp q2, q3, [sp, #-32]!\n"
          "stp q0, q1, [sp, #-32]!\n"
          
          "stp x6, x7, [sp, #-16]!\n"
          "stp x4, x5, [sp, #-16]!\n"
          "stp x2, x3, [sp, #-16]!\n"
          "stp x0, x1, [sp, #-16]!\n"
          "stp x8, lr, [sp, #-16]!\n"
          
          "bl _before_objc_msgSend\n"
          
          "ldp x8, lr, [sp], #16\n"
          "ldp x0, x1, [sp], #16\n"
          "stp x0, x1, [sp, #-16]!\n"
          "stp x8, lr, [sp, #-16]!\n"
          
          "mov x2, lr\n"
          
          "bl _save_lr\n"
          
          "mov x9, x0\n"
          
          "ldp x8, lr, [sp], #16\n"
          "ldp x0, x1, [sp], #16\n"
          "ldp x2, x3, [sp], #16\n"
          "ldp x4, x5, [sp], #16\n"
          "ldp x6, x7, [sp], #16\n"
          
          "ldp q0, q1, [sp], #32\n"
          "ldp q2, q3, [sp], #32\n"
          "ldp q4, q5, [sp], #32\n"
          "ldp q6, q7, [sp], #32\n"
          
          "blr x9\n"
          "stp x0, x1, [sp, #-16]!\n"
          "stp q0, q1, [sp, #-32]!\n"
          "bl _get_lr\n"
          "mov lr, x0\n"
          "ldp q0, q1, [sp], #32\n"
          "ldp x0, x1, [sp], #16\n"
          "ret\n"
          
          "ret\n"
);

#elif __LP64__
__asm__  (
          ".text\n"
          ".global _hook_objc_msgSend\n"
          
          "_hook_objc_msgSend:\n"
          "pushq  %rbp\n"
          "movq %rsp, %rbp\n"
          "subq $0x210, %rsp\n"
          
          "movq  %xmm15, -0x210(%rbp)\n"
          "movq  %xmm14, -0x200(%rbp)\n"
          "movq  %xmm13, -0x190(%rbp)\n"
          "movq  %xmm12, -0x180(%rbp)\n"
          "movq  %xmm11, -0x170(%rbp)\n"
          "movq  %xmm10, -0x160(%rbp)\n"
          "movq  %xmm9, -0x150(%rbp)\n"
          "movq  %xmm8, -0x140(%rbp)\n"
          "movq  %xmm7, -0x130(%rbp)\n"
          "movq  %xmm6, -0x120(%rbp)\n"
          "movq  %xmm5, -0x110(%rbp)\n"
          "movq  %xmm4, -0x100(%rbp)\n"
          "movq  %xmm3, -0x90(%rbp)\n"
          "movq  %xmm2, -0x80(%rbp)\n"
          "movq  %xmm1, -0x70(%rbp)\n"
          "movq  %xmm0, -0x60(%rbp)\n"
          "movq  %rbx, -0x58(%rbp)\n"
          "movq  %rdi, -0x50(%rbp)\n"
          "movq  %rsi, -0x48(%rbp)\n"
          "movq  %rdx, -0x40(%rbp)\n"
          "movq  %rcx, -0x38(%rbp)\n"
          "movq  %r8, -0x30(%rbp)\n"
          "movq  %r9, -0x28(%rbp)\n"
          "movq  %r12, -0x20(%rbp)\n"
          "movq  %r13, -0x18(%rbp)\n"
          "movq  %r14, -0x10(%rbp)\n"
          "movq  %r15, -0x8(%rbp)\n"
          
          "call _before_objc_msgSend\n"
          
          "movq   -0x50(%rbp), %rdi\n"
          "movq   -0x48(%rbp), %rsi\n"
          "movq 0x8(%rbp), %rdx\n"
          
          "call _save_lr\n"
          
          "movq   -0x210(%rbp), %xmm15\n"
          "movq   -0x200(%rbp), %xmm14\n"
          "movq   -0x190(%rbp), %xmm13\n"
          "movq   -0x180(%rbp), %xmm12\n"
          "movq   -0x170(%rbp), %xmm11\n"
          "movq   -0x160(%rbp), %xmm10\n"
          "movq   -0x150(%rbp), %xmm9\n"
          "movq   -0x140(%rbp), %xmm8\n"
          "movq   -0x130(%rbp), %xmm7\n"
          "movq   -0x120(%rbp), %xmm6\n"
          "movq   -0x110(%rbp), %xmm5\n"
          "movq   -0x100(%rbp), %xmm4\n"
          "movq   -0x90(%rbp), %xmm3\n"
          "movq   -0x80(%rbp), %xmm2\n"
          "movq   -0x70(%rbp), %xmm1\n"
          "movq   -0x60(%rbp), %xmm0\n"
          "movq   -0x58(%rbp), %rbx\n"
          "movq   -0x50(%rbp), %rdi\n"
          "movq   -0x48(%rbp), %rsi\n"
          "movq   -0x40(%rbp), %rdx\n"
          "movq   -0x38(%rbp), %rcx\n"
          "movq   -0x30(%rbp), %r8\n"
          "movq   -0x28(%rbp), %r9\n"
          "movq   -0x20(%rbp), %r12\n"
          "movq   -0x18(%rbp), %r13\n"
          "movq   -0x10(%rbp), %r14\n"
          "movq   -0x8(%rbp), %r15\n"
          
          "addq $0x210, %rsp\n"
          
          "popq %rbp\n"
          "addq $0x8, %rsp\n"
          "call *%rax\n"
          "pushq %rax\n"
          "pushq %rdx\n"
          
          "subq $0x20, %rsp\n"
          "movq  %xmm0, 0x0(%rsp)\n"
          "movq  %xmm1, 0x10(%rsp)\n"
          "call _get_lr\n"
          "movq %rax, %r10\n"
          "movq  0x0(%rsp), %xmm0\n"
          "movq  0x10(%rsp), %xmm1\n"
          "addq $0x20, %rsp\n"
          
          "popq %rdx\n"
          "popq %rax\n"
          "push %r10\n"
          "retq\n"
          "ret\n"
          );
#else
    id hook_objc_msgSend(id obj, SEL sel, ...) {
        return nil;
    }
#endif

void lcs_start(char* log_path) {
    lcs_resume_print();
    
    memset(_log_path, 0, 1024);
    memcpy(_log_path, log_path, strlen(log_path));
    
    _log_file = fopen(log_path, "w");
    if (_log_file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    
    fprintf(_log_file, "[\n");
    
    queue_ = dispatch_queue_create("apptrace.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_set_target_queue(queue_, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0));
    begin_ = lcs_getCurrentTime();
    _main_ptread = pthread_self();
    pthread_threadid_np(_main_ptread, &main_thread_id);
    pthread_key_create(&_thread_lr_stack_key, NULL);
    
//    rebind_symbols((struct rebinding[1]){{"objc_msgSend", hook_objc_msgSend, (void *)&orig_objc_msgSend}}, 1);
    int index = 0;
#if TARGET_IPHONE_SIMULATOR
    index = 1;
#endif
    const struct mach_header *header = _dyld_get_image_header(index);
    intptr_t slide = _dyld_get_image_vmaddr_slide(index);
    rebind_symbols_image(header, slide, (struct rebinding[1]){{"objc_msgSend", hook_objc_msgSend, (void *)&orig_objc_msgSend}}, 1);
}
