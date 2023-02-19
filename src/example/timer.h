#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

class Timer
{
public:
    Timer(bool beginStarted) {
        if (beginStarted) {
            start();
        }
    }

    void start() {
        if (!m_bRunning) {
            m_StartTime = std::chrono::system_clock::now();
            m_bRunning = true;
        }
    }
    
    void stop() {
        if (m_bRunning) {
            m_EndTime = std::chrono::system_clock::now();
            m_bRunning = false;
        }
    }
    
    double milliseconds() {
        std::chrono::time_point<std::chrono::system_clock> endTime;
        
        if (m_bRunning)
            endTime = std::chrono::system_clock::now();
        else
            endTime = m_EndTime;
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }
    
    double seconds() {
        return milliseconds() / 1000.0;
    }

private:
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool m_bRunning = false;
};
