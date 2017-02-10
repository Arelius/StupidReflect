#include "srfl.h"

#pragma warning(disable:4390)

srfl_type* srfl_types_head = 0;

void srfl_link_type(srfl_type* type) {
    type->next = srfl_types_head;
    srfl_types_head = type;
}

#if SRFL_SUPPORT_POINTERS

#include <string.h>

unsigned int srfl_count_ptr(const char* name, size_t len) {
    unsigned int r = 0;
    for(size_t i = len-1; i >= 0 && name[i] == '*'; i--)
        r++;
    return r;
}

// We could create pointer types here on demand if we are happy with dynamic allocation.
srfl_type* srfl_get_pointer_type(srfl_type* type, unsigned int indrs) {
    while(type->ptrType && indrs > 0) {
        type = type->ptrType;
#if SRFL_LATE_LINK_POINTERS
        // This check is valid because we link the base type first, so this will
        // never be null so long as it's linked into the list.
        if(type->next == 0)
            srfl_link_type(type);
#endif //SRFL_LATE_LINK_POINTERS
        indrs--;
    }
    return type;
}

// Should probably build an index.
srfl_type* srfl_get_meta_type(const char* name, size_t len) {
    len = len - 1;
    unsigned int indrs = srfl_count_ptr(name, len);
    if(indrs)
        len = len- indrs;
    srfl_type* rootType = 0;
    for(srfl_type* type = srfl_types_head; type; type = type->next) {
        if((strlen(type->name) == len) && strncmp(type->name, name, len) == 0) {
            rootType = type;
            break;
        }
    }
    rootType = srfl_get_pointer_type(rootType, indrs);
    SRFL_ASSERT(rootType);
    return rootType;
}

#define SRFL_GET_TYPE(typ) srfl_get_meta_type(#typ, sizeof(#typ))

#else //SRFL_SUPPORT_POINTERS

#define SRFL_GET_TYPE(typ) get_meta_##typ()

#endif //SRFL_SUPPORT_POINTERS

void srfl_init_type(srfl_type* type, const char* name, size_t size, bool lateLink = false) {
    type->next = 0;
    type->name = name;
    type->size = size;
    type->members = 0;
    type->infos = 0;
#if SRFL_SUPPORT_POINTERS
    type->ptrType = 0;
#endif //SRFL_SUPPORT_POINTERS
    if(!lateLink)
        srfl_link_type(type);
}

#define STR(a) #a
#define EXP(a) a

#if SRFL_SUPPORT_POINTERS

#define _SRFL_DEFINE_STATICS() \
    static srfl_type type = {}; \
    static srfl_type ptrType = {}; \
    static srfl_type ptrPtrType = {};

#define _SRFL_DEFINE_INIT_TYPES(typ) \
    srfl_init_type(&type, #typ, sizeof(typ)); \
    srfl_init_type(&ptrType, STR(typ ## *), sizeof(typ*), EXP(SRFL_LATE_LINK_POINTERS)); \
    srfl_init_type(&ptrPtrType, STR(typ ## **), sizeof(typ**), EXP(SRFL_LATE_LINK_POINTERS)); \
    ptrType.ptrType = &ptrPtrType; \
    type.ptrType = &ptrType;

#else //SRFL_SUPPORT_POINTERS

#define _SRFL_DEFINE_STATICS() \
    static srfl_type type = {};

#define _SRFL_DEFINE_INIT_TYPES(typ) \
    srfl_init_type(&type, #typ, sizeof(typ));

#endif //SRFL_SUPPORT_POINTERS

#if SRFL_DEFER_MEMBERS
#define INIT_META(typ) \
    type.init_meta = (init_meta_fn)init_meta_##typ;
#else //SRFL_DEFER_MEMBERS
#define INIT_META(typ) \
    init_meta_##typ(&type, type.members, &type.infos, (typ*)0);
#endif //SRFL_DEFER_MEMBERS

#define SRFL_DEFINE_TYPE(typ) \
void init_meta_##typ(srfl_type* type, srfl_member* member, srfl_info** ppinfo, typ* dummy); \
srfl_type* get_meta_ ## typ () { \
    _SRFL_DEFINE_STATICS() \
    static srfl_type* ptype = nullptr; \
    if(ptype) return ptype; \
    ptype = &type; \
    _SRFL_DEFINE_INIT_TYPES(typ) \
    INIT_META(typ) \
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
SRFL_DEFINE_TYPE(char) {}
SRFL_DEFINE_TYPE(size_t) {}

struct Foo {
    Foo* next;
    float x;
    int y;
};

SRFL_DEFINE_TYPE(Foo) {
#if SRFL_SUPPORT_POINTERS
    SRFL_MEMBER(Foo*, next);
#endif //SRFL_SUPPORT_POINTERS
    SRFL_MEMBER(float, x);
    SRFL_MEMBER(int, y) {
        SRFL_INFO(range, 0 - 2);
        SRFL_INFO(notes, Funny);
    }
    SRFL_INFO(author, indy);
}

#if SRFL_SUPPORT_POINTERS && SRFL_REFLECT_OWN_TYPES
SRFL_DEFINE_TYPE(srfl_info) {
    SRFL_MEMBER(srfl_info*, next);
    SRFL_MEMBER(char*, key) {
        //SRFL_INFO(const);
        //SRFL_INFO(null_terminated);
    }
    SRFL_MEMBER(char*, value) {
        //SRFL_INFO(const);
        //SRFL_INFO(null_terminated);
    }
}

SRFL_DEFINE_TYPE(srfl_member) {
    SRFL_MEMBER(srfl_member*, next);
    SRFL_MEMBER(srfl_type*, type);
    SRFL_MEMBER(char*, name) {
        //SRFL_INFO(const);
        //SRFL_INFO(null_terminated);
    }
    SRFL_MEMBER(size_t, offset);
    SRFL_MEMBER(srfl_info*, infos);
}

SRFL_DEFINE_TYPE(srfl_type) {
    SRFL_MEMBER(srfl_type*, next);
    SRFL_MEMBER(char*, name) {
        //SRFL_INFO(const);
        //SRFL_INFO(null_terminated);
    }
    SRFL_MEMBER(srfl_member*, members);
    SRFL_MEMBER(srfl_info*, infos);
#ifdef SRFL_SUPPORT_POINTERS
    SRFL_MEMBER(srfl_type*, ptrType);
#endif //SRFL_SUPPORT_POINTERS
}
#endif

#if SRFL_DEFER_MEMBERS
void srfl_init_meta_types()
{
    srfl_type* type = srfl_types_head;
    while(type) {
        type->init_meta(type, type->members, &type->infos, 0);
        type = type->next;
    }
}
#endif //SRFL_DEFER_MEMBERS

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
#if SRFL_DEFER_MEMBERS
    srfl_init_meta_types();
#endif //SRFL_DEFER_MEMBERS
    srfl_print_types(srfl_types_head);
    return 0;
}
