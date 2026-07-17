#pragma once
#include <cmath>
#include "Vector3.h"
#include "Matrix4x4.h"

// クォータニオン
class Quaternion
{
public:

    float w; // 実部
    float x; // 虚部 i
    float y; // 虚部 j
    float z; // 虚部 k

    // ---------------------------------------------------------------
    // コンストラクタ
    // ---------------------------------------------------------------

    // デフォルト：単位クォータニオン（回転なし）
    Quaternion(void);

    // w, x, y, z を直接指定
    Quaternion(float w, float x, float y, float z);

    // ---------------------------------------------------------------
    // 生成
    // ---------------------------------------------------------------

    // 単位クォータニオン（回転なし）を返す
    static Quaternion Identity(void);

    // オイラー角（ラジアン）からクォータニオンを生成する
    static Quaternion Euler(float radX, float radY, float radZ);
    static Quaternion Euler(const Vector3& rad);

    // 指定した軸周りに指定角度（ラジアン）回転するクォータニオンを生成する
    static Quaternion AngleAxis(float rad, const Vector3& axis);

    // 前方向ベクトルからクォータニオンを生成する
    // forward: 向きたい方向（正規化済みであること）
    static Quaternion LookRotation(const Vector3& forward);
    static Quaternion LookRotation(
        const Vector3& forward,
        const Vector3& up);

    // from から to への最短回転クォータニオンを生成する
    static Quaternion FromToRotation(
        const Vector3& from,
        const Vector3& to);

    // 行列からクォータニオンを生成する
    Matrix4x4 ToMatrix(void) const;

    // ---------------------------------------------------------------
    // 演算
    // ---------------------------------------------------------------

    // クォータニオンの合成（回転の掛け合わせ）
    // q1 の後に q2 を回転させる
    Quaternion operator*(const Quaternion& other) const;

    // 比較
    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;

    // ---------------------------------------------------------------
    // 回転
    // ---------------------------------------------------------------

    // ベクトルを回転させる
    Vector3 RotateVector(const Vector3& v) const;

    // ---------------------------------------------------------------
    // 変換
    // ---------------------------------------------------------------

    // オイラー角（ラジアン）に変換する
    Vector3 ToEuler(void) const;

    // ---------------------------------------------------------------
    // 正規化
    // ---------------------------------------------------------------

    // 長さ
    float Length(void)        const;
    float LengthSquared(void) const;

    // 正規化したクォータニオンを返す（自分は変わらない）
    Quaternion Normalized(void) const;

    // 自分自身を正規化する
    void Normalize(void);

    // ---------------------------------------------------------------
    // 補間
    // ---------------------------------------------------------------

    // 球面線形補間
    // t: 0.0f = from, 1.0f = to
    static Quaternion Slerp(
        const Quaternion& from,
        const Quaternion& to,
        float t);

    // ---------------------------------------------------------------
    // その他
    // ---------------------------------------------------------------

    // 逆クォータニオン（回転を逆にする）
    Quaternion Inverse(void) const;

    // 内積
    static float Dot(const Quaternion& a, const Quaternion& b);
    float        Dot(const Quaternion& other) const;

    // 2つのクォータニオン間の角度（度）を返す
    static float Angle(const Quaternion& a, const Quaternion& b);

    // 最大角度を制限しながら to に向けて回転する
    // maxDegreesDelta: 1フレームの最大回転角度（度）
    static Quaternion RotateTowards(
        const Quaternion& from,
        const Quaternion& to,
        float maxDegreesDelta);

    // デバッグ用
    bool IsNormalized(float tolerance = 1e-4f) const;

private:

    // 補間（クランプなし）
    static Quaternion SlerpUnclamped(
        const Quaternion& a,
        const Quaternion& b,
        float t);
};