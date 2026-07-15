// Core/Color.cpp
#include <sstream>
#include <iomanip>
#include "Color.h"
#include "ColorF.h"

// ---------------------------------------------------------------
// íËêîÇÃíËã`
// ---------------------------------------------------------------
const Color Color::WHITE = { 0xFFFFFF };
const Color Color::BLACK = { 0x000000 };
const Color Color::RED = { 0xFF0000 };
const Color Color::GREEN = { 0x00FF00 };
const Color Color::BLUE = { 0x0000FF };
const Color Color::YELLOW = { 0xFFFF00 };
const Color Color::CYAN = { 0x00FFFF };
const Color Color::MAGENTA = { 0xFF00FF };
const Color Color::ORANGE = { 0xFF8000 };
const Color Color::PURPLE = { 0x800080 };
const Color Color::PINK = { 0xFF69B4 };
const Color Color::BROWN = { 0x8B4513 };
const Color Color::GRAY = { 0x808080 };
const Color Color::SILVER = { 0xC0C0C0 };

const Color Color::HP_HIGH = { 0x00FF00 };
const Color Color::HP_MID = { 0xFFFF00 };
const Color Color::HP_LOW = { 0xFF0000 };

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