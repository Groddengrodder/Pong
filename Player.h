#ifndef __PLAYER__
#define __PLAYER__

#include "OpenGl_Header.h"

class Player {
    public:
    float x;
    float y;
    float v;

    float width;
    float height;

    uint score;

    bool control;

    Player(bool Control, float _x, float _y, float _v, float Width, float Height) {
        control = Control;

        x = _x;
        y = _y;
        v = _v;

        score = 0;

        width = Width;
        height = Height;
    }
};

class Ball {
    public:
    float x;
    float y;
    float vx;
    float vy;

    float width;
    float height;

    Ball(float _x, float _y, float _vx, float _vy, float Width, float Height) {
        x = _x;
        y = _y;
        vx = _vx;
        vy = _vy;
        width = Width;
        height = Height;
    }
};
#endif
