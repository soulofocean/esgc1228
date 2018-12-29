
#include <stdio.h>
#include <string.h>
#include "cJson.h"

#include "mydev_json.h"

#define CHECK_NULL_PTR(ptr,ret) \
    if(NULL == ptr) return ret;

/*
*   json number maybe int, double, long long, unsigned long long.
*   then,the following process is the same,so,create a macro.
*   template is so good for this scene.
*/
#define JSON_GET_NUMBER(sub_obj,obj,key,out)     \
{   CHECK_NULL_PTR(obj,1);                                      \
    CHECK_NULL_PTR(out,1);                                      \
    if(!key)                                                                    \
        sub_obj = obj;                                                    \
    else{                                                                       \
        sub_obj = cJSON_GetObjectItem(obj,key);      \
        CHECK_NULL_PTR(sub_obj,2);                          \
    }                                                                             \
    if(cJSON_Number != sub_obj->type)                     \
        return 3;                                                            \
}

#define JSON_ADD_NUMBER(obj,key,value,pfunc)     \
{   CHECK_NULL_PTR(obj,1);                                        \
    if(!(cJSON_Array == obj->type || (cJSON_Object == obj->type && key != NULL))) \
        return 3;                                                              \
    cJSON* value_obj = pfunc(value);                            \
    CHECK_NULL_PTR(value_obj,5);                              \
    if(cJSON_Object == obj->type)                              \
        cJSON_AddItemToObject(obj,key,value_obj);    \
    else                                                                        \
        cJSON_AddItemToArray(obj,value_obj);            \
}

mydev_json_obj mydev_json_parse(char* str)
{
    CHECK_NULL_PTR(str,NULL);

    cJSON* obj = cJSON_Parse(str);
    CHECK_NULL_PTR(obj,NULL);

    return obj;
}

int mydev_json_clear(mydev_json_obj obj)
{
    CHECK_NULL_PTR(obj,1);

    cJSON_Delete(obj);
    return 0;
}

mydev_json_obj mydev_json_get_object(mydev_json_obj obj, char* key)
{
    CHECK_NULL_PTR(obj,NULL);
    CHECK_NULL_PTR(key,NULL);

    cJSON* sub_obj = cJSON_GetObjectItem(obj,key);
    CHECK_NULL_PTR(sub_obj,NULL);

    /* array is also available, but maybe it's not a good way. use mydev_json_get_list please. */
    if((cJSON_Object != sub_obj->type) && (cJSON_Array != sub_obj->type))
        return NULL;

    return sub_obj;
}

mydev_json_obj mydev_json_get_array(mydev_json_obj obj, char* key, int* size)
{
    CHECK_NULL_PTR(obj,NULL);
    CHECK_NULL_PTR(key,NULL);

    cJSON* sub_obj = cJSON_GetObjectItem(obj,key);
    CHECK_NULL_PTR(sub_obj,NULL);

    if(cJSON_Array != sub_obj->type)
        return NULL;
    if(NULL != size)
        *size = cJSON_GetArraySize(sub_obj);

    return sub_obj;
}

int mydev_json_get_array_size(mydev_json_obj            list, int* size)
{
    CHECK_NULL_PTR(list,1);
    CHECK_NULL_PTR(size,1);

    *size = cJSON_GetArraySize(list);

    return 0;
}

/*  user need to know the return cJSON ptr is array or object */
mydev_json_obj mydev_json_get_array_item(mydev_json_obj            list, int index)
{
    CHECK_NULL_PTR(list,NULL);

    cJSON* sub_obj = cJSON_GetArrayItem(list,index);
    CHECK_NULL_PTR(sub_obj,NULL);

    return sub_obj;
}

int mydev_json_get_string(mydev_json_obj obj, char* key, char* out, unsigned int outlen)
{
    CHECK_NULL_PTR(obj,1);
    CHECK_NULL_PTR(out,1);

    cJSON* sub_obj = NULL;
    if(!key)
        sub_obj = obj;
    else
    {
        sub_obj = cJSON_GetObjectItem(obj,key);
        CHECK_NULL_PTR(sub_obj,2);
    }

    if(cJSON_String != sub_obj->type)
        return 3;
    CHECK_NULL_PTR(sub_obj->valuestring,4);

    snprintf(out,outlen,"%s",sub_obj->valuestring);
    return 0;
}

int mydev_json_get_double(mydev_json_obj obj, char* key, double* out)
{
    cJSON* sub_obj = NULL;

    JSON_GET_NUMBER(sub_obj,obj,key,out);

    *out = sub_obj->valuedouble;
    return 0;
}

int mydev_json_get_int(mydev_json_obj obj, char* key, int* out)
{
    cJSON* sub_obj = NULL;

    JSON_GET_NUMBER(sub_obj,obj,key,out);

    *out = sub_obj->valueint;
    return 0;
}

int mydev_json_get_longlong(mydev_json_obj obj, char* key, long long* out)
{
    cJSON* sub_obj = NULL;

    JSON_GET_NUMBER(sub_obj,obj,key,out);

    *out = sub_obj->valuelonglong;
    return 0;
}

int mydev_json_get_ull(mydev_json_obj obj, char* key, unsigned long long* out)
{
    cJSON* sub_obj = NULL;

    JSON_GET_NUMBER(sub_obj,obj,key,out);

    *out = sub_obj->valueull;
    return 0;
}

/* return buf ptr need free by user */
char*   mydev_json_print_tostring(mydev_json_obj obj)
{
    return cJSON_PrintUnformatted(obj);
}

int mydev_json_delete_key(mydev_json_obj json_obj, char* key)
{
    cJSON * obj = (cJSON *)json_obj;

    CHECK_NULL_PTR(obj,1);
    CHECK_NULL_PTR(key,1);

    if(cJSON_Object != obj->type)
        return 3;
    cJSON* sub_obj = cJSON_GetObjectItem(obj,key);
    CHECK_NULL_PTR(sub_obj,2);

    cJSON_DeleteItemFromObject(obj,key);
    return 0;
}

int mydev_json_delete_index(mydev_json_obj json_obj, int index)
{
    cJSON * obj = (cJSON *)json_obj;
    CHECK_NULL_PTR(obj,1);

    if(cJSON_Array != obj->type)
        return 3;

    if((index >= cJSON_GetArraySize(obj)) || (index < 0))
        return 2;

    cJSON_DeleteItemFromArray(obj,index);
    return 0;
}

int mydev_json_delete_string(mydev_json_obj json_list,char* str)
{
    cJSON * list = (cJSON *)json_list;

    CHECK_NULL_PTR(list,1);
    CHECK_NULL_PTR(str,1);

    if(cJSON_Array != list->type)
        return 3;

    char       info[100] = {0};
    cJSON*  sub_list = NULL;
    int flag = -1;
    int num  =  cJSON_GetArraySize(list);
    while(num>0)
    {
        num--;
        sub_list = mydev_json_get_array_item(list,num);
        if(mydev_json_get_string(sub_list, NULL, info, sizeof(info)))
            continue;

        if(!strncmp(str,info,sizeof(info)))
        {
            flag = 1;
            break;
        }
    }

    if(1 == flag)
        cJSON_DeleteItemFromArray(list,num);
    else
        return 4;

    return 0;
}

int mydev_json_add_int(mydev_json_obj json_obj, char* key, int value)
{
    cJSON * obj = (cJSON *)json_obj;

    JSON_ADD_NUMBER(obj,key,value,cJSON_CreateLongLong);
    return 0;
}

int mydev_json_add_double(mydev_json_obj json_obj, char* key, double value)
{
    cJSON * obj = (cJSON *)json_obj;
    JSON_ADD_NUMBER(obj,key,value,cJSON_CreateNumber);
    return 0;
}

int mydev_json_add_longlong(mydev_json_obj json_obj, char* key, long long value)
{
    cJSON * obj = (cJSON *)json_obj;
    JSON_ADD_NUMBER(obj,key,value,cJSON_CreateLongLong);
    return 0;
}

int mydev_json_add_ull(mydev_json_obj json_obj, char* key, unsigned long long value)
{
    cJSON * obj = (cJSON *)json_obj;
    JSON_ADD_NUMBER(obj,key,value,cJSON_CreateUll);
    return 0;
}

int mydev_json_add_string(mydev_json_obj json_obj, char* key, char* string)
{
    cJSON * obj = (cJSON *)json_obj;
    CHECK_NULL_PTR(obj,1);
    CHECK_NULL_PTR(string,1);

    if(!(cJSON_Array == obj->type || (cJSON_Object == obj->type && key != NULL)))
        return 3;
    cJSON* str_obj = cJSON_CreateString(string);
    CHECK_NULL_PTR(str_obj,4);

    if(cJSON_Object == obj->type)
        cJSON_AddItemToObject(obj,key,str_obj);
    else
        cJSON_AddItemToArray(obj,str_obj);

    return 0;
}

int mydev_json_add_item(mydev_json_obj json_obj, char* key, mydev_json_obj json_item)
{
    cJSON * obj = (cJSON *)json_obj;
    cJSON * item = (cJSON *)json_item;
    CHECK_NULL_PTR(obj,1);
    CHECK_NULL_PTR(item,1);

    if(!(cJSON_Array == obj->type || (cJSON_Object == obj->type && key != NULL)))
        return 3;

    if(cJSON_Object == obj->type)
        cJSON_AddItemToObject(obj,key,item);
    else
        cJSON_AddItemToArray(obj,item);

    return 0;
}

mydev_json_obj mydev_json_create_double(double num)
{
    return cJSON_CreateNumber(num);
}

mydev_json_obj mydev_json_create_string(const char *string)
{
    return cJSON_CreateString(string);
}

mydev_json_obj mydev_json_create_array(void)
{
    return cJSON_CreateArray();
}

mydev_json_obj mydev_json_create_object(void)
{
    return cJSON_CreateObject();
}

mydev_json_obj mydev_json_create_int(int num)
{
    return cJSON_CreateInt(num);
}

mydev_json_obj mydev_json_create_longlong( long long num)
{
    return cJSON_CreateLongLong(num);
}

mydev_json_obj mydev_json_create_ull(unsigned long long num)
{
    return cJSON_CreateUll(num);
}


