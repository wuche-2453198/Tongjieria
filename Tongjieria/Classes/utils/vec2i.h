#pragma once

namespace cocos2d
{
    class Vec2;
    class Vec3;
}

/*
* @brief 整数二维向量，支持简单的数学运算。
* 已经实现std::hash，可以用作哈希键。
*
* @note Vec2i针对cocos2d::Vec2 实现了自动转换。
* 由于Vec2i基本只在方块中使用，因此转换方法就是转换到对应的方块坐标，即向下取整。
* 
* @tease cocos2d还是太孱弱了，怎么没有完备的数学库，还得自己造轮子。
*/
class Vec2i
{
public:
    Vec2i();
    Vec2i(int x, int y);
    Vec2i(const cocos2d::Vec2& vec2);
    
    ~Vec2i();
    Vec2i operator+(const Vec2i& vec) const;
    Vec2i operator-(const Vec2i& vec) const;
    Vec2i operator-() const;
    Vec2i operator*(const int& n) const;
    Vec2i operator/(const int& n) const;
    friend Vec2i operator*(const int& n, const Vec2i& vec);
    Vec2i& operator+=(const Vec2i& vec);
    Vec2i& operator-=(const Vec2i& vec);
    Vec2i& operator*=(const int& n);
    Vec2i& operator/=(const int& n);
    bool operator==(const Vec2i& vec) const;
    bool operator!=(const Vec2i& vec) const;
    operator cocos2d::Vec2() const;
    operator cocos2d::Vec3() const;
    float dis();

    static size_t vec2ihash(const Vec2i& v);

    const int& get_x() const;
    void set_x(int x);
    const int& get_y() const;
    void set_y(int y);
    int x;
    int y;
};

/*
* @brief 两点间的曼哈顿距离。
* 
* @param p1 第一个点 
* @param p1 第二个点
*/
int manhattanDis(const Vec2i& p1, const Vec2i& p2);

struct Vec2iHash {
    std::size_t operator()(const Vec2i& v) const {
        uint64_t combined =
            (static_cast<uint64_t>(v.x) << 32) |
            static_cast<uint64_t>(v.y);
        return std::hash<uint64_t>()(combined);
    }
};

namespace std {
    template<>
    struct hash<Vec2i> {
        std::size_t operator()(const Vec2i& v) const noexcept {
            std::size_t h1 = std::hash<int32_t>{}(v.x);
            std::size_t h2 = std::hash<int32_t>{}(v.y);
            return h1 ^ (h2 << 1);
        }
    };
}