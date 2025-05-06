
#include <algorithm>
inline Q_Value negate(Q_Value const& q){
    //standard negation
    return Q_Value(1 - q.v);
}
inline Q_Value t_norm(Q_Value const& q1, Q_Value const& q2){
    //min norm
    return Q_Value(std::min(q1.v, q2.v));
}
inline Q_Value s_norm(Q_Value const& q1, Q_Value const& q2){
    //max norm
    return Q_Value(std::max(q1.v, q2.v));
}

template<int id, typename T1, typename T2>
auto eq_substitution(T1 v1, T2 v2) {
    //normal non-fuzzy behaviour
	return Q_Value(v1 == v2);
}
template<int id, typename T1, typename T2>
auto lt_substitution(T1 v1, T2 v2) {
	//normal non-fuzzy behaviour
	return Q_Value(v1 < v2);
}
