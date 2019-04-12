//
//  AppTrace.h
//  AppTrace
//
//  Created by chenzhengxu on 2019/4/10.
//

#import <Foundation/Foundation.h>

@interface AppTrace : NSObject

// 开始检测
+ (void)startTrace;

// 暂停检测，不再写入检测数据
+ (void)endTrace;

// 设置打印最小方法执行时间 单位毫秒 默认为1毫秒
+ (void)setMinDuration:(int)minDuration;

@end
