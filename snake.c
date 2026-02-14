#include "common.h"
int score = 0;

#define MAX_SNAKE_LENGTH 100
#define BOARD_WIDTH 80
#define BOARD_HEIGHT 25
int game_running = 0;
struct Point {
    int x, y;
};
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
struct Point snake[MAX_SNAKE_LENGTH];
int snake_length = 3;
int dx = 1, dy = 0; // Starting direction (moving right)
struct Point food;

void place_food() {
    // In a real kernel, you'd use a random number generator
    // For now, we'll just pick a spot
    food.x = 20;
    food.y = 10;
}

void move_snake() {
    // 1. Body segments follow the head
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
    }
    // 2. Apply the direction chosen by update_direction
    snake[0].x += dx;
    snake[0].y += dy;
}

void draw_snake() {
    for (int i = 0; i < snake_length; i++) {
        // Draw each segment as an 'O'
        terminal_putentryat('O', 0x0A, snake[i].x, snake[i].y);
    }
    // Draw food as an '*'
    terminal_putentryat('*', 0x0C, food.x, food.y);
}

void update_direction(char c) {
    // DEBUG: This will draw the key you pressed at the top-right corner.
    // If you don't see the letter appear, your isr.c isn't calling this!
    terminal_putentryat(c, 0x0E, 79, 0); 

    // Handle WASD (Lowercase and Uppercase)
    // The '&& dy == 0' part prevents the snake from reversing into itself
    if ((c == 'w' || c == 'W') && dy == 0) {
        dx = 0; 
        dy = -1;
    } else if ((c == 's' || c == 'S') && dy == 0) {
        dx = 0; 
        dy = 1;
    } else if ((c == 'a' || c == 'A') && dx == 0) {
        dx = -1; 
        dy = 0;
    } else if ((c == 'd' || c == 'D') && dx == 0) {
        dx = 1; 
        dy = 0;
    }
    
    // Optional: Press 'q' to quit the game manually
    if (c == 'q' || c == 'Q') {
        game_running = 0;
    }
}

void run_snake_game() {
    game_running = 1;
    score = 0;
    snake_length = 3;
    
    // Reset starting position and direction
    snake[0].x = 40; snake[0].y = 12; 
    dx = 1; dy = 0; 
    
    place_food(); // Ensure food has a position

    while(game_running) {
        move_snake();

        // --- WALLS ---
        if (snake[0].x < 0 || snake[0].x >= 80 || snake[0].y < 0 || snake[0].y >= 25) {
            game_running = 0;
            break; 
        }

        // --- SCORE & EATING ---
        if (snake[0].x == food.x && snake[0].y == food.y) {
            score += 1;
            snake_length++;
            place_food(); 
        }

        terminal_initialize(); // Clear previous frame
        
        // Draw Score at (0,0)
        terminal_putentryat('S', 0x07, 0, 0);
        terminal_putentryat((score % 10) + '0', 0x0E, 1, 0); 
        
        draw_snake(); // Draws 'O' for snake and '*' for food
        
        delay(60); // Use a high value so you can actually see it move
    }
}
