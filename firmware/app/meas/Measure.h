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

#define MAX_DIFF_BETWEEN_MEASUREMENTS 30
#define VECTOR_BUFFER_LENGTH 32
#define SMOOTH_FILTER_LENGTH 8

struct coordinatesType
{
    int16_t x;
    int16_t y;
    int16_t z;
};

class Measure
{
private:
    std::array<coordinatesType, VECTOR_BUFFER_LENGTH> m_vectorBuffer;
    std::array<coordinatesType, SMOOTH_FILTER_LENGTH> m_vectorPrevoius;
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
