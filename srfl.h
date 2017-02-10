#ifndef SRFL_H
#define SRFL_H

#ifndef SRFL_SUPPORT_POINTERS
#define SRFL_SUPPORT_POINTERS 0
#endif //SRFL_SUPPORT_POINTERS

#ifndef SRFL_LATE_LINK_POINTERS
#define SRFL_LATE_LINK_POINTERS 1
#endif //SRFL_LATE_LINK_POINTERS

struct srfl_info;
struct srfl_member;
struct srfl_type;

struct srfl_info
{
    srfl_info* next;
    const char* key;
    const char* value;
};

struct srfl_member
{
    srfl_member* next;
    srfl_type* type;
    const char* name;
    size_t offset;
    srfl_info* infos;
};

struct srfl_type
{
    srfl_type* next;
    const char* name;
    size_t size;
    srfl_member* members;
    srfl_info* infos;
#if SRFL_SUPPORT_POINTERS
    srfl_type* ptrType;
#endif //SRFL_SUPPORT_POINTERS
};

#endif //SRFL_H
