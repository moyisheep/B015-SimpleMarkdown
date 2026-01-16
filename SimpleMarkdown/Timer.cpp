#include "Timer.h"
#include <wx/wx.h>
void debug_print( std::string info)
{
    wxLogMessage(info);
}

Timer::Timer(const std::string& name, int level) :
     m_start(std::chrono::high_resolution_clock::now())
{
    m_name = "";


    if (level)
    {
        m_name += "[" + std::to_string(level) + "]";
        
        for (int i = 0; i < level * 4; i++)
        {
            m_name += " ";
        }

    }
    else { m_name += "[*]"; }
    m_name += name;

}
Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start);
    TimerOutput::Instance().add(m_name, duration.count());
   

}

void TimerOutput::add(std::string name, long long duration)
{
    if(m_start_record)
    {
        for (auto& m : m_map)
        {
            if (m.name == name)
            {
                m.duration += duration;
                m.times += 1;
                return;
            }

        }

        m_map.push_back(data{ name, duration, 1 });
    }

}

void TimerOutput::print()
{
    //std::sort(m_map.begin(), m_map.end(), [](const data& a, const data& b)
    //    {
    //        return a.duration < b.duration;
    //    });
    auto map = m_map;
    clear();
    for (auto& m : map)
    {
        // 格式化为 9 位小数（纳秒精度）
        std::ostringstream time_oss;
        time_oss << std::fixed << std::setprecision(9) << (m.duration / 1000000000.0);
        std::string time_str = time_oss.str();

        // 在 . 后每 3 位插入一个空格
        size_t dot_pos = time_str.find('.');
        if (dot_pos != std::string::npos) {
            for (size_t i = dot_pos + 4; i < time_str.length(); i += 4) {
                time_str.insert(i, " ");
            }
        }

        // 构建完整输出字符串
        std::ostringstream oss;
        oss << std::left << std::setw(30) << m.name << ": "
            << std::right << std::setw(12) << time_str << " s, "
            << std::right << std::setw(6) << m.times << " Hz\n";

        debug_print(oss.str().c_str());
    }


}
void TimerOutput::clear()
{
    m_map = {};
}
