/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <array>

#define MAX_DIFF_BETWEEN_MEASUREMENTS 100
#define VECTOR_BUFFER_LENGTH 32
#define SMOOTH_FILTER_LENGTH 4

#define ACC_RAMGE 8192 /* 4*8192 ---> ±4g Range */

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
}coordinatesType;

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
}offsetType;

class Measure
{
private:
    std::array<coordinatesType, VECTOR_BUFFER_LENGTH> m_vectorBuffer;
    std::array<coordinatesType, SMOOTH_FILTER_LENGTH> m_vectorPrevoius;
    coordinatesType m_average;
    offsetType m_offset;
    bool m_offserReady;
    int m_index;

    struct timespec m_time;
    double m_timeSinceBoot;

public:
    Measure();
    ~Measure();

    bool appendBuffer(int16_t x, int16_t y, int16_t z);
    void setFpgaAverageBuffer(int16_t xSample, int16_t ySample, int16_t zSample);
    void averageBuffer();
    void calibrationOfset();
    offsetType* getCalibrationOfset();
    void clearBuffer();
};
