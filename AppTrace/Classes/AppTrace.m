//
//  AppTrace.m
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import "AppTrace.h"
#include "AppTraceImpl.h"

static TraceFileCachePattern kFileCachePattern = 0;
static int kAppTraceStarted = 0;

@implementation AppTrace

+ (void)setTraceFileCachePattern:(TraceFileCachePattern)pattern {
    kFileCachePattern = pattern;
}

+ (void)startTrace {
    if (kAppTraceStarted != 0) {
        lcs_resume_print();
        return;
    }
    kAppTraceStarted = 1;
    NSString *rootdir = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES)[0];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *work_dir = [rootdir stringByAppendingPathComponent:@"apptrace"];
    if ([fm fileExistsAtPath:work_dir] && kFileCachePattern == TraceFileCachePatternSingle) {
        [fm removeItemAtPath:work_dir error:nil];
    }
    if (![fm fileExistsAtPath:work_dir]) {
        [fm createDirectoryAtPath:work_dir withIntermediateDirectories:YES attributes:nil error:nil];
    }
    NSTimeInterval timeInterval = [[NSDate date] timeIntervalSince1970];
    NSString *log_name = [NSString stringWithFormat:@"trace_%.f.json", timeInterval];
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
