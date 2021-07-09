#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <map>
#include <string>

class Cell
{
public:
    int north;
    int south;
    int east;
    int west;
    float x_pos;
    float y_pos;

    Cell(int north, int south, int east, int west, float x_pos, float y_pos)
    {
        this->north = north;
        this->south = south;
        this->east = east;
        this->west = west;
        this->x_pos = x_pos;
        this->y_pos = y_pos;
    }
    void update_boundaries(int north, int south, int east, int west)
    {
        this->north = north;
        this->south = south;
        this->east = east;
        this->west = west;
    }
};

class Player
{
public:
    float x_pos;
    float y_pos;
    float l_big;
    float b_big;
    float l_small;
    float b_small;
    float translate_X;
    float translate_Y;
    float translate_speed;
    int tasks_done;
    int pwp_btn;
    int vap_btn;
    int score;
    int torch;
    int health;

    Player(float x_pos, float y_pos, float l_big, float b_big, float l_small, float b_small, float translate_X, float translate_Y)
    {
        this->x_pos = x_pos;
        this->y_pos = y_pos;
        this->l_big = l_big;
        this->b_big = b_big;
        this->l_small = l_small;
        this->b_small = b_small;
        this->translate_X = translate_X;
        this->translate_Y = translate_Y;
        this->translate_speed = 0.02f;
        this->tasks_done = 0;
        this->pwp_btn = 0;
        this->vap_btn = 0;
        this->score = 0;
        this->torch = 0;
        this->health = 100;
    }
    void move_X(int direction)
    {
        if (direction == 1)
        {
            this->translate_X = this->translate_X + this->translate_speed;
        }
        else
        {
            this->translate_X = this->translate_X - this->translate_speed;
        }
    }
    void move_Y(int direction)
    {
        if (direction == 1)
        {
            this->translate_Y = this->translate_Y + this->translate_speed;
        }
        else
        {
            this->translate_Y = this->translate_Y - this->translate_speed;
        }
    }
    void pwp_btn_taken()
    {
        this->tasks_done = this->tasks_done + 1;
        this->pwp_btn = 1;
    }
    void vap_btn_taken()
    {
        this->tasks_done = this->tasks_done + 1;
        this->vap_btn = 1;
    }
    void update_score(int value)
    {
        this->score = this->score + value;
    }
    void toggle_torch()
    {
        this->torch = !this->torch;
    }
};

class Powerups
{
public:
    int isbomb;
    int iscoin;
    int istaken;
    float x_pos;
    float y_pos;
    Powerups(int isbomb, int iscoin, float x_pos, float y_pos)
    {
        this->isbomb = isbomb;
        this->iscoin = iscoin;
        this->x_pos = x_pos;
        this->y_pos = y_pos;
        this->istaken = 0;
    }
    void pwp_picked()
    {
        this->istaken = 1;
    }
};

class Darkness
{
public:
    float x_pos;
    float y_pos;
    float translate_X;
    float translate_Y;
    float translate_speed;

    Darkness(float x_pos, float y_pos)
    {
        this->x_pos = x_pos;
        this->y_pos = y_pos;
        this->translate_X = 0.0f;
        this->translate_Y = 0.0f;
        this->translate_speed = 0.02f;
    }
    void move_X(int direction)
    {
        if (direction == 1)
        {
            this->translate_X = this->translate_X + this->translate_speed;
        }
        else
        {
            this->translate_X = this->translate_X - this->translate_speed;
        }
    }
    void move_Y(int direction)
    {
        if (direction == 1)
        {
            this->translate_Y = this->translate_Y + this->translate_speed;
        }
        else
        {
            this->translate_Y = this->translate_Y - this->translate_speed;
        }
    }
};

struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void keyboardChar(GLFWwindow *window, unsigned int key);
void RenderText(unsigned int shaderProgramText, std::string text, float x, float y, float scale, glm::vec3 color);

// maze array
Cell *maze = (Cell *)malloc(sizeof(Cell) * 100);
int maze_size;

// Player
Player p1 = Player(-0.9f, -0.9f, 0.06f, 0.03f, 0.015f, 0.0225f, 0.3f, 0.4f);

// bombs and coins
Powerups *bombcoin = (Powerups *)malloc(sizeof(Powerups) * 10);
int bombcoin_size;
float vertices_bomb_coin[10][36];

// Darkness
Darkness dark = Darkness(-0.6f, -0.5f);
time_t torch_time;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::map<GLchar, Character> Characters;

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "uniform mat4 MVP;\n"
                                 "out vec3 ourColor;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   vec4 v = vec4(aPos , 1);\n"
                                 "   gl_Position = MVP * v;\n"
                                 "   ourColor = aColor;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(ourColor, 1.0f);\n"
                                   "}\n\0";

const char *vertexShaderTextSource = "#version 330 core\n"
                                     "layout (location = 0) in vec4 vertex;\n"
                                     "out vec2 TexCoords;\n"
                                     "uniform mat4 projection;\n"
                                     "void main()\n"
                                     "{\n"
                                     "gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
                                     "TexCoords = vertex.zw;\n"
                                     "}\0";

const char *fragmentShaderTextSource = "#version 330 core\n"
                                       "in vec2 TexCoords;\n"
                                       "out vec4 color;\n"
                                       "uniform sampler2D text;\n"
                                       "uniform vec3 textColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
                                       "color = vec4(textColor, 1.0) * sampled;\n"
                                       "}\0";

void rendermaze()
{
    // make maze
    int index = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            int north = rand() % 2;
            int south = rand() % 2;
            int east = rand() % 2;
            int west = rand() % 2;
            if ((i == 0) && (j == 0))
            {
                south = 1;
                east = 0;
                north = 0;
                west = 0;
                maze[index] = Cell(north, south, east, west, -0.5f + 0.1f * i, -0.5f + 0.1f * j);
                index++;
            }
            else if ((i == 9) && (j == 9))
            {
                north = 1;
                west = 0;
                south = 0;
                east = 0;
                maze[index] = Cell(north, south, east, west, -0.5f + 0.1f * i, -0.5f + 0.1f * j);
                maze[index - 1].update_boundaries(0, 1, 0, 1);
                maze[index - 10].update_boundaries(1, 0, 1, 0);
                maze[index - 11].update_boundaries(0, maze[index - 11].south, 0, 0);
                index++;
            }
            else
            {
                if (i == 0)
                {
                    south = maze[index - 1].north;
                    east = 1;
                    if (j == 9)
                    {
                        north = 1;
                    }
                }
                else if (j == 0)
                {
                    south = 1;
                    east = maze[index - 10].west;
                    if (i == 9)
                    {
                        west = 1;
                    }
                }
                else
                {
                    south = maze[index - 1].north;
                    east = maze[index - 10].west;
                    if (i == 9)
                    {
                        west = 1;
                    }
                    if (j == 9)
                    {
                        north = 1;
                    }
                }

                maze[index] = Cell(north, south, east, west, -0.5f + 0.1f * i, -0.5f + 0.1f * j);
                index++;
            }
        }
    }
}

float *getcoordmaze(float *vertices_maze)
{
    // float vertices_maze[4800];
    // getting coordinates of maze
    int index = 0;
    for (int i = 0; i < 100; i++)
    {
        // cout << "HelloIn" << endl;
        // cout << maze[i].north << " " << maze[i].south << " " << maze[i].east << " " << maze[i].west << " " << maze[i].x_pos << " " << maze[i].y_pos << endl;
        if (maze[i].north == 1)
        {
            // cout << "North" << endl;
            // coordinates 1
            vertices_maze[index] = maze[i].x_pos;
            index++;
            vertices_maze[index] = maze[i].y_pos + 0.1f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 1
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;

            // coordinates 2
            vertices_maze[index] = maze[i].x_pos + 0.1f;
            index++;
            vertices_maze[index] = maze[i].y_pos + 0.1f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 2
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
        }
        if (maze[i].south == 1)
        {
            // cout << "South" << endl;
            // coordinates 1
            vertices_maze[index] = maze[i].x_pos;
            index++;
            vertices_maze[index] = maze[i].y_pos;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 1
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;

            // coordinates 2
            vertices_maze[index] = maze[i].x_pos + 0.1f;
            index++;
            vertices_maze[index] = maze[i].y_pos;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 2
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
        }
        if (maze[i].east == 1)
        {
            // cout << "East" << endl;
            // coordinates 1
            vertices_maze[index] = maze[i].x_pos;
            index++;
            vertices_maze[index] = maze[i].y_pos;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 1
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;

            // coordinates 2
            vertices_maze[index] = maze[i].x_pos;
            index++;
            vertices_maze[index] = maze[i].y_pos + 0.1f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 2
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
        }
        if (maze[i].west == 1)
        {
            // cout << "West" << endl;
            // coordinates 1
            vertices_maze[index] = maze[i].x_pos + 0.1f;
            index++;
            vertices_maze[index] = maze[i].y_pos;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 1
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;

            // coordinates 2
            vertices_maze[index] = maze[i].x_pos + 0.1f;
            index++;
            vertices_maze[index] = maze[i].y_pos + 0.1f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            // colors 2
            vertices_maze[index] = 1.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
            vertices_maze[index] = 0.0f;
            index++;
        }
    }
    maze_size = index;
    return vertices_maze;
}

float *getcoordchar(float *vertices_char_temp)
{
    int index = 0;
    // main body
    // triangle 1
    vertices_char_temp[index] = p1.x_pos;
    index++;
    vertices_char_temp[index] = p1.y_pos;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos;
    index++;
    vertices_char_temp[index] = p1.y_pos + p1.l_big;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + p1.b_big;
    index++;
    vertices_char_temp[index] = p1.y_pos + p1.l_big;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    // triangle 2
    vertices_char_temp[index] = p1.x_pos;
    index++;
    vertices_char_temp[index] = p1.y_pos;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + p1.b_big;
    index++;
    vertices_char_temp[index] = p1.y_pos;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + p1.b_big;
    index++;
    vertices_char_temp[index] = p1.y_pos + p1.l_big;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    // body tag
    // triangle 1
    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small);
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big);
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small);
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big) + p1.l_small;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small) + p1.b_small;
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big) + p1.l_small;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    // triangle 2
    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small);
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big);
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small) + p1.b_small;
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big);
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    vertices_char_temp[index] = p1.x_pos + 0.5f * (p1.b_big - p1.b_small) + p1.b_small;
    index++;
    vertices_char_temp[index] = p1.y_pos + 0.7f * (p1.l_big) + p1.l_small;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 0.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;
    vertices_char_temp[index] = 1.0f;
    index++;

    // head
    for (int i = 0; i < 180; i++)
    {
        vertices_char_temp[index] = p1.x_pos + 0.5f * p1.b_big;
        index++;
        vertices_char_temp[index] = p1.y_pos + p1.l_big;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 1.0f;
        index++;
        vertices_char_temp[index] = p1.x_pos + 0.5f * p1.b_big + cos(i * (M_PI / 180)) * p1.b_big * 0.5f;
        index++;
        vertices_char_temp[index] = p1.y_pos + p1.l_big + sin(i * (M_PI / 180)) * p1.b_big * 0.5f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 1.0f;
        index++;
        vertices_char_temp[index] = p1.x_pos + 0.5f * p1.b_big + cos((i + 1) * (M_PI / 180)) * p1.b_big * 0.5f;
        index++;
        vertices_char_temp[index] = p1.y_pos + p1.l_big + sin((i + 1) * (M_PI / 180)) * p1.b_big * 0.5f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 0.0f;
        index++;
        vertices_char_temp[index] = 1.0f;
        index++;
    }
    return vertices_char_temp;
}

void renderbombcoin()
{
    float x_pos = -0.08f;
    float y_pos[] = {
        0.12f,
        0.32f,
        -0.38f,
        0.02f,
        -0.08f,
        -0.28f,
        -0.18f,
    };
    for (int i = 0; i < 10; i++)
    {
        int isbomb = rand() % 2;
        int iscoin = 1 - isbomb;
        float x_posi = x_pos;
        float y_posi = y_pos[rand() % 7];
        if ((i > 0) && (x_posi == bombcoin[i - 1].x_pos) && (y_posi == bombcoin[i - 1].y_pos))
        {
            y_posi = y_posi + 0.1f;
        }
        bombcoin[i] = Powerups(isbomb, iscoin, x_posi, y_posi);
        if (i % 2 == 1)
        {
            x_pos = x_pos + 0.10f;
        }
    }
}

int collision_maze_p1(int direction)
{
    // check collision
    for (int i = 0; i < 100; i++)
    {
        // check collision with maze cell with respect to direction
        // 1 for up
        // 2 for down
        // 3 for left
        // 4 for right
        // cell selection
        if ((p1.x_pos + p1.translate_X >= maze[i].x_pos && p1.x_pos + p1.translate_X <= maze[i].x_pos + 0.1f) && (p1.y_pos + p1.translate_Y >= maze[i].y_pos && p1.y_pos + p1.translate_Y <= maze[i].y_pos + 0.1f))
        {
            if (direction == 1)
            {
                if (maze[i].north == 1)
                {
                    if ((maze[i].y_pos + 0.1f <= p1.y_pos + p1.translate_Y + p1.l_big + 0.5f * p1.b_big + p1.translate_speed) && (maze[i].y_pos + 0.1f >= p1.y_pos + p1.translate_Y + p1.l_big + 0.5f * p1.b_big))
                    {
                        // cout << i << endl;
                        return 1;
                    }
                }
            }
            else if (direction == 2)
            {
                if (maze[i].south == 1)
                {
                    if ((maze[i].y_pos >= p1.y_pos + p1.translate_Y - p1.translate_speed) && (maze[i].y_pos <= p1.y_pos + p1.translate_Y))
                    {
                        // cout << i << endl;
                        return 1;
                    }
                }
            }
            else if (direction == 3)
            {
                if (maze[i].east == 1)
                {
                    if ((maze[i].x_pos >= p1.x_pos + p1.translate_X - p1.translate_speed) && (maze[i].x_pos <= p1.x_pos + p1.translate_X))
                    {
                        // cout << i << endl;
                        return 1;
                    }
                }
            }
            else
            {
                if (maze[i].west == 1)
                {
                    if ((maze[i].x_pos + 0.1f <= p1.x_pos + p1.translate_X + p1.b_big + p1.translate_speed) && (maze[i].x_pos + 0.1f >= p1.x_pos + p1.translate_X + p1.b_big))
                    {
                        // cout << i << endl;
                        return 1;
                    }
                }
            }
        }
    }

    // if not collision
    return 0;
}

int collision_pwpbtn_p1()
{
    if (((p1.x_pos + p1.translate_X >= 0.02f && p1.x_pos + p1.translate_X <= 0.06f) && (p1.y_pos + p1.translate_Y >= -0.49f && p1.y_pos + p1.translate_Y <= -0.45f)) || ((p1.x_pos + p1.translate_X + p1.b_big * 0.5f >= 0.02f && p1.x_pos + p1.translate_X + p1.b_big * 0.5f <= 0.06f) && (p1.y_pos + p1.translate_Y + 0.5f * p1.l_big >= -0.49f && p1.y_pos + p1.translate_Y + 0.5f * p1.l_big <= -0.45f)))
    {
        return 1;
    }

    return 0;
}

int collision_vapbtn_p1()
{
    if (((p1.x_pos + p1.translate_X >= -0.08f && p1.x_pos + p1.translate_X <= -0.04f) && (p1.y_pos + p1.translate_Y >= 0.21f && p1.y_pos + p1.translate_Y <= 0.25f)) || ((p1.x_pos + p1.translate_X + 0.5f * p1.b_big >= -0.08f && p1.x_pos + p1.translate_X + 0.5f * p1.b_big <= -0.04f) && (p1.y_pos + p1.translate_Y + 0.5f * p1.l_big >= 0.21f && p1.y_pos + p1.translate_Y + 0.5f * p1.l_big <= 0.25f)))
    {
        return 1;
    }
    return 0;
}
void collision_p1_powerups()
{
    for (int i = 0; i < 10; i++)
    {
        if (bombcoin[i].istaken == 0)
        {
            if (((p1.x_pos + p1.translate_X >= bombcoin[i].x_pos && p1.x_pos + p1.translate_X <= bombcoin[i].x_pos + 0.05f) && (p1.y_pos + p1.translate_Y >= bombcoin[i].y_pos && p1.y_pos + p1.translate_Y <= bombcoin[i].y_pos + 0.05f)) || ((p1.x_pos + p1.translate_X + p1.b_big * 0.5f >= bombcoin[i].x_pos && p1.x_pos + p1.translate_X + p1.b_big * 0.5f <= bombcoin[i].x_pos + 0.05f) && (p1.y_pos + p1.translate_Y + 0.5f * p1.l_big >= bombcoin[i].y_pos && p1.y_pos + p1.translate_Y + 0.5f * p1.l_big <= bombcoin[i].y_pos + 0.05f)))
            {
                bombcoin[i].pwp_picked();
                if (bombcoin[i].iscoin == 1)
                {
                    p1.update_score(10);
                }
                else
                {
                    p1.update_score(-10);
                }
            }
        }
    }
}

int collision_endgate_p1()
{
    if ((0.5f <= p1.x_pos + p1.translate_X + p1.b_big + p1.translate_speed) && (0.5f >= p1.x_pos + p1.translate_X + p1.b_big))
    {
        // cout << i << endl;
        return 1;
    }
    return 0;
}

int gameend()
{
    if (p1.x_pos + 0.5 * p1.b_big + p1.translate_X >= 0.5f)
    {
        return 1;
    }
    return 0;
}

// for textures
unsigned int VAO_text, VBO_text;

int main()
{
    // glfw: initialize and configure
    // how to handle the windows n all are done with this
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Among Us", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCharCallback(window, keyboardChar);

    // glad: load all OpenGL function pointers connect to opengl implementaion from driver
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShaderText = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderText, 1, &vertexShaderTextSource, NULL);
    glCompileShader(vertexShaderText);
    // check for shader compile errors
    int successText;
    char infoLogText[512];
    glGetShaderiv(vertexShaderText, GL_COMPILE_STATUS, &successText);
    if (!successText)
    {
        glGetShaderInfoLog(vertexShaderText, 512, NULL, infoLogText);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLogText << std::endl;
    }
    // fragment shader
    unsigned int fragmentShaderText = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderText, 1, &fragmentShaderTextSource, NULL);
    glCompileShader(fragmentShaderText);
    // check for shader compile errors
    glGetShaderiv(fragmentShaderText, GL_COMPILE_STATUS, &successText);
    if (!successText)
    {
        glGetShaderInfoLog(fragmentShaderText, 512, NULL, infoLogText);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLogText << std::endl;
    }
    // link shaders
    unsigned int shaderProgramText = glCreateProgram();
    glAttachShader(shaderProgramText, vertexShaderText);
    glAttachShader(shaderProgramText, fragmentShaderText);
    glLinkProgram(shaderProgramText);
    // check for linking errors
    glGetProgramiv(shaderProgramText, GL_LINK_STATUS, &successText);
    if (!successText)
    {
        glGetProgramInfoLog(shaderProgramText, 512, NULL, infoLogText);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLogText << std::endl;
    }
    glDeleteShader(vertexShaderText);
    glDeleteShader(fragmentShaderText);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    glUseProgram(shaderProgramText);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramText, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, "../assets/fonts/rustico.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else
    {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)};
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // render maze
    rendermaze();

    float vertices_maze[4800];
    float *vertices_maze_1;
    vertices_maze_1 = getcoordmaze(vertices_maze);

    float vertices[maze_size];
    for (int i = 0; i < maze_size; i++)
    {
        vertices[i] = vertices_maze_1[i];
    }
    // get Player matrix
    float vertices_char_temp[3312];
    float *vertices_char_1;
    vertices_char_1 = getcoordchar(vertices_char_temp);

    // powerup button
    float vertices_pwp_btn[] = {
        0.02f,
        -0.49f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.06f,
        -0.49f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.04f,
        -0.45f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
    };

    // vaporiser button
    float vertices_vap_btn[] = {
        -0.08f,
        0.21f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.04f,
        0.21f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.06f,
        0.25f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
    };

    // Bombs and Coins
    renderbombcoin();
    for (int i = 0; i < 10; i++)
    {
        int factor;
        if (bombcoin[i].isbomb == 1)
        {
            factor = 1;
        }
        else
        {
            factor = 0;
        }

        int index = 0;
        // triangle 1;
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }

        // Triangle 2
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }
        vertices_bomb_coin[i][index] = bombcoin[i].x_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = bombcoin[i].y_pos + 0.05f;
        index++;
        vertices_bomb_coin[i][index] = 0.0f;
        index++;
        if (factor == 1)
        {
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.0f;
            index++;
        }
        else
        {
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 1.0f;
            index++;
            vertices_bomb_coin[i][index] = 0.77f;
            index++;
        }
    }

    // exit closing gate
    float vertices_end_gate[] = {0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.4f, 0.0f, 1.0f, 1.0f, 1.0f};

    // dark rectangles
    float vertices_dark_rectangles[] = {
        -2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -2.0f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -2.0f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -2.0f, -0.56f, 0.0f, 0.0f, 0.0f, 0.0f,
        -2.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.66f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.51f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, -0.365f, 0.0f, 0.0f, 0.0f, 0.0f,
        2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO1, VAO1;
    glBindVertexArray(0);
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO1);

    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_char_temp), vertices_char_temp, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO2, VAO2;
    glBindVertexArray(0);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_pwp_btn), vertices_pwp_btn, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO3, VAO3;
    glBindVertexArray(0);
    glGenVertexArrays(1, &VAO3);
    glGenBuffers(1, &VBO3);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO3);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_vap_btn), vertices_vap_btn, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO4, VAO4;
    glBindVertexArray(0);
    glGenVertexArrays(1, &VAO4);
    glGenBuffers(1, &VBO4);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO4);

    glBindBuffer(GL_ARRAY_BUFFER, VBO4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_end_gate), vertices_end_gate, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO5, VAO5;
    glBindVertexArray(0);
    glGenVertexArrays(1, &VAO5);
    glGenBuffers(1, &VBO5);
    // first parameter is for number of buffer objects to create
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO5);

    glBindBuffer(GL_ARRAY_BUFFER, VBO5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_dark_rectangles), vertices_dark_rectangles, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int VBO_bombcoin[10];
    unsigned int VAO_bombcoin[10];
    for (int i = 0; i < 10; i++)
    {
        glBindVertexArray(0);
        glGenVertexArrays(1, &VAO_bombcoin[i]);
        glGenBuffers(1, &VBO_bombcoin[i]);
        // first parameter is for number of buffer objects to create
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO_bombcoin[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_bombcoin[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_bomb_coin[i]), vertices_bomb_coin[i], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glGenVertexArrays(1, &VAO_text);
    glGenBuffers(1, &VBO_text);
    glBindVertexArray(VAO_text);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // as we only have a single shader, we could also just activate our shader once beforehand if we want to
    glUseProgram(shaderProgram);

    std::cout << p1.score << std::endl;

    // setup time
    time_t setup_time = time(0);

    int game_won = 0;
    int game_lost = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // glEnable(GL_BLEND);

        // render text
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Disable the depth test since the text is being rendered in 2D
        glDisable(GL_DEPTH_TEST);
        std::string health1 = "HEALTH : ";
        std::string health2 = std::to_string(p1.health);
        std::string health = health1 + health2;
        RenderText(shaderProgramText, health, 120.0f, 560.0f, 0.3f, glm::vec3(0.0f, 1.0f, 1.0f));
        std::string task0 = "TASKS : ";
        std::string task1 = std::to_string(p1.tasks_done);
        std::string task2 = " / 2";
        std::string task = task0 + task1 + task2;
        RenderText(shaderProgramText, task, 280.0f, 560.0f, 0.3f, glm::vec3(0.0f, 1.0f, 1.0f));
        if (p1.torch == 0)
        {
            RenderText(shaderProgramText, "TORCH : Off", 430.0f, 560.0f, 0.3f, glm::vec3(0.0f, 1.0f, 1.0f));
        }
        else
        {
            RenderText(shaderProgramText, "TORCH : On", 430.0f, 560.0f, 0.3f, glm::vec3(0.0f, 1.0f, 1.0f));
        }
        std::string time1 = "TIME LEFT : ";
        int total_time = 120 - int(time(0) - setup_time);
        if (total_time < 0 || game_won == 1 || game_lost == 1)
        {
            total_time = 0;
        }
        std::string time2 = std::to_string(total_time);
        std::string time_left = time1 + time2;
        RenderText(shaderProgramText, time_left, 560.0f, 560.0f, 0.3f, glm::vec3(0.0f, 1.0f, 1.0f));

        // glEnable(GL_DEPTH_TEST);

        /**
    * Optionally disable the blending
    * Can be removed since the 3D world might have some parts that needs to be rendered (semi-)transparent
    */
        glDisable(GL_BLEND);

        glUseProgram(shaderProgram);
        if (game_won == 0 && game_lost == 0)
        {
            // // render the triangle
            glm::mat4 test = glm::mat4(1.0f);
            glm::mat4 translate = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
            glm::mat4 MVP = test * translate;
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);

            glBindVertexArray(0);
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINES, 0, maze_size / 6);

            glm::mat4 test1 = glm::mat4(1.0f);
            glm::mat4 translate1 = glm::translate(glm::vec3(p1.translate_X, p1.translate_Y, 0.0f));
            glm::mat4 MVP1 = test1 * translate1;
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP1[0][0]);
            glBindVertexArray(0);
            glBindVertexArray(VAO1);
            glDrawArrays(GL_TRIANGLES, 0, 552);

            // check powerup caught
            if (p1.pwp_btn == 0)
            {
                if (collision_pwpbtn_p1() == 1)
                {
                    p1.pwp_btn_taken();
                }
            }

            if (p1.pwp_btn == 0)
            {
                glm::mat4 test2 = glm::mat4(1.0f);
                glm::mat4 translate2 = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
                glm::mat4 MVP2 = test2 * translate2;
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP2[0][0]);

                glBindVertexArray(0);
                glBindVertexArray(VAO2);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }

            if (p1.vap_btn == 0)
            {
                if (collision_vapbtn_p1() == 1)
                {
                    p1.vap_btn_taken();
                }
            }

            if (p1.vap_btn == 0)
            {
                glm::mat4 test3 = glm::mat4(1.0f);
                glm::mat4 translate3 = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
                glm::mat4 MVP3 = test3 * translate3;
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP3[0][0]);

                glBindVertexArray(0);
                glBindVertexArray(VAO3);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            if (p1.pwp_btn == 1)
            {
                collision_p1_powerups();
                for (int i = 0; i < 10; i++)
                {
                    if (bombcoin[i].istaken == 0)
                    {
                        glm::mat4 test5 = glm::mat4(1.0f);
                        glm::mat4 translate5 = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
                        glm::mat4 MVP5 = test5 * translate5;
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP5[0][0]);

                        glBindVertexArray(0);
                        glBindVertexArray(VAO_bombcoin[i]);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                }
            }
            if (p1.tasks_done != 2)
            {
                glm::mat4 test4 = glm::mat4(1.0f);
                glm::mat4 translate4 = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
                glm::mat4 MVP4 = test4 * translate4;
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP4[0][0]);

                glBindVertexArray(0);
                glBindVertexArray(VAO4);
                glDrawArrays(GL_LINES, 0, 2);
            }
            if (p1.torch == 1)
            {
                glm::mat4 test6 = glm::mat4(1.0f);
                glm::mat4 translate6 = glm::translate(glm::vec3(dark.translate_X, dark.translate_Y, 0.0f));
                glm::mat4 MVP6 = test6 * translate6;
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP6[0][0]);

                glBindVertexArray(0);
                glBindVertexArray(VAO5);
                glDrawArrays(GL_TRIANGLES, 0, 24);
            }
        }

        // end game methods
        game_won = gameend();
        if (game_won == 1)
        {
            if (p1.torch == 1)
            {
                p1.update_score(int(time(0) - torch_time));
                p1.toggle_torch();
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderText(shaderProgramText, "Game Won !!!", 250.0f, 280.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            std::string point1 = "Points are : ";
            std::string point2 = std::to_string(p1.score);
            std::string points_final = point1 + point2;
            RenderText(shaderProgramText, points_final, 250.0f, 230.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shaderProgramText, "Press Q to Quit", 250.0f, 180.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            // Disable the depth test since the text is being rendered in 2D
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
        if (int(time(0) - setup_time) >= 120 && game_won == 0)
        {
            game_lost = 1;
            if (p1.torch == 1)
            {
                p1.update_score(int(time(0) - torch_time));
                p1.toggle_torch();
            }
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderText(shaderProgramText, "Game Lost !!!", 250.0f, 280.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            RenderText(shaderProgramText, "Press Q to Quit", 250.0f, 230.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            // Disable the depth test since the text is being rendered in 2D
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << p1.score << std::endl;

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteVertexArrays(1, &VAO3);
    glDeleteBuffers(1, &VBO3);
    glDeleteVertexArrays(1, &VAO4);
    glDeleteBuffers(1, &VBO4);
    glDeleteVertexArrays(1, &VAO5);
    glDeleteBuffers(1, &VBO5);
    for (int i = 0; i < 10; i++)
    {
        glDeleteVertexArrays(1, &VAO_bombcoin[i]);
        glDeleteBuffers(1, &VBO_bombcoin[i]);
    }
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void keyboardChar(GLFWwindow *window, unsigned int key)
{
    switch (key)
    {
    case 'Q':
    case 'q':
        std::cout << "Quit" << std::endl;
        exit(0);
        break;

    case 'A':
    case 'a':
    {
        int isCollision = collision_maze_p1(3);
        if (isCollision == 0)
        {
            p1.move_X(-1);
            dark.move_X(-1);
        }
        break;
    }

    case 'D':
    case 'd':
    {
        int isCollision = collision_maze_p1(4);
        int end_gate = collision_endgate_p1();
        if (end_gate == 1 && p1.tasks_done != 2)
        {
            break;
        }
        if (isCollision == 0)
        {
            p1.move_X(1);
            dark.move_X(1);
        }
        break;
    }

    case 'W':
    case 'w':
    {
        int isCollision = collision_maze_p1(1);
        if (isCollision == 0)
        {
            p1.move_Y(1);
            dark.move_Y(1);
        }
        break;
    }

    case 'S':
    case 's':
    {
        int isCollision = collision_maze_p1(2);
        // cout << isCollision << endl;
        if (isCollision == 0)
        {
            p1.move_Y(-1);
            dark.move_Y(-1);
        }
        break;
    }
    case 'T':
    case 't':
    {
        p1.toggle_torch();
        if (p1.torch == 1)
        {
            torch_time = time(0);
        }
        else
        {
            p1.update_score(int(time(0) - torch_time));
        }

        break;
    }
    }
}

void RenderText(unsigned int shaderProgramText, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    glUseProgram(shaderProgramText);
    glUniform3f(glGetUniformLocation(shaderProgramText, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO_text);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices_display[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_display), vertices_display); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
