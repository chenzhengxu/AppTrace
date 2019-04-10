//
//  AppTrace.m
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import "AppTrace.h"
#include "locusImpl.h"
#import <objc/runtime.h>

static id filerBlockHolder = nil;

@implementation AppTrace

+ (void)startTrace {
    NSString * rootdir = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
    NSString *work_dir = [rootdir stringByAppendingPathComponent:@"apptrace"];
    NSFileManager *fm = [NSFileManager defaultManager];
    if ([fm fileExistsAtPath:work_dir]) {
        [fm removeItemAtPath:work_dir error:nil];
    }
    [fm createDirectoryAtPath:work_dir withIntermediateDirectories:YES attributes:nil error:nil];
    NSString * log_name = @"trace.json";
    char *log_path = (char *)[[work_dir stringByAppendingPathComponent:log_name] UTF8String];
    
    NSArray* classNames = [self getClassNamesFromBundle];
    id filter = ^int(char *className, char *selName) {
        NSString* sClass = [NSString stringWithUTF8String:className];
        return [classNames containsObject:sClass];
    };
    filerBlockHolder = filter;
    lcs_start(filerBlockHolder, log_path);
}

+ (NSArray *)getClassNamesFromBundle {
    NSMutableArray* classNames = [NSMutableArray array];
    unsigned int count = 0;
    const char** classes = objc_copyClassNamesForImage([[[NSBundle mainBundle] executablePath] UTF8String], &count);
    for(unsigned int i=0;i<count;i++){
        NSString* className = [NSString stringWithUTF8String:classes[i]];
        [classNames addObject:className];
    }
    return classNames;
}

@end
