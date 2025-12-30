
#include <iostream>
#include <type_traits>

class Q_Value {
	public:
    double v;
	
	constexpr Q_Value() noexcept : v{1} {}
	constexpr Q_Value(double v) noexcept : v(v) {}
	
	explicit operator bool() const {
		return v > 0;
	}

    friend std::ostream& operator<<(std::ostream& os, Q_Value const& q) {
        return os << "Q("<<q.v<<")";
    }
    
    bool operator<(Q_Value const& rhs) const {
        return v < rhs.v;
    }
};
#include "fuzzy_impl.hpp"

inline Q_Value operator&&(bool lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(bool lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(bool lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(bool lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&&(Q_Value const& lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(Q_Value const& lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(Q_Value const& lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(Q_Value const& lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&&(Q_Value const& lhs, bool rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(Q_Value const& lhs, bool rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(Q_Value const& lhs, bool rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(Q_Value const& lhs, bool rhs) {
	return s_norm(lhs, rhs);
}

template<int Id, int... Ids>
struct Contains;

template<int Id>
struct Contains<Id> : std::false_type {};

template<int Id, int First, int... Rest>
struct Contains<Id, First, Rest...> : Contains<Id, Rest...> {};

template<int Id, int... Rest>
struct Contains<Id, Id, Rest...> : std::true_type {};

template<typename Seq, int Id>
struct AddIfNotPresent;

template<int Id, int... Ids>
struct AddIfNotPresent<std::integer_sequence<int, Ids...>, Id> {
    using type = std::conditional_t<Contains<Id, Ids...>::value,
                                    std::integer_sequence<int, Ids...>,
                                    std::integer_sequence<int, Ids..., Id>>;
};

template<typename Seq1, typename Seq2>
struct CombineUnique;

template<int FirstId2, int... Ids1, int... Ids2>
struct CombineUnique<std::integer_sequence<int, Ids1...>, std::integer_sequence<int, FirstId2, Ids2...>> {
    using type = typename CombineUnique<
		typename AddIfNotPresent<
			std::integer_sequence<int, Ids1...>,
			FirstId2
		>::type,
		std::integer_sequence<int, Ids2...>
	>::type;
};

template<int... Ids>
struct CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int>> {
    using type = std::integer_sequence<int, Ids...>;
};

template<typename T, int... Ids>
struct ClockVar;

template<typename T, typename Seq>
struct ClockVarT;

template<typename T, int... Ids>
struct ClockVarT<T, std::integer_sequence<int, Ids...>> {
    using type = ClockVar<T, Ids...>;
};

template<typename T, typename Seq>
using ClockVar_t = typename ClockVarT<T, Seq>::type;

template<typename T, int... Ids>
struct ClockVar {
    T value;

    ClockVar() : value{} {}
    ClockVar(T val) : value(val) {}

	/// arithmetic: ClockVar @ ClockVar

    template<typename OtherT, int... OtherIds>
    auto operator+(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value + other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value + other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator-(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value - other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value - other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator*(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value * other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value * other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator/(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value / other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value / other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator%(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value % other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value % other.value);
    }

	/// arithmetic: ClockVar @ builtin
	
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator+(U other) const {
        using ResultType = decltype(value + other);
        return ClockVar<ResultType, Ids...>(value + other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator-(U other) const {
        using ResultType = decltype(value - other);
        return ClockVar<ResultType, Ids...>(value - other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator*(U other) const {
        using ResultType = decltype(value * other);
        return ClockVar<ResultType, Ids...>(value * other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator/(U other) const {
        using ResultType = decltype(value / other);
        return ClockVar<ResultType, Ids...>(value / other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator%(U other) const {
        using ResultType = decltype(value % other);
        return ClockVar<ResultType, Ids...>(value % other);
    }
	
    auto operator-() const {
        return ClockVar<T, Ids...>(-value);
    }

    friend std::ostream& operator<<(std::ostream& os, const ClockVar& w) {
		#ifdef DBG_INFO
        os << "Value: " << w.value << ", Type: "<<typeid(T).name()<<", IDs: { ";
        ((os << Ids << " "), ...);
        os << "}";
        return os;
		#else
		return os << w.value;
		#endif
    }
	
	/// predicates: ClockVar @ ClockVar
	
    template<typename OtherT, int... OtherIds>
    auto operator==(ClockVar<OtherT, OtherIds...> const& other) const {
        return (eq_substitution<Ids>(value, other.value) || ...) || (eq_substitution<OtherIds>(value, other.value) || ...); 
    }
    template<typename OtherT, int... OtherIds>
    auto operator<(ClockVar<OtherT, OtherIds...> const& other) const {
        return (lt_substitution<Ids>(value, other.value) || ...) || (lt_substitution<OtherIds>(value, other.value) || ...); 
    }
    template<typename OtherT, int... OtherIds>
    auto operator<=(ClockVar<OtherT, OtherIds...> const& other) const {
        return *this < other || *this == other; 
    }
    template<typename OtherT, int... OtherIds>
    auto operator>(ClockVar<OtherT, OtherIds...> const& other) const {
        return other < *this;
    }
    template<typename OtherT, int... OtherIds>
    auto operator>=(ClockVar<OtherT, OtherIds...> const& other) const {
        return other <= *this;
    }
    template<typename OtherT, int... OtherIds>
    auto operator!=(ClockVar<OtherT, OtherIds...> const& other) const {
        return !(*this == other);
    }
	
	/// predicates: ClockVar @ builtin
	
    template<typename U>
    auto operator==(U other) const {
        return *this == ClockVar<U>(other);
    }
    template<typename U>
    auto operator<(U other) const {
        return *this < ClockVar<U>(other);
    }
    template<typename U>
    auto operator<=(U other) const {
        return *this <= ClockVar<U>(other);
    }
    template<typename U>
    auto operator>(U other) const {
        return *this > ClockVar<U>(other);
    }
    template<typename U>
    auto operator>=(U other) const {
        return *this >= ClockVar<U>(other);
    }
    template<typename U>
    auto operator!=(U other) const {
        return *this != ClockVar<U>(other);
    }
};
/// arithmetic: builtin @ ClockVar

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator+(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs + rhs.value);
    return ClockVar<ResultType, Ids...>(lhs + rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator-(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs - rhs.value);
    return ClockVar<ResultType, Ids...>(lhs - rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator*(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs * rhs.value);
    return ClockVar<ResultType, Ids...>(lhs * rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator/(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs / rhs.value);
    return ClockVar<ResultType, Ids...>(lhs / rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator%(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs % rhs.value);
    return ClockVar<ResultType, Ids...>(lhs % rhs.value);
}

/// predicates: builtin @ ClockVar

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator==(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs == lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator<=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs >= lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator>=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs <= lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator<(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs > lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator>(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs < lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator!=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs != lhs;
}
