#ifndef BOITATAH_VECTOR_HPP
#define BOITATAH_VECTOR_HPP

namespace boitatah
{

    template <typename T>
    struct Vector2
    {
        T x;
        T y;
        friend auto operator<=>(const Vector2 &, const Vector2 &) = default;
    };

    template <typename T>
    struct Vector3
    {
        T x;
        T y;
        T z;
        friend auto operator<=>(const Vector3 &, const Vector3 &) = default;
    };

}

#endif // BOITATAH_VECTOR_HPP