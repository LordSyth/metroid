#pragma once
template<typename T> struct xy {
	T x, y;
	xy(T x_ = T(), T y_ = T()) : x(x_), y(y_) {}
	double magnitude() const { return sqrt(x*x + y*y); }
	double distance(const xy& other) const { return sqrt((x - other.x)*(x - other.x) + (y - other.y)*(y - other.y)); }
	bool within(const xy& lowcorner, const xy& highcorner) const {
		return lowcorner.x <= x && x <= highcorner.x && lowcorner.y <= y && y <= highcorner.y;
	}
	xy& operator=(const xy& other) { x = other.x; y = other.y; return *this; }
	xy operator+(const xy& other) const { return xy(x + other.x, y + other.y); }
	xy operator-(const xy& other) const { return xy(x - other.x, y - other.y); }
	xy operator*(const xy& other) const { return xy(x * other.x, y * other.y); }
	xy operator/(const xy& other) const { return xy(x / other.x, y / other.y); }
	xy operator%(const xy& other) const { return xy(x % other.x, y % other.y); }
	xy operator*(const T& other) const { return xy(x * other, y * other); }
	xy operator/(const T& other) const { return xy(x / other, y / other); }
	xy operator%(const T& other) const { return xy(x % other, y % other); }
	xy& operator+=(const xy& other) { x += other.x; y += other.y; return *this; }
	xy& operator-=(const xy& other) { x -= other.x; y -= other.y; return *this; }
	xy& operator*=(const xy& other) { x *= other.x; y *= other.y; return *this; }
	xy& operator/=(const xy& other) { x /= other.x; y /= other.y; return *this; }
	xy& operator%=(const xy& other) { x %= other.x; y %= other.y; return *this; }
	xy& operator*=(const T& other) { x *= other; y *= other; return *this; }
	xy& operator/=(const T& other) { x /= other; y /= other; return *this; }
	xy& operator%=(const T& other) { x %= other; y %= other; return *this; }
};
typedef xy<char> xyc;
typedef xy<short> xys;
typedef xy<int> xyi;
typedef xy<long> xyl;
typedef xy<long long> xyll;
typedef xy<float> xyf;
typedef xy<double> xyd;
