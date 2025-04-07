/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <iostream>

typedef struct
{
    /* Signed values from Sensors */
    int x;
    int y;
    int z;
} measureType;

typedef struct Node
{
    measureType point;
    struct Node* nextList;
} iceList;

class MeasList
{
    private:
        iceList* m_head;

    public:
        MeasList();
        ~MeasList();

        void append(int x, int y, int z);
        void printList() const;
        void freeList();
};
