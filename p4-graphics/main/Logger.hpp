/**
 * 
 */

#pragma once

#include <iostream>
#include <utility>

#ifdef ESP_PLATFORM == 1
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
        void Log (const char * TAG, uint_8 level, const char * fmt, Args... args)
        {
            if (level > logLevel)
                return;

            #ifdef ESP_PLATFORM == 1
            switch (level)
            {
                case 0:
                case 1:
                    ESP_LOGE (TAG, fmt, std::forward<Args>(args)...);
                    break;
                
                case 2:
                    ESP_LOGW (TAG, fmt, std::forward<Args>(args)...);
                    break;
                
                case 3:
                default:
                    ESP_LOGI (TAG, fmt, std::forward<Args>(args)...);
                    break;
            }
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
            #ifdef ESP_PLATFORM == 1
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