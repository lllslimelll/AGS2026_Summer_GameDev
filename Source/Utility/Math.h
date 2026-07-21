#pragma once
#include <cmath>
#include <algorithm>

// 汎用数学関数クラス
class Math
{
public:

    // 円周率
    static constexpr float PI = 3.1415926535897932384626433832795f;
    static constexpr float TWO_PI = PI * 2.0f;
    static constexpr float HALF_PI = PI * 0.5f;

    // ラジアン・度変換係数
    static constexpr float RAD2DEG = 180.0f / PI;
    static constexpr float DEG2RAD = PI / 180.0f;

    // 浮動小数点の誤差許容値
    static constexpr float EPSILON = 1e-6f;
    static constexpr float EPSILON_NORMAL = 1e-15f;

    // -----------------------------------------------------------------
    // 基本演算
    // -----------------------------------------------------------------

    // 値を範囲に収める
    static float Clamp(float v, float min, float max);
    static int   Clamp(int v, int min, int max);

    // 絶対値
    static float Abs(float v);
    static int   Abs(int v);

    // 最大・最小
    static float Max(float a, float b);
    static float Min(float a, float b);
    static int   Max(int a, int b);
    static int   Min(int a, int b);

    // 四捨五入
    static int Round(float v);

    // 累乗
    static float Pow(float base, float exp);

    // 平方根
    static float Sqrt(float v);

    // -----------------------------------------------------------------
    // 補間
    // -----------------------------------------------------------------

    // 線形補間（t: 0.0f?1.0f）
    static float  Lerp(float start, float end, float t);
    static int    Lerp(int start, int end, float t);
    static double Lerp(double start, double end, double t);

    // 角度の線形補間（度）
    static double LerpDeg(double start, double end, double t);

    // ベジェ曲線（float版）
    static float Bezier(float p1, float p2, float p3, float t);

    // -----------------------------------------------------------------
    // 角度
    // -----------------------------------------------------------------

    // 度→ラジアン
    static constexpr float  ToRadian(float  deg) { return deg * DEG2RAD; }
    static constexpr double ToRadian(double deg) { return deg * DEG2RAD; }

    // ラジアン→度
    static constexpr float  ToDegree(float  rad) { return rad * RAD2DEG; }
    static constexpr double ToDegree(double rad) { return rad * RAD2DEG; }

    // 0?360度の範囲に収める
    static double DegIn360(double deg);

    // 0?2πの範囲に収める
    static double RadIn2PI(double rad);

    // 回転が少ない方向を取得（時計回り:1、反時計回り:-1）
    static int DirNearAroundRad(float from, float to);
    static int DirNearAroundDeg(float from, float to);

    // -----------------------------------------------------------------
    // 比較
    // -----------------------------------------------------------------

    // 浮動小数点の近似比較
    static bool NearlyEqual(float a, float b, float tolerance = EPSILON);
    static bool NearlyZero(float v, float tolerance = EPSILON);

private:

    // インスタンス化禁止
    Math(void) = delete;
};