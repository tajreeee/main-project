#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int GRID_SIZE = 20;
const int NUM_OBSTACLES = 5;

struct SnakeSegment {
    int x, y;
};

struct Obstacle {
    int x, y;
};

enum class Direction { UP, DOWN, LEFT, RIGHT };

class SnakeGame {
public:
    SnakeGame();
    ~SnakeGame();
    void run();

//public:
    void initialize();
    void close();
    void handleInput();
    void update();
    void render();
    void spawnFood();
    void spawnObstacles();
    bool checkCollision();
    void spawnBonusFood();
    void updateBonusFoodTimer();
	void renderGameOver();
	void updateGameOverTimer();

   
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<SnakeSegment> snake;
    std::vector<Obstacle> obstacles;
    SDL_Point food;
    SDL_Point bonusFood;
    Direction direction;
    
	TTF_Font* font;
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void loadFont();
    
    bool displayGameOver=false;
	bool gameOver= false;
	int score=0;
	int foodsEaten=0;
	int bonusFoodTimer=0;
	int gameOverTimer=0;

};

SnakeGame::SnakeGame() : window(nullptr), renderer(nullptr){
    initialize();
}

SnakeGame::~SnakeGame() {
    close();
}

void SnakeGame::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        exit(1);
    }

    
    snake.push_back({SCREEN_WIDTH/2 , SCREEN_HEIGHT/2 });
    direction = Direction::UP;

    
    spawnFood();
    spawnObstacles();
	 loadFont();
}

void SnakeGame::close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void SnakeGame::run() {
    while (!gameOver) {
        handleInput();
        update();
        render();
        SDL_Delay(150);
    }

	
    displayGameOver = true;
    gameOverTimer = 5000; 

    while (displayGameOver) {
        render();
        updateGameOverTimer();
        SDL_Delay(150);
	}
    

	
}

void SnakeGame::handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            gameOver = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    if (direction != Direction::DOWN) direction = Direction::UP;
                    break;
                case SDLK_DOWN:
                    if (direction != Direction::UP) direction = Direction::DOWN;
                    break;
                case SDLK_LEFT:
                    if (direction != Direction::RIGHT) direction = Direction::LEFT;
                    break;
                case SDLK_RIGHT:
                    if (direction != Direction::LEFT) direction = Direction::RIGHT;
                    break;
            }
        }
    }
}

void SnakeGame::update() {
    // Move the snake
    int dx = 0, dy = 0;
    switch (direction) {
        case Direction::UP:
            dy = -GRID_SIZE;
            break;
        case Direction::DOWN:
            dy = GRID_SIZE;
            break;
        case Direction::LEFT:
            dx = -GRID_SIZE;
            break;
        case Direction::RIGHT:
            dx = GRID_SIZE;
            break;
    }

    
    for (int i = snake.size() - 1; i > 0; --i) {
        snake[i] = snake[i - 1];
    }

    
    snake[0].x += dx;
    snake[0].y += dy;



    if (checkCollision()) {
        gameOver = true;
        return;
    }

    
    if (snake[0].x == food.x && snake[0].y == food.y) {
        
        spawnFood();
        snake.push_back({-1, -1});  
        score++;
        foodsEaten++;

        if (foodsEaten % 5 == 0) {
            
            spawnBonusFood();
            bonusFoodTimer = 3000;  
        }
    }


    if (snake[0].x == bonusFood.x && snake[0].y == bonusFood.y) {
        
        snake.push_back({-1, -1});
        score += 5; 
        bonusFoodTimer = 0; 
    }

    
    updateBonusFoodTimer();


}

void SnakeGame::updateBonusFoodTimer() {
    if (bonusFoodTimer > 0) {
        bonusFoodTimer -= 150;  
        if (bonusFoodTimer <= 0) {
            
            bonusFoodTimer = 0;
        }
    }
}

void SnakeGame::renderGameOver() {
    SDL_Color textColor = {255, 255, 255, 255};
    std::string gameOverText = "Game Over! Score: " + std::to_string(score);
    renderText(gameOverText, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, textColor);
}

void SnakeGame::loadFont() {
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        exit(1);
    }

    font = TTF_OpenFont("tajree.ttf", 24); 
    if (!font) {
        std::cerr << "Font loading failed: " << TTF_GetError() << std::endl;
        exit(1);
    }
}

void SnakeGame::updateGameOverTimer() {
    if (gameOverTimer > 0) {
        gameOverTimer -= 150; 
        if (gameOverTimer <= 0) {
            
            displayGameOver = false;
        }
    }
}


void SnakeGame::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "Text rendering failed: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect textRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &textRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


void SnakeGame::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

	if(!gameOver)
	{
		
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    for (const auto& obstacle : obstacles) {
        SDL_Rect rect = {obstacle.x, obstacle.y, GRID_SIZE, GRID_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }

    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const auto& segment : snake) {
        SDL_Rect rect = {segment.x, segment.y, GRID_SIZE, GRID_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }

    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect foodRect = {food.x, food.y, GRID_SIZE, GRID_SIZE};
    SDL_RenderFillRect(renderer, &foodRect);

    
    if (bonusFoodTimer > 0) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_Rect bonusFoodRect = {bonusFood.x, bonusFood.y, GRID_SIZE, GRID_SIZE};
        SDL_RenderFillRect(renderer, &bonusFoodRect);
    }

	SDL_Color textColor = {255, 255, 255, 255};
    std::string scoreText = "Score: " + std::to_string(score);
    renderText(scoreText, 10, 10, textColor);
	}

	else
	{
		renderGameOver();
	}

    
    SDL_RenderPresent(renderer);
}

void SnakeGame::spawnFood() {
    
    int maxX = SCREEN_WIDTH / GRID_SIZE;
    int maxY = SCREEN_HEIGHT / GRID_SIZE;
    food.x = rand() % maxX * GRID_SIZE;
    food.y = rand() % maxY * GRID_SIZE;

    
    for (const auto& obstacle : obstacles) {
        if (food.x == obstacle.x && food.y == obstacle.y) {
            spawnFood(); 
            return;
        }
    }

    for (const auto& segment : snake) {
        if (food.x == segment.x && food.y == segment.y) {
            spawnFood();
            return;
        }
    }

    if (food.x == bonusFood.x && food.y == bonusFood.y) {
        spawnFood(); 
        return;
    }
}

void SnakeGame::spawnObstacles() {
    
    int maxX = SCREEN_WIDTH / GRID_SIZE;
    int maxY = SCREEN_HEIGHT / GRID_SIZE;

    for (int i = 0; i < NUM_OBSTACLES; ++i) {
        Obstacle obstacle;
        obstacle.x = rand() % maxX * GRID_SIZE;
        obstacle.y = rand() % maxY * GRID_SIZE;

        
        for (const auto& existingObstacle : obstacles) {
            if (obstacle.x == existingObstacle.x && obstacle.y == existingObstacle.y) {
                --i; 
                break;
            }
        }

        for (const auto& segment : snake) {
            if (obstacle.x == segment.x && obstacle.y == segment.y) {
                --i;
                break;
            }
        }

        if (obstacle.x == food.x && obstacle.y == food.y) {
            --i; 
            continue;
        }

        if (obstacle.x == bonusFood.x && obstacle.y == bonusFood.y) {
            --i; 
            continue;
        }

        obstacles.push_back(obstacle);
    }
}

void SnakeGame::spawnBonusFood() {
    
    int maxX = SCREEN_WIDTH / GRID_SIZE;
    int maxY = SCREEN_HEIGHT / GRID_SIZE;
    bonusFood.x = rand() % maxX * GRID_SIZE;
    bonusFood.y = rand() % maxY * GRID_SIZE;

    
    for (const auto& obstacle : obstacles) {
        if (bonusFood.x == obstacle.x && bonusFood.y == obstacle.y) {
            spawnBonusFood(); 
            return;
        }
    }

    for (const auto& segment : snake) {
        if (bonusFood.x == segment.x && bonusFood.y == segment.y) {
            spawnBonusFood(); 
            return;
        }
    }

    if (bonusFood.x == food.x && bonusFood.y == food.y) {
        spawnBonusFood(); 
        return;
    }
}

bool SnakeGame::checkCollision() {
    
    int headX = snake[0].x;
    int headY = snake[0].y;

    
    for (size_t i = 1; i < snake.size(); ++i) {
        if (headX == snake[i].x && headY == snake[i].y) {
            return true;
        }
    }

    
    for (const auto& obstacle : obstacles) {
        if (headX == obstacle.x && headY == obstacle.y) {
            return true;
        }
    } 

    
    if (headX < 0 || headX >= SCREEN_WIDTH || headY < 0 || headY >= SCREEN_HEIGHT) {
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    SnakeGame game;
    game.run();
    return 0;
}