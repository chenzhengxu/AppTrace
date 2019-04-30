//
//  AppTrace.h
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, TraceFileCachePattern) {
    TraceFileCachePatternMultiple = 0,      // 保存历史追踪文件
    TraceFileCachePatternSingle,            // 每次启动删除历史文件
};

@interface AppTrace : NSObject

/**
 设置追踪文件缓存方案
 在+ (void)startTrace;方法之前调用
 */
+ (void)setTraceFileCachePattern:(TraceFileCachePattern)pattern;

// 开始检测
+ (void)startTrace;

// 暂停检测，不再写入检测数据
+ (void)endTrace;

// 设置打印最小方法执行时间 单位毫秒 默认为1毫秒
+ (void)setMinDuration:(int)minDuration;

@end
