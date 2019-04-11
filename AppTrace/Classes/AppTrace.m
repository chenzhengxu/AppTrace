//
//  AppTrace.m
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import "AppTrace.h"
#include "locusImpl.h"

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
    
    lcs_start(log_path);
}

+ (void)setMinDuration:(int)minDuration {
    method_min_duration = minDuration * 1000;
}

@end
