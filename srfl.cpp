#include "srfl.h"

#pragma warning(disable:4390)

srfl_type* srfl_types_head = 0;

#define SRFL_GET_TYPE(typ) get_meta_##typ()

#define SRFL_DECLARE_TYPE(typ) \
srfl_type* get_meta_##typ();

#define SRFL_DEFINE_TYPE(typ) \
void init_meta_##typ(srfl_type* type, srfl_member* member, srfl_info** ppinfo, typ* dummy); \
srfl_type* get_meta_ ## typ () { \
    static srfl_type type = {}; \
    static srfl_type* ptype = nullptr; \
    if(ptype) return ptype; \
    ptype = &type; \
    type.next = 0; \
    type.name = #typ; \
    type.size = sizeof(typ); \
    type.members = 0; \
    type.infos = 0; \
    type.next = srfl_types_head; \
    srfl_types_head = &type; \
    init_meta_##typ(&type, type.members, &type.infos, (typ*)0);   \
    return ptype;  \
} \
static srfl_type* _static_meta_##typ = get_meta_##typ(); \
void init_meta_##typ(srfl_type* type, srfl_member* member, srfl_info** ppinfo, typ* dummy)

#define SRFL_MEMBER(typ, nam) { \
    static srfl_member smember; \
    member = &smember; \
    member->next = 0; \
    member->type = SRFL_GET_TYPE(typ); \
    member->name = #nam; \
    member->offset = (size_t)&dummy->nam; \
    member->infos = 0; \
    member->next = type->members; \
    type->members = member; \
} if(srfl_info** ppinfo = &member->infos)

#define SRFL_INFO(ky, vl) { \
    static srfl_info info; \
    info.key = #ky; \
    info.value = #vl; \
    info.next = *ppinfo; \
    *ppinfo = &info; \
}
      
SRFL_DEFINE_TYPE(int) {}   

SRFL_DEFINE_TYPE(float) {}

struct Foo {
    float x;
    int y;
};

SRFL_DEFINE_TYPE(Foo) {
    SRFL_MEMBER(float, x);
    SRFL_MEMBER(int, y) {
        SRFL_INFO(range, 0 - 2);
        SRFL_INFO(notes, Funny);
    }
    SRFL_INFO(author, indy);
}

#include <stdio.h>

void srfl_print_info(srfl_info* info, int indent = 0) {
    printf("%*sInfo \"%s\" : \"%s\"\n", indent, "", info->key, info->value);
}

void srfl_print_infos(srfl_info* infos, int indent = 0) {
    while(infos) {
        srfl_print_info(infos, indent);
        infos = infos->next;
    }
}

void srfl_print_member(srfl_member* member, int indent = 0) {
    printf("%*sMember %s(%s) offset(%zu)\n", indent, "", member->name, member->type->name, member->offset);
    if(member->infos) {
        srfl_print_infos(member->infos, indent + 2);
        printf("%*sEnd Member %s\n", indent, "", member->name);
    }
}

void srfl_print_members(srfl_member* members, int indent = 0) {
    while(members) {
        srfl_print_member(members, indent);
        members = members->next;
    }
}

void srfl_print_type(srfl_type* type, int indent = 0) {
    printf("%*sBegin %s (%zu)\n", indent, "", type->name, type->size);
    srfl_print_members(type->members, indent + 2);
    srfl_print_infos(type->infos, indent + 2);
    printf("%*sEnd %s\n", indent, "", type->name);
}

void srfl_print_types(srfl_type* types, int indent = 0) {
    while(types) {
        srfl_print_type(types, indent);
        types = types->next;
    }
}

int main(int argc, const char** argv) {
    srfl_print_types(srfl_types_head);
    return 0;
}
