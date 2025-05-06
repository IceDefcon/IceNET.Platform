/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "Measure.h"

Measure::Measure() :
m_index(0),
m_seconds_since_boot(0)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Measure" << std::endl;
}

Measure::~Measure()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Measure" << std::endl;
}

bool Measure::appendBuffer(int16_t x, int16_t y, int16_t z)
{
    bool ret = false;

    if (m_index >= VECTOR_BUFFER_LENGTH)
    {
        std::cout << "[ERNO] [MEAS] Measure index above buffer space -> " << m_index << std::endl;
        m_index = 0;
        return true;
    }

    for (int i = SMOOTH_FILTER_LENGTH - 1; i > 0; --i)
    {
        m_vectorPrevoius[i] = m_vectorPrevoius[i - 1];
    }

    m_vectorPrevoius[0] = {x, y, z};

    int16_t min_x = x, max_x = x;
    int16_t min_y = y, max_y = y;
    int16_t min_z = z, max_z = z;

    for (int i = 1; i < SMOOTH_FILTER_LENGTH; ++i)
    {
        const auto& vec = m_vectorPrevoius[i];
        if (vec.x < min_x) min_x = vec.x;
        if (vec.x > max_x) max_x = vec.x;

        if (vec.y < min_y) min_y = vec.y;
        if (vec.y > max_y) max_y = vec.y;

        if (vec.z < min_z) min_z = vec.z;
        if (vec.z > max_z) max_z = vec.z;
    }

    bool check_x = (max_x - min_x <= MAX_DIFF_BETWEEN_MEASUREMENTS);
    bool check_y = (max_y - min_y <= MAX_DIFF_BETWEEN_MEASUREMENTS);
    bool check_z = (max_z - min_z <= MAX_DIFF_BETWEEN_MEASUREMENTS);

    if (check_x && check_y && check_z)
    {
        m_vectorBuffer[m_index] = {x, y, z};

        clock_gettime(CLOCK_MONOTONIC, &m_ts);
        m_seconds_since_boot = m_ts.tv_sec + m_ts.tv_nsec / 1e9;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "[INFO] [MEAS] [" << m_seconds_since_boot << "] "
        << std::dec << "[" << m_index << "] Vector Acceleration ["
        << x << "," << y << "," << z << "]" << std::endl;

        m_index++;
        ret = (m_index == VECTOR_BUFFER_LENGTH);

        if (ret)
        {
            m_index = 0;
        }
    }
#if 0
    else
    {
        std::cout << "[INFO] [MEAS] Rejected [" << m_index << "] Vector Acceleration ["
                  << x << "," << y << "," << z << "]" << std::endl;
    }
#endif

    return ret;
}

void Measure::averageBuffer()
{
    int64_t sum_x = 0, sum_y = 0, sum_z = 0;

    for (int i = 0; i < VECTOR_BUFFER_LENGTH; i++)
    {
        sum_x += m_vectorBuffer[i].x;
        sum_y += m_vectorBuffer[i].y;
        sum_z += m_vectorBuffer[i].z;
    }

    m_average.x = static_cast<int16_t>(sum_x / VECTOR_BUFFER_LENGTH);
    m_average.y = static_cast<int16_t>(sum_y / VECTOR_BUFFER_LENGTH);
    m_average.z = static_cast<int16_t>(sum_z / VECTOR_BUFFER_LENGTH) + 8192;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "[INFO] [MEAS] [" << m_seconds_since_boot
    << "] " << std::dec << "Average Acceleration ["
    << m_average.x << "," << m_average.y << "," << m_average.z << "]" << std::endl;
}

void Measure::clearBuffer()
{
    for (int i = 0; i < VECTOR_BUFFER_LENGTH; i++)
    {
        m_vectorBuffer[i] = {0, 0, 0};
    }

    m_average = {0, 0, 0};
    m_index = 0;
}
