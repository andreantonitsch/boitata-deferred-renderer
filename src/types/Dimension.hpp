namespace boitatah{

    template<typename T> struct Dimension2{
        T x;
        T y;
        friend auto operator<=>(const Dimension2&, const Dimension2&) = default;
    };

    template<typename T> struct Dimension3{
        T x;
        T y;
        T z;
        friend auto operator<=>(const Dimension3&, const Dimension3&) = default;
    };


}