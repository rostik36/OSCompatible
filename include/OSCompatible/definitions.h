/**
* @file definitions.h
* @author Rostik
* @brief 
* @version 0.1
* @date 2024-11-10
* 
* @copyright Copyright (c) 2024
* 
*/
#ifndef _DEFINITIONS__
#define _DEFINITIONS__
// Or: #pragma once

// 1. Forward declarations (wherever possible)

// 2. Standard library headers

// 3. Third-party library headers

// 4. Other project headers (only if necessary)



namespace OSCompatible
{
    enum BufferSize : size_t
    {
        Default = 1024,
        Medium = 4096,
        Large = 8192,
        ExtraLarge = 32768
    };

    enum Time : int
    {
        Infinite = -1,
        Zero = 0,
        Milliseconds_1 = 1,
        Milliseconds_2 = 2,
        Milliseconds_3 = 3,
        Milliseconds_5 = 5,
        Milliseconds_10 = 10,
        Milliseconds_20 = 20,
        Milliseconds_25 = 25,
        Milliseconds_50 = 50,
        Milliseconds_100 = 100,
        Milliseconds_250 = 250,
        Milliseconds_500 = 500,
        Milliseconds_750 = 750,
        Seconds_1 = 1000,
        Seconds_2 = 2000,
        Seconds_3 = 3000,
        Seconds_5 = 5000,
        Seconds_10 = 10000,
        Seconds_20 = 20000,
        Seconds_30 = 30000,
        Seconds_40 = 40000,
        Seconds_50 = 50000,
        Minutes_1 = 1*60000,
        Minutes_2 = 2*60000,
        Minutes_3 = 3*60000,
        Minutes_5 = 5*60000,
        Minutes_10 = 10*60000,
        Minutes_20 = 20*60000,
        Minutes_30 = 30*60000,
        Minutes_40 = 40*60000,
        Minutes_50 = 50*60000,
        Hours_1 = 60*60000,
    };
}


#endif //_DEFINITIONS__