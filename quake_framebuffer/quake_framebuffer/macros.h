#ifndef QUAKE_MACROS_H_
#define QUAKE_MACROS_H_
// this macro handles equality for classes
// just to save my poor poor fingers
#ifndef EQUAL_CMP_OPS_FRIEND
#define EQUAL_CMP_OPS_FRIEND(BASE,MEMBERNAME) \
friend  static inline bool operator==(const BASE& s1, const BASE& s2);\
friend  static inline bool operator==(const BASE& s1, const decltype(BASE::MEMBERNAME)& s2);\
friend  static inline bool operator==(const decltype(BASE::MEMBERNAME)& s1, const BASE& s2);\
friend  static inline bool operator!=(const BASE& s1, const BASE& s2);\
friend  static inline bool operator!=(const BASE& s1, const decltype(BASE::MEMBERNAME)& s2);\
friend  static inline bool operator!=(const decltype(BASE::MEMBERNAME)& s1, const BASE& s2)
#endif

#ifndef EQUAL_CMP_OPS
#define EQUAL_CMP_OPS(BASE,MEMBERNAME) \
static inline bool operator==(const BASE& s1, const BASE& s2) { return s1.MEMBERNAME == s2.MEMBERNAME; }\
static inline bool operator==(const BASE& s1, const decltype(BASE::MEMBERNAME)& s2){ return s1.MEMBERNAME == s2; }\
static inline bool operator==(const decltype(BASE::MEMBERNAME)& s1, const BASE& s2){ return s1 == s2.MEMBERNAME; }\
static inline bool operator!=(const BASE& s1, const BASE& s2) { return !(s1 == s2);}\
static inline bool operator!=(const BASE& s1, const decltype(BASE::MEMBERNAME)& s2) { return !(s1 == s2);}\
static inline bool operator!=(const decltype(BASE::MEMBERNAME)& s1, const BASE& s2) { return !(s1 == s2);}
#endif

#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__)>::type 

#endif