#include "IndexBuffer.h"
#include "OpenGl_Header.h"
#include "Player.h"
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include <unistd.h>

typedef enum { NONE, LEFT, RIGHT, UP, DOWN } dir;

GLFWwindow *window;
const GLuint window_width = 1120;
const GLuint window_height = 630;
const GLchar *window_name = "Pong";

const char VertexSource[] = "shader/vertex_shader.glsl";
const char FragmentSource[] = "shader/fragment_shader.glsl";
const char CircleSource[] = "shader/circle_shader.glsl";
Shader *shader = NULL;

const float bg_position[2] = {-1., -1.};
const float bg_color[3] = {0., 0., 0.};
const float bg_width = 2;
const float bg_height = 2;

const float x_offset = 0.1;
const float PlayerWidth = 0.07;
const float PlayerHeight = 0.25;
const float PlayerSpeed = 0.05;
const float PlayerColor[3] = {1., 1., 1.};

const float BallWidth = 0.08;
const float BallHeight = BallWidth * ((float)window_width) / window_height;
const float BallPos_x = 0;
const float BallPos_y = 0;
const float BallSpeed_x = 0.01;
const float BallSpeed_y = 0.01 * ((float)window_width) / window_height;
const float BallColor[3] = {1., 1., 1.};

Ball game_ball = Ball(BallPos_x, BallPos_y, BallSpeed_x, BallSpeed_y, BallWidth, BallHeight);

Player player_left = Player(false, x_offset - 1, -PlayerHeight / 2, 0, PlayerWidth,
                            ((float)window_width) / window_height * PlayerHeight);
Player player_right = Player(false, 1 - x_offset - PlayerWidth, -PlayerHeight / 2, 0, PlayerWidth,
                             ((float)window_width) / window_height * PlayerHeight);

// sollte den square shader bekommen
void drawPlayer(Player player, Shader &shader) {
    shader.bind();

    float position[2] = {player.x, player.y};
    drawRect(position, PlayerColor, player.width, player.height);
}

// sollte den circle shader bekommen
void drawBall(Ball ball, Shader &shader) {
    shader.bind();
    shader.setUniform("rect_info", ball.x, ball.y, ball.width, ball.height);
    shader.setUniform("bg_color", bg_color[0], bg_color[1], bg_color[2], 1.);

    float position[2] = {ball.x, ball.y};
    drawRect(position, BallColor, ball.width, ball.height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {}

void PlayerController(Player &player, int &key_code, int key, int key1, int key2, int action);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    static int key_code_left = 0;
    static int key_code_right = 0;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    PlayerController(player_left, key_code_left, key, GLFW_KEY_W, GLFW_KEY_S, action);
    PlayerController(player_right, key_code_right, key, GLFW_KEY_I, GLFW_KEY_K, action);

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        player_left.control = player_left.control ? false : true;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        player_right.control = player_right.control ? false : true;
    }
}

void PlayerController(Player &player, int &key_code, int key, int key1, int key2, int action) {
    if (!player.control) {
        return;
    }

    if (key == key1 && action == GLFW_PRESS) {
        player.v = PlayerSpeed;
        key_code = key;
    }

    if (key == key2 && action == GLFW_PRESS) {
        player.v = -PlayerSpeed;
        key_code = key;
    }

    if (key == key_code && action == GLFW_RELEASE) {
        player.v = 0;
    }
}

bool hitWall(Ball ball, uint comp) {
    if (((ball.x <= -1 && ball.vx < 0) || (ball.x + ball.width >= 1 && ball.vx > 0)) && comp == 0) {
        return true;
    }

    if (((ball.y <= -1 && ball.vy < 0) || (ball.y + ball.height >= 1 && ball.vy > 0)) &&
        comp == 1) {
        return true;
    }

    return false;
}

bool hitWallPlayer(Player player) {
    if ((player.y <= -1 && player.v < 0) || (player.y + player.height >= 1 && player.v > 0)) {
        return true;
    }

    return false;
}

bool inRange(Ball ball, Player player, uint comp) {
    if ((ball.x + ball.width >= player.x && ball.x <= player.x + player.width) && comp == 0) {
        return true;
    }

    if ((ball.y + ball.height >= player.y && ball.y <= player.y + player.height) && comp == 1) {
        return true;
    }

    return false;
}

dir hitPlayer(Ball ball, Player player, uint comp) {
    if (comp == 0 && inRange(ball, player, 1)) {
        if (ball.x <= player.x + player.width && ball.x >= player.x && ball.vx < 0) {
            return RIGHT;
        }

        if (ball.x + ball.width >= player.x && ball.x + ball.width <= player.x + player.width &&
            ball.vx > 0) {
            return LEFT;
        }
    } else if (comp == 1 && inRange(ball, player, 0)) {
        if (ball.y <= player.y + player.height && ball.y >= player.y && ball.vy < 0) {
            return UP;
        }

        if (ball.y + ball.height >= player.y && ball.y + ball.height <= player.y + player.height &&
            ball.vy > 0) {
            return DOWN;
        }
    }

    return NONE;
}

void BallCollision(Ball &ball, Player player) {
    Ball nextBall = ball;
    nextBall.x += nextBall.vx;
    nextBall.y += nextBall.vy;

    if (!inRange(ball, player, 0)) {
        if (hitPlayer(nextBall, player, 0) == LEFT || hitPlayer(nextBall, player, 0) == RIGHT) {
            ball.vx *= -1;
        }
    }

    if (!inRange(ball, player, 1)) {
        if (hitPlayer(nextBall, player, 1) == UP || hitPlayer(nextBall, player, 1) == DOWN) {
            ball.vy *= -1;
        }
    }

    if (inRange(ball, player, 0) && inRange(ball, player, 1)) {
        ball.vx *= -1;
    }
}

void AiAction(Ball test_ball, Player *player) {
    const uint max_steps = 10000;

    for (uint i = 0; i < max_steps; i++) {
        if (player == &player_left && test_ball.x <= player->x + player->width) {
            break;
        } else if (player == &player_right && test_ball.x + test_ball.width >= player->x) {
            break;
        }

        if (hitWall(test_ball, 0)) {
            test_ball.vx *= -1;
        }

        if (hitWall(test_ball, 1)) {
            test_ball.vy *= -1;
        }

        BallCollision(test_ball, player == &player_left ? player_right : player_left);

        test_ball.x += test_ball.vx;
        test_ball.y += test_ball.vy;
    }

    if (test_ball.y > player->y + player->height) {
        player->v = PlayerSpeed;
    } else if (test_ball.y + test_ball.height < player->y) {
        player->v = -PlayerSpeed;
    }

    float percent = 0.3;
    Player newPlayer = *player;
    newPlayer.width *= percent;
    newPlayer.height *= percent;
    newPlayer.x += (1 - percent) / 2 * player->width;
    newPlayer.y += (1 - percent) / 2 * player->height;

    if (inRange(test_ball, newPlayer, 1)) {
        player->v = 0;
    }
}

int main(int argc, char *argv[]) {
    init_OpenGL(&window, window_width, window_height, window_name);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    VertexArray GlobalVertexArray;
    GlobalVertexArray.bind();

    Shader square(VertexSource, FragmentSource);
    Shader circle(VertexSource, CircleSource);

    shader = &circle;
    square.bind();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        square.bind();
        drawRect(bg_position, bg_color, bg_width, bg_height);

        if (hitWall(game_ball, 0)) {
            if (game_ball.vx < 0) {
                player_left.score += 1;
            } else if (game_ball.vx > 0) {
                player_right.score += 1;
            }

            game_ball.vx *= -1;
        }

        if (hitWall(game_ball, 1)) {
            game_ball.vy *= -1;
        }

        BallCollision(game_ball, player_left);
        BallCollision(game_ball, player_right);

        if (!player_left.control) {
            AiAction(game_ball, &player_left);
        }

        if (!player_right.control) {
            AiAction(game_ball, &player_right);
        }

        game_ball.x += game_ball.vx;
        game_ball.y += game_ball.vy;

        if (!hitWallPlayer(player_left)) {
            player_left.y += player_left.v;
        }

        if (!hitWallPlayer(player_right)) {
            player_right.y += player_right.v;
        }

        drawBall(game_ball, circle);

        drawPlayer(player_left, square);
        drawPlayer(player_right, square);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    printf("score left: %u  ", player_left.score);
    printf("score right: %u\n", player_right.score);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
