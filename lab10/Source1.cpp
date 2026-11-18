#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct Point { double x, y; };
struct MedianBases { Point ma, mb, mc; };

double distance(Point p, Point q) {
    return std::hypot(p.x - q.x, p.y - q.y);
}

class Triangle {
public:
    virtual ~Triangle() = default;
    virtual std::array<double, 3> sides() const = 0;
    virtual std::string name() const = 0;
    double perimeter() const {
        const auto s = sides();
        return s[0] + s[1] + s[2];
    }
    virtual double area() const {
        const auto s = sides();
        const double p = perimeter() / 2;
        return std::sqrt(p * (p - s[0]) * (p - s[1]) * (p - s[2]));
    }
    double median(std::size_t side) const {
        if (side > 2) throw std::out_of_range("Side index must be 0, 1 or 2");
        const auto s = sides();
        const double a = s[side];
        const double b = s[(side + 1) % 3];
        const double c = s[(side + 2) % 3];
        return 0.5 * std::sqrt(2 * b * b + 2 * c * c - a * a);
    }
    double midline(std::size_t side) const {
        if (side > 2) throw std::out_of_range("Side index must be 0, 1 or 2");
        return sides()[side] / 2;
    }
    virtual double inradius() const { return 2 * area() / perimeter(); }
    virtual double circumradius() const {
        const auto s = sides();
        return s[0] * s[1] * s[2] / (4 * area());
    }
protected:
    static void validate(double a, double b, double c) {
        if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c)
            || a <= 0 || b <= 0 || c <= 0
            || a + b <= c || a + c <= b || b + c <= a) {
            throw std::invalid_argument("Sides must form a nondegenerate triangle");
        }
    }
};

class TriangleByVertices final : public Triangle {
    Point a_, b_, c_;
public:
    TriangleByVertices(Point a, Point b, Point c) : a_(a), b_(b), c_(c) {
        const auto s = sides();
        validate(s[0], s[1], s[2]);
    }
    explicit TriangleByVertices(const MedianBases& m)
        : TriangleByVertices(
            Point{ m.mb.x + m.mc.x - m.ma.x, m.mb.y + m.mc.y - m.ma.y },
            Point{ m.ma.x + m.mc.x - m.mb.x, m.ma.y + m.mc.y - m.mb.y },
            Point{ m.ma.x + m.mb.x - m.mc.x, m.ma.y + m.mb.y - m.mc.y }) {
    }
    std::array<double, 3> sides() const override {
        return { { distance(b_, c_), distance(c_, a_), distance(a_, b_) } };
    }
    double area() const override {
        return std::abs((b_.x - a_.x) * (c_.y - a_.y) - (b_.y - a_.y) * (c_.x - a_.x)) / 2;
    }
    std::string name() const override { return "Triangle by vertices"; }
    std::array<Point, 3> vertices() const { return { { a_, b_, c_ } }; }
    std::array<Point, 2> midlineEnds(std::size_t side) const {
        if (side > 2) throw std::out_of_range("Side index must be 0, 1 or 2");
        const auto v = vertices();
        const Point opposite = v[side];
        const Point p = v[(side + 1) % 3], q = v[(side + 2) % 3];
        return { { { (opposite.x + p.x) / 2, (opposite.y + p.y) / 2 },
                 { (opposite.x + q.x) / 2, (opposite.y + q.y) / 2 } } };
    }
};

class TriangleBySides : public Triangle {
    std::array<double, 3> sides_;
public:
    TriangleBySides(double a, double b, double c) : sides_{ { a,b,c } } {
        validate(a, b, c);
    }
    std::array<double, 3> sides() const override { return sides_; }
    std::string name() const override { return "Triangle by sides"; }
};

class RightTriangle final : public TriangleBySides {
    double x_, y_;
public:
    RightTriangle(double x, double y)
        : TriangleBySides(x, y, std::hypot(x, y)), x_(x), y_(y) {
    }
    std::string name() const override { return "Right triangle"; }
    double area() const override { return x_ * y_ / 2; }
    double inradius() const override { return (x_ + y_ - std::hypot(x_, y_)) / 2; }
    double circumradius() const override { return std::hypot(x_, y_) / 2; }
};


#include <cassert>
bool near(double x, double y) { return std::abs(x - y) < 1e-9; }
int main() {
    TriangleBySides t(13, 14, 15);
    assert(near(t.area(), 84));
    assert(near(t.inradius(), 4));
    assert(near(t.circumradius(), 8.125));
    TriangleByVertices restored(MedianBases{ { 5,6.5 },{ 2,5.5 },{ 4,3 } });
    auto v = restored.vertices();
    assert(near(v[0].x, 1) && near(v[0].y, 2));
    assert(near(v[1].x, 7) && near(v[1].y, 4));
    assert(near(v[2].x, 3) && near(v[2].y, 9));
    for (std::size_t i = 0;i < 3;++i) {
        const auto ends = restored.midlineEnds(i);
        assert(near(distance(ends[0], ends[1]), restored.midline(i)));
    }
    RightTriangle right(3, 4);
    const Triangle& base = right;
    assert(near(base.area(), 6) && near(base.inradius(), 1) && near(base.circumradius(), 2.5));
    bool bad = false;
    try { TriangleByVertices degenerate({ 0,0 }, { 1,1 }, { 2,2 }); }
    catch (const std::invalid_argument&) { bad = true; }
    assert(bad);
    bad = false;
    try { t.median(3); }
    catch (const std::out_of_range&) { bad = true; }
    assert(bad);
    std::cout << "LR10 independent geometry checks passed\n";
}