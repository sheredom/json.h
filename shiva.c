#include <json.h>
#include <stdio.h>

void main()
{
    const char json[] = "{\"a\" : true, \"b\" : [false, null, \"foo\"], \"c\" : false}";
    struct json_value_s* root = json_parse(json, strlen(json));
    
    struct json_object_s* object = json_value_as_object(root);
    assert(object != NULL);
    assert(object->length == 3);
    
    struct json_object_element_s* a = object->start;
    
    struct json_string_s* a_name = a->name;
    assert(0 == strcmp(a_name->string, "a"));
    assert(a_name->string_size == strlen("a"));
    
    struct json_value_s* a_value = a->value;
    assert(json_value_is_true(a_value));
    
    struct json_object_element_s* b = a->next;
    assert(b->next == NULL);
    
    struct json_string_s* b_name = b->name;
    assert(0 == strcmp(b_name->string, "b"));
    assert(b_name->string_size == strlen("b"));
    
    struct json_array_s* array = json_value_as_array(b->value);
    assert(array->length == 3);
    
    struct json_array_element_s* b_1st = array->start;
    
    struct json_value_s* b_1st_value = b_1st->value;
    assert(json_value_is_false(b_1st_value));
    
    struct json_array_element_s* b_2nd = b_1st->next;
    
    struct json_value_s* b_2nd_value = b_2nd->value;
    assert(json_value_is_null(b_2nd_value));
    
    struct json_array_element_s* b_3rd = b_2nd->next;
    assert(b_3rd->next == NULL);
    
    struct json_string_s* string = json_value_as_string(b_3rd->value);
    assert(string != NULL);
    assert(0 == strcmp(string->string, "foo"));
    assert(string->string_size == strlen("foo"));
    
    struct json_object_element_s* c = b->next;
    assert(c->next == NULL);
    
    struct json_string_s* c_name = c->name;
    assert(0 == strcmp(c_name->string, "c"));
    assert(c_name->string_size == strlen("c"));
    
    struct json_value_s* c_value = c->value;
    assert(json_value_is_false(c_value));
    
    /* Don't forget to free the one allocation! */
    free(root);
}
