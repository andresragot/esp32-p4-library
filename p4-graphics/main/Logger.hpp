/**
 * @file Logger.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief This file implements the Logger class, which provides a singleton logger for the Ragot engine.
 * @details The Logger class allows logging messages with different severity levels (INFO, WARNING, ERROR).
 * It supports formatted logging using printf-style format strings and can be used across different platforms.
 * The logger can be configured to set the log level, and it uses the ESP-IDF logging system on ESP platforms.
 * @version 1.0
 * @date 2025-06-01
 * 
 * @copyright Copyright (c) 2025
 * MIT License
 * 
 * Copyright (c) 2025 Andrés Ragot 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#pragma once

#include <iostream>
#include <utility>

#if ESP_PLATFORM == 1
#include "esp_log.h"
#endif

namespace Ragot
{   
    /**
     * @class Logger
     * @brief Singleton logger class for the Ragot engine.
     * 
     * This class provides a singleton logger that allows logging messages with different severity levels (INFO, WARNING, ERROR).
     * It supports formatted logging using printf-style format strings and can be used across different platforms.
     */
    class Logger
    {
    public:
        
        /**
         * @brief Gets the singleton instance of the Logger class.
         * 
         * This method returns a reference to the singleton Logger instance.
         * 
         * @return Logger& Reference to the singleton Logger instance.
         */
        static Logger &instance()
        {
            static Logger instance;
            return instance;
        }

        /**
         * @brief Logs a message with the specified tag and level.
         * 
         * This method logs a message with the given tag and severity level. The message can be formatted using printf-style format strings.
         * 
         * @param TAG The tag for the log message.
         * @param level The severity level of the log message (0 = INFO, 1 = WARNING, 2 = ERROR).
         * @param fmt The format string for the log message.
         * @param args The arguments to format the log message.
         */
        template < typename... Args >
        void Log (const char * TAG, uint8_t level, const char * fmt, Args... args)
        {
            if (level > logLevel)
                return;

            #if ESP_PLATFORM == 1
            esp_log_write ((esp_log_level_t)level, TAG, fmt, std::forward<Args>(args)...);
            #else           
            // 1) Calculamos el tamaño del buffer necesario
            int needed = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...) + 1;
            std::vector<char> buffer(needed);

            // 2) Rellenamos el buffer con el texto formateado
            std::snprintf(buffer.data(), buffer.size(), fmt, std::forward<Args>(args)...);

            // 3) Lo imprimimos
            std::cout << "[" << TAG << "]: " << buffer.data() << std::endl;
            #endif
        }

    private:

        /**
         * @brief Default constructor for the Logger class.
         * 
         * This constructor is private to enforce the singleton pattern.
         */
        Logger () = default;

        /**
         * @brief Default destructor for the Logger class.
         * 
         * This destructor is defaulted and does not perform any special cleanup.
         */
       ~Logger () = default;

       /**
        * @brief Construct a new Logger object (Deleted).
        */
        Logger (const Logger & ) = delete;

        /**
         * @brief Construct a new Logger object (Deleted).
         * 
         * This constructor is deleted to prevent moving the Logger instance.
         */
        Logger (const Logger &&) = delete;

        /**
         * @brief Assignment operator for the Logger class (Deleted).
         * 
         * This operator is deleted to prevent assignment of Logger instances.
         * 
         * @return Logger& Reference to the current object.
         */
        Logger & operator = (const Logger & ) = delete;

        /**
         * @brief Assignment operator for the Logger class (Deleted).
         * 
         * This operator is deleted to prevent moving Logger instances.
         * 
         * @return Logger& Reference to the current object.
         */
        Logger & operator = (const Logger &&) = delete;

    private:

        /**
         * @brief Current log level for the logger.
         * 
         * This variable stores the current log level, which determines the severity of messages that will be logged.
         * 0 = INFO, 1 = WARNING, 2 = ERROR.
         */
        uint8_t logLevel = 0; // 0 = INFO, 1 = WARNING, 2 = ERROR
        
    public:

        /**
         * @brief Sets the log level for the logger.
         * 
         * This method sets the log level for the logger, which determines the severity of messages that will be logged.
         * It also configures the ESP-IDF logging system if running on an ESP platform.
         * 
         * @param level The new log level to set (0 = INFO, 1 = WARNING, 2 = ERROR).
         */
        void setLogLevel (uint8_t level)
        {
            logLevel = level;
            #if ESP_PLATFORM == 1
            esp_log_level_set("*", (esp_log_level_t)level);
            #endif
        }

        /**
         * @brief Gets the current log level of the logger.
         * 
         * This method returns the current log level, which determines the severity of messages that will be logged.
         * 
         * @return uint8_t The current log level (0 = INFO, 1 = WARNING, 2 = ERROR).
         */
        uint8_t getLogLevel () const
        {
            return logLevel;
        }
        
    };

    extern Logger & logger;
}
