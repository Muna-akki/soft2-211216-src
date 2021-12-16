#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int maxlen = 1000;

typedef struct node Node;

struct node 
{
    char *str;
    Node *next;
};

typedef struct list{
    Node* begin;
} List;

List* make_list(Node* begin){
    List* l = (List*)malloc(sizeof(List));
    Node* b = begin;
    *l = (List){.begin = b};
    return l;
}

List *push_front(List* list, const char *str)
{
    
    Node *p = (Node *)malloc(sizeof(Node));
    char *s = (char *)malloc(strlen(str) + 1);
    strcpy(s, str);
    
    *p = (Node){.str = s , .next = list->begin};
    (list->begin) = p;
    return list; 
}


List *pop_front(List* list)
{
    assert(list->begin != NULL);
    Node *p = list->begin->next;
    
    free(list->begin->str);
    free(list->begin);
    list->begin = p;
    return list;
}

List *push_back(List* list, const char *str)
{
    if (list->begin == NULL) { 
	    return push_front(list, str);
    }
    
    Node *p = list->begin;
    while (p->next != NULL) {
	    p = p->next;
    }
    
    Node *q = (Node *)malloc(sizeof(Node));
    char *s = (char *)malloc(strlen(str) + 1);
    strcpy(s, str);
    
    *q = (Node){.str = s, .next = NULL};
    p->next = q;
    
    return list;
}

// Let's try: pop_back の実装
List *pop_back(List* list)
{
    // write an implementation.
    assert(list->begin != NULL);
    Node* p = list->begin;
    Node* pp = NULL;
    while(p->next !=NULL){
        pp = p;
        p = p->next;
    }
    free(p->str);
    free(p);
    if(pp!=NULL){
        pp->next = NULL;
    }
    return list;
}


List *remove_all(List *list)
{
    while ((list->begin = pop_front(list)->begin)) 
	; // Repeat pop_front() until the list becomes empty
    return list;
}

int main()
{
    Node *begin = NULL; 
    List* list = make_list(begin);
    
    char buf[maxlen];
    while (fgets(buf, maxlen, stdin)) {
	    list = push_front(list, buf);
	    //begin = push_back(begin, buf); // Try this instead of push_front()
    }
    
    //begin = pop_front(begin);  // What will happen if you do this?
    //begin = pop_back(begin);   // What will happen if you do this?
    
    //begin = remove_all(begin); // What will happen if you do this?
    
    for (const Node *p = list->begin; p != NULL; p = p->next) {
	    printf("%s", p->str);
    }
    
    return EXIT_SUCCESS;
}
