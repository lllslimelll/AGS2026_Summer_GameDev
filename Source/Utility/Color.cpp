// Core/Color.cpp
#include <sstream>
#include <iomanip>
#include "Color.h"
#include "ColorF.h"

Color::Color(unsigned int value)
    : value(value)
{
}

Color::Color(unsigned char r, unsigned char g, unsigned char b)
    : value((r << 16) | (g << 8) | b)
{
}

Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : value((a << 24) | (r << 16) | (g << 8) | b)
{
}

unsigned char Color::R(void) const
{
    return (value >> 16) & 0xFF;
}

unsigned char Color::G(void) const
{
    return (value >> 8) & 0xFF;
}

unsigned char Color::B(void) const
{
    return value & 0xFF;
}

unsigned char Color::A(void) const
{
    return (value >> 24) & 0xFF;
}

ColorF Color::ToColorF(void) const
{
    return
    {
        R() / 255.0f,
        G() / 255.0f,
        B() / 255.0f,
        A() / 255.0f,
    };
}

bool Color::operator==(const Color& other) const
{
    return value == other.value;
}

bool Color::operator!=(const Color& other) const
{
    return !(*this == other);
}

std::string Color::ToString(void) const
{
    std::ostringstream oss;
    oss << "#" << std::hex << std::uppercase
        << std::setw(6) << std::setfill('0') << value;
    return oss.str();
}