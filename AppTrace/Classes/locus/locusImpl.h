//
//  hookObjcSend.h
//  fishhookdemo
//
//  Created by FanFamily on 2018/7/21.
//  Copyright © 2018年 Family Fan. All rights reserved.
//

#ifndef hookObjcSend_h
#define hookObjcSend_h

static int method_min_duration = 1 * 1000; // 1 milliseconds

void lcs_start(char* log_path);
void lcs_stop_print(void);
void lcs_resume_print(void);

#endif /* hookObjcSend_h */
