//
//  AppTrace.h
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import <Foundation/Foundation.h>

@interface AppTrace : NSObject

+ (void)startTrace;

// 设置打印最小方法执行时间 单位毫秒 默认为1毫秒
+ (void)setMinDuration:(int)minDuration;

@end
