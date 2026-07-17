#include <cmath>
#include "Matrix4x4.h"
#include "Quaternion.h"

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------

// 単位行列で初期化
Matrix4x4::Matrix4x4(void)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Matrix4x4::Matrix4x4(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33)
{
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

// ---------------------------------------------------------------
// 生成
// ---------------------------------------------------------------

// 単位行列
Matrix4x4 Matrix4x4::Identity(void)
{
    return Matrix4x4();
}

// 平行移動行列
// | 1 0 0 tx |
// | 0 1 0 ty |
// | 0 0 1 tz |
// | 0 0 0 1  |
Matrix4x4 Matrix4x4::Translation(const Vector3& pos)
{
    Matrix4x4 mat;
    mat.m[0][3] = pos.x;
    mat.m[1][3] = pos.y;
    mat.m[2][3] = pos.z;
    return mat;
}

// スケール行列
// | sx 0  0  0 |
// | 0  sy 0  0 |
// | 0  0  sz 0 |
// | 0  0  0  1 |
Matrix4x4 Matrix4x4::Scale(const Vector3& scl)
{
    Matrix4x4 mat;
    mat.m[0][0] = scl.x;
    mat.m[1][1] = scl.y;
    mat.m[2][2] = scl.z;
    return mat;
}

// クォータニオンから回転行列を生成
// Unreal の FQuatRotationMatrix に相当
Matrix4x4 Matrix4x4::Rotation(const Quaternion& q)
{
    float x2 = q.x * q.x * 2.0f;
    float y2 = q.y * q.y * 2.0f;
    float z2 = q.z * q.z * 2.0f;
    float xy = q.x * q.y * 2.0f;
    float xz = q.x * q.z * 2.0f;
    float yz = q.y * q.z * 2.0f;
    float wx = q.w * q.x * 2.0f;
    float wy = q.w * q.y * 2.0f;
    float wz = q.w * q.z * 2.0f;

    Matrix4x4 mat;
    mat.m[0][0] = 1.0f - (y2 + z2);
    mat.m[0][1] = xy - wz;
    mat.m[0][2] = xz + wy;
    mat.m[0][3] = 0.0f;

    mat.m[1][0] = xy + wz;
    mat.m[1][1] = 1.0f - (x2 + z2);
    mat.m[1][2] = yz - wx;
    mat.m[1][3] = 0.0f;

    mat.m[2][0] = xz - wy;
    mat.m[2][1] = yz + wx;
    mat.m[2][2] = 1.0f - (x2 + y2);
    mat.m[2][3] = 0.0f;

    mat.m[3][0] = 0.0f;
    mat.m[3][1] = 0.0f;
    mat.m[3][2] = 0.0f;
    mat.m[3][3] = 1.0f;

    return mat;
}

// 位置・回転・スケールを合成した Transform 行列を生成
// 順序: Scale → Rotation → Translation
Matrix4x4 Matrix4x4::Compose(
    const Vector3& pos,
    const Quaternion& rot,
    const Vector3& scl)
{
    Matrix4x4 s = Scale(scl);
    Matrix4x4 r = Rotation(rot);
    Matrix4x4 t = Translation(pos);

    // Scale → Rotation → Translation の順で合成
    return t * r * s;
}

// ---------------------------------------------------------------
// 演算
// ---------------------------------------------------------------

// 行列の乗算
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
    Matrix4x4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------
// 変換
// ---------------------------------------------------------------

// 点を変換する（平行移動あり）
Vector3 Matrix4x4::TransformPosition(const Vector3& pos) const
{
    return Vector3(
        m[0][0] * pos.x + m[0][1] * pos.y + m[0][2] * pos.z + m[0][3],
        m[1][0] * pos.x + m[1][1] * pos.y + m[1][2] * pos.z + m[1][3],
        m[2][0] * pos.x + m[2][1] * pos.y + m[2][2] * pos.z + m[2][3]
    );
}

// ベクトルを変換する（平行移動なし）
Vector3 Matrix4x4::TransformVector(const Vector3& vec) const
{
    return Vector3(
        m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z,
        m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z,
        m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z
    );
}

// ---------------------------------------------------------------
// 分解
// ---------------------------------------------------------------

// 行列から位置を取得
Vector3 Matrix4x4::GetPosition(void) const
{
    return Vector3(m[0][3], m[1][3], m[2][3]);
}

// 行列からスケールを取得
// 各列ベクトルの長さがスケール値
Vector3 Matrix4x4::GetScale(void) const
{
    float sx = Vector3(m[0][0], m[1][0], m[2][0]).Length();
    float sy = Vector3(m[0][1], m[1][1], m[2][1]).Length();
    float sz = Vector3(m[0][2], m[1][2], m[2][2]).Length();
    return Vector3(sx, sy, sz);
}

// 行列からクォータニオンを取得
// スケールを除去してから変換する
Quaternion Matrix4x4::GetRotation(void) const
{
    // スケールを取得して除去
    Vector3 scl = GetScale();

    // スケールが 0 なら単位クォータニオンを返す
    if (scl.x < 1e-6f || scl.y < 1e-6f || scl.z < 1e-6f)
    {
        return Quaternion::Identity();
    }

    // スケールを除去した回転行列を取得
    float r00 = m[0][0] / scl.x;
    float r10 = m[1][0] / scl.x;
    float r20 = m[2][0] / scl.x;

    float r01 = m[0][1] / scl.y;
    float r11 = m[1][1] / scl.y;
    float r21 = m[2][1] / scl.y;

    float r02 = m[0][2] / scl.z;
    float r12 = m[1][2] / scl.z;
    float r22 = m[2][2] / scl.z;

    // 回転行列からクォータニオンに変換
    float trace = r00 + r11 + r22;
    Quaternion q;

    if (trace > 0.0f)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (r21 - r12) * s;
        q.y = (r02 - r20) * s;
        q.z = (r10 - r01) * s;
    }
    else if (r00 > r11 && r00 > r22)
    {
        float s = 2.0f * sqrtf(1.0f + r00 - r11 - r22);
        q.w = (r21 - r12) / s;
        q.x = 0.25f * s;
        q.y = (r01 + r10) / s;
        q.z = (r02 + r20) / s;
    }
    else if (r11 > r22)
    {
        float s = 2.0f * sqrtf(1.0f + r11 - r00 - r22);
        q.w = (r02 - r20) / s;
        q.x = (r01 + r10) / s;
        q.y = 0.25f * s;
        q.z = (r12 + r21) / s;
    }
    else
    {
        float s = 2.0f * sqrtf(1.0f + r22 - r00 - r11);
        q.w = (r10 - r01) / s;
        q.x = (r02 + r20) / s;
        q.y = (r12 + r21) / s;
        q.z = 0.25f * s;
    }

    return q.Normalized();
}