#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>


class Timer {
public:
    // 构造函数开始计时
    Timer(const std::string& name = "Timer", int level=0);

    void end();
    // 析构函数结束计时并输出结果
    ~Timer();

    // 禁止拷贝和赋值
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // 可以移动构造
    Timer(Timer&&) = default;
    Timer& operator=(Timer&&) = default;

private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    int m_level;
};


class TimerOutput
{
public:
    static TimerOutput& Instance()
    {
        static TimerOutput instance;
        return instance;
    }
    void add(std::string name, long long duration);
    void print();

    void clear();
    void start(std::string name = "timer") 
    { 
        m_timer = std::make_unique<Timer>(name);
        m_start_record = true;
    }
    void end() 
    { 
        if (m_timer) 
        {
        m_timer.reset(); 
        print(); 
        }
        m_start_record = false;
    }
private:
    TimerOutput() = default;
    ~TimerOutput() = default;

    TimerOutput(const TimerOutput&) = delete;
    TimerOutput& operator=(const TimerOutput&) = delete;

    struct data
    {
        std::string name = "";
        long long duration = 0;
        uint64_t times = 0;
    };
    std::unique_ptr<Timer> m_timer;
    std::vector<data> m_map;
    bool m_start_record = false;
};