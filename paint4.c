#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

typedef struct{
    int width;
    int height;
    char** canvas;
    char pen;
} Canvas;

typedef struct command{
    char* str;
    size_t bufsize;
    struct command* next;
    struct command* prev;
} Command;

typedef struct {
    Command* begin;
} History;

Canvas* init_canvas(int width, int height, char pen);
void reset_canvas(Canvas* c);
void print_canvas(Canvas* c);
void free_canvas(Canvas* c);

void rewind_screen(unsigned int line);
void clear_command(void);
void clear_screen(void);

// Historyの操作
Command* push_back(History* his, const char* str, size_t bufsize);
Command* pop_back(History* his);

typedef enum res{EXIT, NORMAL, COMMAND, UNKNOWN, ERROR} Result;

int max(const int a, const int b);
void draw_line(Canvas* c, const int x0, const int y0, const int x1, const int y1);
void draw_rect(Canvas* c, const int x0, const int y0, const int w0, const int h0);
void draw_circle(Canvas* c, const int x0, const int y0, const int r0);
Result interpret_command(const char* command, History* his, Canvas* c);
void save_history(const char *filename, History* his);

int main(int argc, char** argv){
    const int bufsize = 1000;
    History his = (History){.begin = NULL};
    //入力のチェック
    int width;
    int height;
    if(argc != 3){
        fprintf(stderr, "usage: %s <width> <hieght>\n",argv[0]);
        return EXIT_FAILURE;
    }else{
        char* e;
        long w = strtol(argv[1],&e,10);
        if(*e != '\0'){
            fprintf(stderr, "%s: irregular character found %s\n",argv[1],e);
            return EXIT_FAILURE;
        }
        long h = strtol(argv[2],&e,10);
        if(*e != '\0'){
            fprintf(stderr, "%s: irregular character found %s\n",argv[2],e);
            return EXIT_FAILURE;
        }
        width = (int) w;
        height = (int) h;
    }
    char pen = '*';

    char buf[bufsize];
    Canvas* c = init_canvas(width, height,pen);

    printf("\n");
    unsigned long count = 0;
    while(1){
        count++;
        print_canvas(c);
        printf("%zu > ",count);
        fgets(buf, bufsize, stdin);
        const Result r = interpret_command(buf, &his,c);
        if(r == EXIT){
            break;
        }
        if(r == NORMAL){
            push_back(&his, buf, bufsize);
        }

        rewind_screen(2);
        clear_command();
        rewind_screen(height+2);
    }
    clear_screen();
    free_canvas(c);

    return 0;
}



Canvas* init_canvas(int width, int height, char pen){
    Canvas* new = (Canvas*)malloc(sizeof(Canvas));
    new->width = width;
    new->height = height;

    new->canvas = (char**)malloc(width*sizeof(char*));

    char* tmp = (char*)malloc(width*height*sizeof(char));
    memset(tmp, ' ', width*height*sizeof(char));

    for(int i=0 ; i<width ; i++){
        new->canvas[i] = tmp+i*height;
    }
    new->pen = pen;
    return new;
}

void reset_canvas(Canvas* c){
    const int width = c->width;
    const int height = c->height;
    memset(c->canvas[0], ' ', width*height*sizeof(char));
}

void print_canvas(Canvas* c){
    const int height = c->height;
    const int width = c->width;
    char** canvas = c->canvas;

    printf("+");
    for(int x=0 ; x<width ; x++){
        printf("-");
    }
    printf("+\n");

    for(int y=0 ; y<height ; y++){
        printf("|");
        for(int x=0 ; x<width ; x++){
            const char c = canvas[x][y];
            putchar(c);
        }
        printf("|\n");
    }

    printf("+");
    for(int x=0 ; x<width ; x++){
        printf("-");
    }
    printf("+\n");
    fflush(stdout);
}

void free_canvas(Canvas* c){
    free(c->canvas[0]);
    free(c->canvas);
    free(c);
}


void rewind_screen(unsigned int line){
    printf("\e[%dA",line);
}

void clear_command(void){
    printf("\e[2K");
}

void clear_screen(void){
    printf("\e[2J");
}


// Historyの操作
Command* push_back(History* his, const char* str, size_t bufsize){
    Command* p = his->begin;
    char* s = (char*)malloc(strlen(str)+1);
    strcpy(s,str);
    if(p==NULL){
        p = (Command*)malloc(sizeof(Command));
        *p = (Command){.bufsize = bufsize, .prev = NULL, .next = his->begin, .str = s};
        his->begin = p;
        return p;
    }
    while(p->next != NULL){
        p = p->next;
    }
    Command* q = (Command*)malloc(sizeof(Command));
    *q = (Command){.str = s, .bufsize = bufsize, .next = NULL, .prev = p};
    p->next = q;
    return q; 
}

Command* pop_back(History* his){
    Command* p = his->begin;
    Command* q = NULL;
    if(p==NULL){
        return NULL;
    }
    while(p->next != NULL){
        q = p;
        p = p->next;
    }

    if(q==NULL){
        his->begin = p->next;
        p->next = NULL;
        free(p->str);
        free(p);
    }
    if(q!=NULL){
        q->next = NULL;
        free(p->str);
        free(p);
    }
    return p;
}


int max(const int a, const int b){
    return (a>b) ? a:b;
}

void draw_line(Canvas* c, const int x0, const int y0, const int x1, const int y1){
    const int width = c->width;
    const int height = c->height;
    char pen = c->pen;

    const int n = max(abs(x1-x0),abs(y1-y0));
    c->canvas[x0][y0] = pen;
    for(int i=1 ; i<=n ; i++){
        const int x = x0 + i*(x1-x0)/n;
        const int y = y0 + i*(y1-y0)/n;
        if(x>=0 && x<width && y>=0 && y<height){
            c->canvas[x][y] = pen;
        }
    }
}

void draw_rect(Canvas* c, const int x0, const int y0, const int w0, const int h0){
    const int width = c->width;
    const int height = c->height;
    char pen = c->pen;

    //縦
    for(int i=0 ; i<h0 ; i++){
        const int x1 = x0;
        const int y1 = y0+i;
        const int x2 = x0+w0-1;
        if(x1>=0 && x1<width && y1>=0 && y1<height){
            c->canvas[x1][y1] = pen;
        }
        if(x2>=0 && x2<width && y1>=0 && y1<height){
            c->canvas[x2][y1] = pen;
        }
    }
    //横
    for(int i=0 ; i<w0 ; i++){
        const int x1 = x0+i;
        const int y1 = y0;
        const int y2 = y0+h0-1;
        if(x1>=0 && x1<width && y1>=0 && y1<height){
            c->canvas[x1][y1] = pen;
        }
        if(x1>=0 && x1<width && y2>=0 && y2<height){
            c->canvas[x1][y2] = pen;
        }
    }
}

void draw_circle(Canvas* c, const int x0, const int y0, const int r0){
    const int width = c->width;
    const int height = c->height;
    char pen = c->pen;
    for(int x=x0-r0+1 ; x<=x0+r0-1 ; x++){
        int miny = 0;
        double minval = width*height;
        for(int y=y0-r0+1 ; y<=y0+r0-1 ; y++){
            double dx = x-x0;
            double dy = y-y0;
            double dis = sqrt(dx*dx+dy*dy);
            if(fabs(dis-r0)<minval){
                miny = y;
                minval = dis-r0;
            }
        }
        if(x>=0 && x<width && miny>=0 && miny<height){
            c->canvas[x][(int)miny] = pen;
        }
        double miny2 = 2*y0-miny;
        if(x>=0 && x<width && miny2>=0 && miny2<height){
            c->canvas[x][(int)miny2] = pen;
        }
        double y1 = (miny>miny2) ? miny:miny2;
        double y2 = (miny>miny2) ? miny2:miny;
        if(x==x0-r0+1 || x==x0+r0-1){
            for(int y = y2 ; y<=y1 ; y++){
                if(x>=0 && x<width && y>=0 && y<height){
                    c->canvas[x][y] = pen;
                }
            }
        }
    }
    
}

Result interpret_command(const char* command, History* his, Canvas* c){
    char* buf = (char*)malloc(strlen(command)+1);
    strcpy(buf,command);
    buf[strlen(buf)-1] = 0;
    const char* s = strtok(buf, " ");
    
    if(strcmp(s,"line") == 0){
        int p[4] = {0};
        char* b[4];
        for(int i=0 ; i<4 ; i++){
            b[i] = strtok(NULL," ");
            if(b[i] == NULL){
                clear_command();
                printf("the number of point is not enough.\n");
                return ERROR;
            }
        }
        for(int i=0 ; i<4 ; i++){
            char* e;
            long v = strtol(b[i],&e,10);
            if(*e != '\0'){
                clear_command();
                printf("Non-int value is included.\n");
                return ERROR;
            }
            p[i] = (int)v;
        }
        draw_line(c,p[0],p[1],p[2],p[3]);
        clear_command();
        printf("1 line drawn\n");
        return NORMAL;
    }

    if(strcmp(s, "rect") == 0){
        int p[4] = {0};
        char* b[4];
        for(int i=0 ; i<4 ; i++){
            b[i] = strtok(NULL," ");
            if(b[i] == NULL){
                clear_command();
                printf("the number of point is not enough.\n");
                return ERROR;
            }
        }
        for(int i=0 ; i<4 ; i++){
            char* e;
            long v = strtol(b[i],&e,10);
            if(*e != '\0'){
                clear_command();
                printf("Non-int value is included.\n");
                return ERROR;
            }
            p[i] = (int)v;
        }
        draw_rect(c,p[0],p[1],p[2],p[3]);
        clear_command();
        printf("1 rectangle drawn\n");
        return NORMAL;
    }

    if(strcmp(s, "circle") == 0){
        int p[3] = {0};
        char* b[3];
        for(int i=0 ; i<3 ; i++){
            b[i] = strtok(NULL," ");
            if(b[i] == NULL){
                clear_command();
                printf("the number of point is not enough.\n");
                return ERROR;
            }
        }
        for(int i=0 ; i<3 ; i++){
            char* e;
            long v = strtol(b[i],&e,10);
            if(*e != '\0'){
                clear_command();
                printf("Non-int value is included.\n");
                return ERROR;
            }
            p[i] = (int)v;
        }
        draw_circle(c,p[0],p[1],p[2]);
        clear_command();
        printf("1 circle drawn\n");
        return NORMAL;
    }

    if(strcmp(s, "chpen") == 0){
        s = strtok(NULL, " ");
        if(strlen(s)!=1){
            clear_command();
            fprintf(stderr, "error: %s is not \"one\" character.\n",s);
        }
        char past = c->pen;
        c->pen = s[0];
        clear_command();
        printf("pen changed: %c -> %c\n",past, c->pen);
        return NORMAL;
    }

    if(strcmp(s, "save") == 0){
        s = strtok(NULL, " ");
        save_history(s, his);
        printf("saved as \"%s\"\n", (s==NULL) ? "history.txt":s);
        return COMMAND;
    }

    if(strcmp(s,"load") == 0){
        s = strtok(NULL, " ");
        const char* default_history_file = "history.txt";
        if(s == NULL){
            s = default_history_file;
        }

        FILE* fp;
        if((fp = fopen(s, "r")) == NULL){
            clear_command();
            fprintf(stderr, "error: cannot open %s.\n", s);
            return ERROR;
        }
        //clear_command();
        const int bufsize = 1000;
        unsigned long count = 0;
        while(fgets(buf, bufsize, fp) != NULL){
            const Result r = interpret_command(buf, his,c);
            if(r == EXIT){
                break;
            }
            if(r == NORMAL){
                push_back(his, buf, bufsize);
            }
            rewind_screen(1);
        }
        clear_command();
        printf("%s is loaded.\n",s);
        fclose(fp);
        return COMMAND;
    }

    if(strcmp(s, "undo") == 0){
        reset_canvas(c);
        Command* p = his->begin;
        if(p != NULL){
            while(p->next != NULL){
                interpret_command((p->str), his, c);
                p = p->next;
                rewind_screen(1);
            }
            pop_back(his);
        }
        clear_command();
        printf("undo one operation\n");
        return COMMAND;
    }

    if(strcmp(s, "quit") == 0){
        return EXIT;
    }

    printf("error: unknown command.\n");

    return UNKNOWN;

}

void save_history(const char *filename, History* his){
    const char* default_history_file = "history.txt";
    if(filename == NULL){
        filename = default_history_file;
    }

    FILE* fp;
    if((fp = fopen(filename, "w")) == NULL){
        fprintf(stderr, "error: cannot open %s.\n", filename);
        return;
    }

    Command* p = his->begin;
    while(p != NULL){
        fprintf(fp, "%s", p->str);
        p = p->next;
    }

    fclose(fp);
}



