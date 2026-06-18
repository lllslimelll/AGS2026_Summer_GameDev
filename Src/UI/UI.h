#pragma once

class UI
{
public:

    virtual ~UI(void) = default;
    virtual void Init(void) = 0;
    virtual void Draw(void) = 0;
    virtual void Release(void) = 0;
};

