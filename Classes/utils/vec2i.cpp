#include "cocos2d.h"
#include "vec2i.h"

Vec2i::Vec2i() : x(0), y(0) {};
Vec2i::Vec2i(int x, int y) : x(x), y(y) {};
Vec2i::Vec2i(const cocos2d::Vec2& vec2) : x(floor(vec2.x)), y(floor(vec2.y)) {};
Vec2i::~Vec2i() {};
Vec2i Vec2i::operator+(const Vec2i& vec) const { return Vec2i(x + vec.x, y + vec.y); }
Vec2i Vec2i::operator-(const Vec2i& vec) const { return Vec2i(x - vec.x, y - vec.y); }
Vec2i Vec2i::operator-() const { return Vec2i(-x, -y); }
Vec2i Vec2i::operator*(const int& n) const { return Vec2i(x * n, y * n); }
Vec2i Vec2i::operator/(const int& n) const { return Vec2i(x / n, y / n); }
Vec2i operator*(const int& n, const Vec2i& vec) { return Vec2i(vec.x * n, vec.y * n); }

Vec2i& Vec2i::operator+=(const Vec2i& vec) {
    x += vec.x;
    y += vec.y;
    return *this;
}
Vec2i& Vec2i::operator-=(const Vec2i& vec) {
    x -= vec.x;
    y -= vec.y;
    return *this;
}
Vec2i& Vec2i::operator*=(const int& n) {
    x *= n;
    y *= n;
    return *this;
}
Vec2i& Vec2i::operator/=(const int& n) {
    x /= n;
    y /= n;
    return *this;
}
bool Vec2i::operator==(const Vec2i& vec) const {
    return x == vec.x && y == vec.y;
}
bool Vec2i::operator!=(const Vec2i& vec) const {
    return !(*this == vec);
}
Vec2i::operator cocos2d::Vec2() const {
    return cocos2d::Vec2(x, y);
}
Vec2i::operator cocos2d::Vec3() const {
    return cocos2d::Vec3(x, y, 0);
}

size_t Vec2i::vec2ihash(const Vec2i& v)
{
    std::size_t h1 = std::hash<int32_t>{}(v.x);
    std::size_t h2 = std::hash<int32_t>{}(v.y);
    return h1 ^ (h2 << 1);
}

const int& Vec2i::get_x() const { return x; }

void Vec2i::set_x(int x) { this->x = x; }

const int& Vec2i::get_y() const { return y; }

void Vec2i::set_y(int y) { this->x = x; }

int manhattanDis(const Vec2i& p1, const Vec2i& p2)
{
    return (labs(p1.x - p2.x) + labs(p1.y - p2.y));
}