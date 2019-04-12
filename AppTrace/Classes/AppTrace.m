//
//  AppTrace.m
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import "AppTrace.h"
#include "AppTraceImpl.h"

static int kAppTraceStarted = 0;

@implementation AppTrace

+ (void)startTrace {
    if (kAppTraceStarted != 0) {
        lcs_resume_print();
        return;
    }
    kAppTraceStarted = 1;
    NSString * rootdir = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
    NSString *work_dir = [rootdir stringByAppendingPathComponent:@"apptrace"];
    NSFileManager *fm = [NSFileManager defaultManager];
    if ([fm fileExistsAtPath:work_dir]) {
        [fm removeItemAtPath:work_dir error:nil];
    }
    [fm createDirectoryAtPath:work_dir withIntermediateDirectories:YES attributes:nil error:nil];
    NSString * log_name = @"trace.json";
    char *log_path = (char *)[[work_dir stringByAppendingPathComponent:log_name] UTF8String];
    
    lcs_start(log_path);
}

+ (void)endTrace {
    lcs_stop_print();
}

+ (void)setMinDuration:(int)minDuration {
    method_min_duration = minDuration * 1000;
}

@end
