#include <stdio.h>
#include <stdlib.h>

// Define the list_head structure
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

// Initialize a list head
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

// Embedded structure
struct my_data {
    int value;                // Example data
    struct list_head list;    // Embedded list node
};

// Initialize a list
void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

// Add a new node to the list
void list_add(struct list_head *new, struct list_head *head) {
    struct list_head *next = head->next;
    head->next = new;
    new->prev = head;
    new->next = next;
    next->prev = new;
}

// Iterate over the list
#define list_for_each_entry(pos, head, member)                \
    for (pos = (typeof(pos))((head)->next);                   \
         &pos->member != (head);                              \
         pos = (typeof(pos))(pos->member.next))

int main() {
    LIST_HEAD(my_list);  // Create and initialize a list head

    // Add elements to the list
    for (int i = 0; i < 5; i++) {
        struct my_data *item = malloc(sizeof(*item));
        item->value = i;  // Assign data
        list_add(&item->list, &my_list);
        printf("Added: %d\n", i);
    }

    // Print all elements in the list
    struct my_data *entry;
    printf("List contents:\n");
    list_for_each_entry(entry, &my_list, list) {
        printf("Value: %d\n", entry->value);
    }

    return 0;
}
