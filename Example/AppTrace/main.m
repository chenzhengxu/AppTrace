//
//  main.m
//  AppTrace
//
//  Created by chenzhengxu2 on 04/08/2019.
//  Copyright (c) 2019 chenzhengxu2. All rights reserved.
//

@import UIKit;
#import "NEPAppDelegate.h"
#import "AppTrace.h"

int main(int argc, char * argv[])
{
    [AppTrace setMinDuration:3];
    [AppTrace startTrace];
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([NEPAppDelegate class]));
    }
}
