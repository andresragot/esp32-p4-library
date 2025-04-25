/**
 * 
 */

#pragma once

#include <iostream>
#include <utility>

#if ESP_PLATFORM == 1
#include "esp_log.h"
#endif

namespace Ragot
{
    class Logger
    {
    public:
        
        static Logger &instance()
        {
            static Logger instance;
            return instance;
        }


        template < typename... Args >
        void Log (const char * TAG, uint8_t level, const char * fmt, Args... args)
        {
            if (level > logLevel)
                return;

            #if ESP_PLATFORM == 1
            esp_log_write ((esp_log_level_t)level, TAG, fmt, std::forward<Args>(args)...);
            #else
            std::cout << "[" << TAG << "]: ";
            ( (std::cout << std::forward<Args>(args)), ... );
            std::cout << std::endl;
            #endif
        }

    private:
        Logger () = default;
       ~Logger () = default;

        Logger (const Logger & ) = delete;
        Logger (const Logger &&) = delete;

        Logger & operator = (const Logger & ) = delete;
        Logger & operator = (const Logger &&) = delete;

    private:
        uint8_t logLevel = 0; // 0 = INFO, 1 = WARNING, 2 = ERROR
        
    public:
        void setLogLevel (uint8_t level)
        {
            logLevel = level;
            #if ESP_PLATFORM == 1
            esp_log_level_set("*", (esp_log_level_t)level);
            #endif
        }

        uint8_t getLogLevel () const
        {
            return logLevel;
        }
        
    };

    extern Logger & logger;
}