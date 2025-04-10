#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    float x;
    float y;
    float z;
} Point3D;

typedef struct Node
{
    Point3D point;
    struct Node* nextList;
} iceList;

iceList* createList(float x, float y, float z)
{
    iceList* newList = (iceList*)malloc(sizeof(iceList));

    if (newList == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newList->point.x = x;
    newList->point.y = y;
    newList->point.z = z;
    newList->nextList = NULL;
    return newList;
}

void appendList(iceList** head, float x, float y, float z)
{
    iceList* newList = createList(x, y, z);

    if (*head == NULL)
    {
        *head = newList;
    }
    else
    {
        iceList* current = *head;
        while (current->nextList != NULL)
        {
            current = current->nextList;
        }
        current->nextList = newList;
    }
}

void printList(iceList* head)
{
    int i = 1;
    while (head != NULL)
    {
        printf("Point %d: x = %.2f, y = %.2f, z = %.2f\n", i++, head->point.x, head->point.y, head->point.z);
        head = head->nextList;
    }
}

void freeList(iceList* head)
{
    iceList* temp;
    while (head != NULL)
    {
        temp = head;
        head = head->nextList;
        free(temp);
    }
}

int main()
{
    iceList* head = NULL;

    appendList(&head, 1.0, 2.0, 3.0);
    appendList(&head, 4.0, 5.0, 6.0);
    appendList(&head, 7.0, 8.0, 9.0);
    appendList(&head, 10.0, 11.0, 12.0);

    printList(head);
    freeList(head);

    return 0;
}
