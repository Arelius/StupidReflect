#ifndef SRFL_H
#define SRFL_H

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
};

#endif //SRFL_H
