/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <iostream>
#include <iomanip>
#include <array>

#define MAX_MEAS_SIZE 32
#define MAX_FILTER_SIZE 4

struct coordinatesType
{
    int16_t x;
    int16_t y;
    int16_t z;
};

class Measure
{
private:
    std::array<coordinatesType, MAX_MEAS_SIZE> m_vectorBuffer;
    std::array<coordinatesType, MAX_FILTER_SIZE> m_vectorPrevoius;
    coordinatesType m_average;
    int m_index;

    struct timespec m_ts;
    double m_seconds_since_boot;

public:
    Measure();
    ~Measure();

    bool appendBuffer(int16_t x, int16_t y, int16_t z);
    void averageBuffer();
    void clearBuffer();
    coordinatesType getAverage();
};
