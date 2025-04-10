/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "list.h"

MeasList::MeasList()
: m_head(nullptr)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate MeasList" << std::endl;
}

MeasList::~MeasList()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy MeasList" << std::endl;
    freeList();
}

void MeasList::append(int x, int y, int z)
{
    iceList* newNode = new iceList;
    newNode->point = {x, y, z};
    newNode->nextList = nullptr;

    if (!m_head)
    {
        m_head = newNode;
    }
    else
    {
        iceList* current = m_head;
        while (current->nextList != nullptr)
        {
            current = current->nextList;
        }
        current->nextList = newNode;
    }
}

void MeasList::printList() const
{
    iceList* current = m_head;
    int i = 1;
    while (current != nullptr)
    {
        std::cout << "[CTRL] [LST] P[" << i << "] ["
        << current->point.x << ", "
        << current->point.y << ", "
        << current->point.z << "]" << std::endl;

        i++;
        current = current->nextList;
    }
}

void MeasList::freeList()
{
    iceList* current = m_head;
    while (current != nullptr)
    {
        iceList* temp = current;
        current = current->nextList;
        delete temp;
    }
    m_head = nullptr;
}

int main()
{
    MeasList list;

    list.append(1, 2, 3);
    list.append(4, 5, 6);
    list.append(7, 8, 9);

    list.printList();

    return 0;
}
