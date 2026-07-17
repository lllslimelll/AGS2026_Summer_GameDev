#include <cmath>
#include <algorithm>
#include "Quaternion.h"
#include "Vector3.h"

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------

// デフォルト：単位クォータニオン（回転なし）
Quaternion::Quaternion(void)
    : w(1.0f), x(0.0f), y(0.0f), z(0.0f)
{
}

// w, x, y, z を直接指定
Quaternion::Quaternion(float w, float x, float y, float z)
    : w(w), x(x), y(y), z(z)
{
}

// ---------------------------------------------------------------
// 生成
// ---------------------------------------------------------------

// 単位クォータニオン（回転なし）
Quaternion Quaternion::Identity(void)
{
    return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

// オイラー角（ラジアン）からクォータニオンを生成
// 回転順序: X → Y → Z
Quaternion Quaternion::Euler(float radX, float radY, float radZ)
{
    // 各軸の半角の cos / sin を計算
    float cx = cosf(radX * 0.5f);
    float sx = sinf(radX * 0.5f);
    float cy = cosf(radY * 0.5f);
    float sy = sinf(radY * 0.5f);
    float cz = cosf(radZ * 0.5f);
    float sz = sinf(radZ * 0.5f);

    // XYZ 順に合成
    return Quaternion(
        cx * cy * cz + sx * sy * sz,
        sx * cy * cz + cx * sy * sz,
        cx * sy * cz - sx * cy * sz,
        cx * cy * sz - sx * sy * cz
    ).Normalized();
}

Quaternion Quaternion::Euler(const Vector3& rad)
{
    return Euler(rad.x, rad.y, rad.z);
}

// 指定した軸周りに指定角度（ラジアン）回転する
Quaternion Quaternion::AngleAxis(float rad, const Vector3& axis)
{
    // 軸が零ベクトルなら単位クォータニオンを返す
    if (axis.IsNearlyZero()) return Identity();

    Vector3 n = axis.Normalized();
    float   s = sinf(rad * 0.5f);

    return Quaternion(
        cosf(rad * 0.5f),
        n.x * s,
        n.y * s,
        n.z * s
    );
}

// 前方向ベクトルからクォータニオンを生成
Quaternion Quaternion::LookRotation(const Vector3& forward)
{
    return LookRotation(forward, Vector3::UP);
}

Quaternion Quaternion::LookRotation(
    const Vector3& forward,
    const Vector3& up)
{
    Vector3 f = forward.Normalized();
    Vector3 r = Vector3::Cross(up, f).Normalized();
    Vector3 u = Vector3::Cross(f, r);

    // 回転行列から直接クォータニオンを計算
    float m00 = r.x, m01 = r.y, m02 = r.z;
    float m10 = u.x, m11 = u.y, m12 = u.z;
    float m20 = f.x, m21 = f.y, m22 = f.z;

    float trace = m00 + m11 + m22;
    Quaternion q;

    if (trace > 0.0f)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (m12 - m21) * s;
        q.y = (m20 - m02) * s;
        q.z = (m01 - m10) * s;
    }
    else if (m00 > m11 && m00 > m22)
    {
        float s = 2.0f * sqrtf(1.0f + m00 - m11 - m22);
        q.w = (m12 - m21) / s;
        q.x = 0.25f * s;
        q.y = (m01 + m10) / s;
        q.z = (m02 + m20) / s;
    }
    else if (m11 > m22)
    {
        float s = 2.0f * sqrtf(1.0f + m11 - m00 - m22);
        q.w = (m20 - m02) / s;
        q.x = (m01 + m10) / s;
        q.y = 0.25f * s;
        q.z = (m12 + m21) / s;
    }
    else
    {
        float s = 2.0f * sqrtf(1.0f + m22 - m00 - m11);
        q.w = (m01 - m10) / s;
        q.x = (m02 + m20) / s;
        q.y = (m12 + m21) / s;
        q.z = 0.25f * s;
    }

    return q.Normalized();
}

// from から to への最短回転クォータニオンを生成
Quaternion Quaternion::FromToRotation(
    const Vector3& from,
    const Vector3& to)
{
    Vector3 f = from.Normalized();
    Vector3 t = to.Normalized();

    float dot = Vector3::Dot(f, t);

    // 同じ方向 → 回転なし
    if (dot >= 1.0f - 1e-6f) return Identity();

    // 逆方向 → 任意の軸で180度回転
    if (dot <= -1.0f + 1e-6f)
    {
        // f と直交する軸を見つける
        Vector3 axis = Vector3::Cross(Vector3::RIGHT, f);
        if (axis.IsNearlyZero())
        {
            axis = Vector3::Cross(Vector3::UP, f);
        }
        return AngleAxis(3.14159265f, axis.Normalized());
    }

    // 通常の計算
    Vector3 axis = Vector3::Cross(f, t);
    float   angle = acosf(dot);
    return AngleAxis(angle, axis);
}

Matrix4x4 Quaternion::ToMatrix(void) const
{
    return Matrix4x4::Rotation(*this);
}

// ---------------------------------------------------------------
// 演算
// ---------------------------------------------------------------

// クォータニオンの合成（回転の掛け合わせ）
Quaternion Quaternion::operator*(const Quaternion& other) const
{
    return Quaternion(
        w * other.w - x * other.x - y * other.y - z * other.z,
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w
    );
}

bool Quaternion::operator==(const Quaternion& other) const
{
    return w == other.w && x == other.x
        && y == other.y && z == other.z;
}

bool Quaternion::operator!=(const Quaternion& other) const
{
    return !(*this == other);
}

// ---------------------------------------------------------------
// 回転
// ---------------------------------------------------------------

// ベクトルを回転させる
// q * v * q^-1 の計算を最適化した実装
// Unreal の RotateVector に相当
Vector3 Quaternion::RotateVector(const Vector3& v) const
{
    // q * (0, v) * q^-1 を展開した高速実装
    Vector3 qv(x, y, z);
    Vector3 t = Vector3::Cross(qv, v) * 2.0f;
    return v + t * w + Vector3::Cross(qv, t);
}

// ---------------------------------------------------------------
// 変換
// ---------------------------------------------------------------

// オイラー角（ラジアン）に変換
Vector3 Quaternion::ToEuler(void) const
{
    // X 軸回転（Pitch）
    float sinX = 2.0f * (w * x + y * z);
    float cosX = 1.0f - 2.0f * (x * x + y * y);
    float eulerX = atan2f(sinX, cosX);

    // Y 軸回転（Yaw）
    float sinY = 2.0f * (w * y - z * x);
    float eulerY;
    if (fabsf(sinY) >= 1.0f)
    {
        // ジンバルロック
        eulerY = copysignf(3.14159265f * 0.5f, sinY);
    }
    else
    {
        eulerY = asinf(sinY);
    }

    // Z 軸回転（Roll）
    float sinZ = 2.0f * (w * z + x * y);
    float cosZ = 1.0f - 2.0f * (y * y + z * z);
    float eulerZ = atan2f(sinZ, cosZ);

    return Vector3(eulerX, eulerY, eulerZ);
}

// ---------------------------------------------------------------
// 正規化
// ---------------------------------------------------------------

float Quaternion::Length(void) const
{
    return sqrtf(LengthSquared());
}

float Quaternion::LengthSquared(void) const
{
    return w * w + x * x + y * y + z * z;
}

// 正規化したクォータニオンを返す（自分は変わらない）
Quaternion Quaternion::Normalized(void) const
{
    float len = Length();
    if (len < 1e-6f) return Identity();
    float inv = 1.0f / len;
    return Quaternion(w * inv, x * inv, y * inv, z * inv);
}

// 自分自身を正規化する
void Quaternion::Normalize(void)
{
    float len = Length();
    if (len < 1e-6f)
    {
        *this = Identity();
        return;
    }
    float inv = 1.0f / len;
    w *= inv;
    x *= inv;
    y *= inv;
    z *= inv;
}

// ---------------------------------------------------------------
// 補間
// ---------------------------------------------------------------

// 球面線形補間
Quaternion Quaternion::Slerp(
    const Quaternion& from,
    const Quaternion& to,
    float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return SlerpUnclamped(from, to, t);
}

// クランプなし球面線形補間（内部用）
Quaternion Quaternion::SlerpUnclamped(
    const Quaternion& a,
    const Quaternion& b,
    float t)
{
    // どちらかが零クォータニオンなら単純に返す
    if (a.LengthSquared() < 1e-6f) return b.LengthSquared() < 1e-6f ? Identity() : b;
    if (b.LengthSquared() < 1e-6f) return a;

    float cosHalf = Dot(a, b);

    // 最短経路を選ぶ（内積が負なら片方を反転）
    Quaternion b2 = b;
    if (cosHalf < 0.0f)
    {
        b2 = Quaternion(-b.w, -b.x, -b.y, -b.z);
        cosHalf = -cosHalf;
    }

    // 角度がほぼ0なら線形補間
    if (cosHalf > 0.9995f)
    {
        return Quaternion(
            a.w + t * (b2.w - a.w),
            a.x + t * (b2.x - a.x),
            a.y + t * (b2.y - a.y),
            a.z + t * (b2.z - a.z)
        ).Normalized();
    }

    // 球面線形補間
    float halfAngle = acosf(cosHalf);
    float sinHalfAngle = sinf(halfAngle);
    float ratioA = sinf((1.0f - t) * halfAngle) / sinHalfAngle;
    float ratioB = sinf(t * halfAngle) / sinHalfAngle;

    return Quaternion(
        ratioA * a.w + ratioB * b2.w,
        ratioA * a.x + ratioB * b2.x,
        ratioA * a.y + ratioB * b2.y,
        ratioA * a.z + ratioB * b2.z
    );
}

// ---------------------------------------------------------------
// その他
// ---------------------------------------------------------------

// 逆クォータニオン（回転を逆にする）
// 正規化済みの場合は共役と同じ
Quaternion Quaternion::Inverse(void) const
{
    float lenSq = LengthSquared();
    if (lenSq < 1e-6f) return Identity();
    float inv = 1.0f / lenSq;
    return Quaternion(w * inv, -x * inv, -y * inv, -z * inv);
}

// 内積
float Quaternion::Dot(const Quaternion& a, const Quaternion& b)
{
    return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

float Quaternion::Dot(const Quaternion& other) const
{
    return Dot(*this, other);
}

// 2つのクォータニオン間の角度（度）
float Quaternion::Angle(const Quaternion& a, const Quaternion& b)
{
    float d = Dot(a, b);
    // dot が 1.0 に近いほど角度が小さい
    d = std::clamp(fabsf(d), 0.0f, 1.0f);
    return acosf(d) * 2.0f * (180.0f / 3.14159265f);
}

// 最大角度を制限しながら to に向けて回転する
Quaternion Quaternion::RotateTowards(
    const Quaternion& from,
    const Quaternion& to,
    float maxDegreesDelta)
{
    float angle = Angle(from, to);
    if (angle < 1e-6f) return to;
    float t = std::min(1.0f, maxDegreesDelta / angle);
    return SlerpUnclamped(from, to, t);
}

// 正規化済みか確認（デバッグ用）
bool Quaternion::IsNormalized(float tolerance) const
{
    return fabsf(LengthSquared() - 1.0f) <= tolerance;
}