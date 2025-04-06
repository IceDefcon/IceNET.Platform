#include <iostream>

struct Point3D
{
    float x, y, z;
};

class IceList
{
    public:
        Point3D point;
        IceList* next;

        IceList(float x, float y, float z)
            : point{x, y, z}, next(nullptr) {}
};

class IceLinkedList
{
private:
    IceList* head;

public:
    IceLinkedList() : head(nullptr) {}

    ~IceLinkedList()
    {
        freeList();
    }

    void append(float x, float y, float z)
    {
        IceList* newNode = new IceList(x, y, z);

        if (!head)
        {
            head = newNode;
        }
        else
        {
            IceList* current = head;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = newNode;
        }
    }

    void print() const
    {
        IceList* current = head;
        int i = 1;
        while (current != nullptr)
        {
            std::cout << "Point " << i++ << ": x = " << current->point.x
                      << ", y = " << current->point.y
                      << ", z = " << current->point.z << std::endl;
            current = current->next;
        }
    }

private:
    void freeList()
    {
        IceList* current = head;
        while (current != nullptr)
        {
            IceList* temp = current;
            current = current->next;
            delete temp;
        }
        head = nullptr;
    }
};

int main()
{
    IceLinkedList list;

    list.append(1.0, 2.0, 3.0);
    list.append(4.0, 5.0, 6.0);
    list.append(7.0, 8.0, 9.0);
    list.append(10.0, 11.0, 12.0);

    list.print();

    return 0;
}
