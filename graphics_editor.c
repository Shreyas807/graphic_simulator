#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS 30
#define COLS 60
#define MAX_OBJECTS 20

/* ─── Canvas ─────────────────────────────────────────── */
char canvas[ROWS][COLS];

void clearCanvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

void displayCanvas() {
    /* Top border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            printf("%c", canvas[r][c]);
        printf("|\n");
    }

    /* Bottom border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

/* Safe pixel set – ignores out-of-bounds coordinates */
void setPixel(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* ─── Shape store ─────────────────────────────────────── */
typedef enum { SHAPE_CIRCLE, SHAPE_RECT, SHAPE_LINE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int active;
    /* Parameters (meaning depends on type) */
    int x1, y1, x2, y2, x3, y3, radius;
} Shape;

Shape objects[MAX_OBJECTS];
int objectCount = 0;

/* ─── Drawing functions ───────────────────────────────── */

/* Bresenham's line algorithm */
void drawLine(int r1, int c1, int r2, int c2, char ch) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        setPixel(r1, c1, ch);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

/* Midpoint circle algorithm */
void drawCircle(int cr, int cc, int radius, char ch) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        setPixel(cr + y, cc + x, ch);
        setPixel(cr - y, cc + x, ch);
        setPixel(cr + y, cc - x, ch);
        setPixel(cr - y, cc - x, ch);
        setPixel(cr + x, cc + y, ch);
        setPixel(cr - x, cc + y, ch);
        setPixel(cr + x, cc - y, ch);
        setPixel(cr - x, cc - y, ch);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void drawRect(int r1, int c1, int r2, int c2, char ch) {
    /* Horizontal edges */
    for (int c = c1; c <= c2; c++) {
        setPixel(r1, c, ch);
        setPixel(r2, c, ch);
    }
    /* Vertical edges */
    for (int r = r1; r <= r2; r++) {
        setPixel(r, c1, ch);
        setPixel(r, c2, ch);
    }
}

void drawTriangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch) {
    drawLine(r1, c1, r2, c2, ch);
    drawLine(r2, c2, r3, c3, ch);
    drawLine(r3, c3, r1, c1, ch);
}

/* ─── Redraw all active objects ───────────────────────── */
void redrawAll() {
    clearCanvas();
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        Shape *s = &objects[i];
        switch (s->type) {
            case SHAPE_CIRCLE:
                drawCircle(s->y1, s->x1, s->radius, '*'); break;
            case SHAPE_RECT:
                drawRect(s->y1, s->x1, s->y2, s->x2, '*'); break;
            case SHAPE_LINE:
                drawLine(s->y1, s->x1, s->y2, s->x2, '_'); break;
            case SHAPE_TRIANGLE:
                drawTriangle(s->y1, s->x1, s->y2, s->x2, s->y3, s->x3, '*'); break;
        }
    }
}

/* ─── Add object helpers ──────────────────────────────── */
int newSlot() {
    if (objectCount >= MAX_OBJECTS) {
        printf("Max objects reached (%d).\n", MAX_OBJECTS);
        return -1;
    }
    return objectCount++;
}

void addCircle() {
    int cx, cy, r;
    printf("Center col (0-%d): ", COLS - 1); scanf("%d", &cx);
    printf("Center row (0-%d): ", ROWS - 1); scanf("%d", &cy);
    printf("Radius: ");                        scanf("%d", &r);

    int i = newSlot(); if (i < 0) return;
    objects[i] = (Shape){ SHAPE_CIRCLE, 1, cx, cy, 0, 0, 0, 0, r };
    printf("Circle added as object #%d.\n", i + 1);
}

void addRect() {
    int c1, r1, c2, r2;
    printf("Top-left  col row: "); scanf("%d %d", &c1, &r1);
    printf("Bot-right col row: "); scanf("%d %d", &c2, &r2);

    int i = newSlot(); if (i < 0) return;
    objects[i] = (Shape){ SHAPE_RECT, 1, c1, r1, c2, r2, 0, 0, 0 };
    printf("Rectangle added as object #%d.\n", i + 1);
}

void addLine() {
    int c1, r1, c2, r2;
    printf("Start col row: "); scanf("%d %d", &c1, &r1);
    printf("End   col row: "); scanf("%d %d", &c2, &r2);

    int i = newSlot(); if (i < 0) return;
    objects[i] = (Shape){ SHAPE_LINE, 1, c1, r1, c2, r2, 0, 0, 0 };
    printf("Line added as object #%d.\n", i + 1);
}

void addTriangle() {
    int c1, r1, c2, r2, c3, r3;
    printf("Vertex 1 col row: "); scanf("%d %d", &c1, &r1);
    printf("Vertex 2 col row: "); scanf("%d %d", &c2, &r2);
    printf("Vertex 3 col row: "); scanf("%d %d", &c3, &r3);

    int i = newSlot(); if (i < 0) return;
    objects[i] = (Shape){ SHAPE_TRIANGLE, 1, c1, r1, c2, r2, c3, r3, 0 };
    printf("Triangle added as object #%d.\n", i + 1);
}

/* ─── List / Delete ───────────────────────────────────── */
void listObjects() {
    int found = 0;
    printf("\n  ID  Type        Details\n");
    printf("  --- ----------- ------------------------------------\n");
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Shape *s = &objects[i];
        printf("  %-3d ", i + 1);
        switch (s->type) {
            case SHAPE_CIRCLE:
                printf("Circle      center(%d,%d) r=%d\n", s->x1, s->y1, s->radius); break;
            case SHAPE_RECT:
                printf("Rectangle   (%d,%d)->(%d,%d)\n", s->x1, s->y1, s->x2, s->y2); break;
            case SHAPE_LINE:
                printf("Line        (%d,%d)->(%d,%d)\n", s->x1, s->y1, s->x2, s->y2); break;
            case SHAPE_TRIANGLE:
                printf("Triangle    (%d,%d) (%d,%d) (%d,%d)\n",
                       s->x1, s->y1, s->x2, s->y2, s->x3, s->y3); break;
        }
    }
    if (!found) printf("  (no objects)\n");
    printf("\n");
}

void deleteObject() {
    listObjects();
    if (objectCount == 0) return;
    int id;
    printf("Enter object ID to delete (1-%d): ", objectCount);
    scanf("%d", &id);
    if (id < 1 || id > objectCount || !objects[id - 1].active) {
        printf("Invalid ID.\n"); return;
    }
    objects[id - 1].active = 0;
    printf("Object #%d deleted.\n", id);
}

/* ─── Menus ───────────────────────────────────────────── */
void addMenu() {
    int ch;
    printf("\n  Add shape:\n");
    printf("  1. Circle\n");
    printf("  2. Rectangle\n");
    printf("  3. Line\n");
    printf("  4. Triangle\n");
    printf("  Choice: ");
    scanf("%d", &ch);
    switch (ch) {
        case 1: addCircle();   break;
        case 2: addRect();     break;
        case 3: addLine();     break;
        case 4: addTriangle(); break;
        default: printf("Invalid choice.\n");
    }
    redrawAll();
}

void mainMenu() {
    int choice;
    clearCanvas();

    while (1) {
        printf("\n╔═══════════════════════════════╗\n");
        printf("║   2D ASCII Graphics Editor    ║\n");
        printf("╠═══════════════════════════════╣\n");
        printf("║  1. Add object                ║\n");
        printf("║  2. Delete object             ║\n");
        printf("║  3. List objects              ║\n");
        printf("║  4. Show canvas               ║\n");
        printf("║  5. Clear all                 ║\n");
        printf("║  6. Quit                      ║\n");
        printf("╚═══════════════════════════════╝\n");
        printf("  Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addMenu(); break;
            case 2:
                deleteObject();
                redrawAll();
                break;
            case 3: listObjects(); break;
            case 4:
                redrawAll();
                displayCanvas();
                break;
            case 5:
                objectCount = 0;
                clearCanvas();
                printf("Canvas cleared.\n");
                break;
            case 6:
                printf("Bye!\n");
                return;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

int main() {
    mainMenu();
    return 0;
}
