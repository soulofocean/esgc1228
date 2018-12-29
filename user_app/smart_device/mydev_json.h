#ifndef _MYDEV_JSON_H_
#define _MYDEV_JSON_H_

#define MYDEV_JSON_SUCCESS                    0
#define MYDEV_JSON_INPUT_PARAM_IS_NULL        1
#define MYDEV_JSON_OBJECT_KEY_ERROR           2
#define MYDEV_JSON_OBJECT_TYPE_ERROR          3
#define MYDEV_JSON_OBJECT_VALUE_ERROR         4
#define MYDEV_JSON_OBJECT_CREATE_ERROR        5

typedef void * mydev_json_obj;
int     mydev_json_clear(mydev_json_obj obj);
mydev_json_obj mydev_json_parse(char* str);
mydev_json_obj mydev_json_get_object(mydev_json_obj obj, char* key);
mydev_json_obj mydev_json_get_array(mydev_json_obj obj, char* key, int* size);
mydev_json_obj mydev_json_get_array_item(mydev_json_obj list, int index);
mydev_json_obj mydev_json_create_double(double num);
mydev_json_obj mydev_json_create_string(const char *string);
mydev_json_obj mydev_json_create_array(void);
mydev_json_obj mydev_json_create_object(void);
mydev_json_obj mydev_json_create_int(int num);
mydev_json_obj mydev_json_create_longlong( long long num);
mydev_json_obj mydev_json_create_ull(unsigned long long num);
int     mydev_json_get_array_size(mydev_json_obj list, int* size);
int     mydev_json_get_int(mydev_json_obj obj, char* key, int* out);
int     mydev_json_get_double(mydev_json_obj obj, char* key, double* out);
int     mydev_json_get_longlong(mydev_json_obj obj, char* key, long long* out);
int     mydev_json_get_ull(mydev_json_obj obj, char* key, unsigned long long* out);
int     mydev_json_get_string(mydev_json_obj obj, char* key, char* out, unsigned int outlen);
char*   mydev_json_print_tostring(mydev_json_obj obj);
int     mydev_json_delete_key(mydev_json_obj obj, char* key);
int     mydev_json_delete_index(mydev_json_obj list, int index);
int     mydev_json_delete_string(mydev_json_obj list,char* str);
int     mydev_json_add_int(mydev_json_obj obj, char* key, int value);
int     mydev_json_add_double(mydev_json_obj obj, char* key, double value);
int     mydev_json_add_longlong(mydev_json_obj obj, char* key, long long value);
int     mydev_json_add_ull(mydev_json_obj obj, char* key, unsigned long long value);
int     mydev_json_add_string(mydev_json_obj obj, char* key, char* string);
int     mydev_json_add_item(mydev_json_obj obj, char* key, mydev_json_obj item);
// -------------------------------------------------------------------------

#endif
