#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include "cJSON.h"

#define MAX_LEN 100 * 1024 * 1024

char buffer[MAX_LEN];

int count_ifs(json_t *root) {
    int count = 0;
    const char *key;
    json_t *value;

    if (!json_is_object(root)) return 0;

    json_object_foreach(root, key, value) {
        if (json_is_string(value) && strcmp(key, "_nodetype") == 0 && strcmp(json_string_value(value), "If") == 0) {
            count++;
        }

        if (json_is_object(value)) {
            count += count_ifs(value);
        } else if (json_is_array(value)) {
            size_t index;
            json_t *value_element;
            json_array_foreach(value, index, value_element) {
                count += count_ifs(value_element);
            }
        }
    }
    return count;
}

void find_if_inFunc(json_t *root) {
    const char* key; 
    json_t* value;

    json_object_foreach(root, key, value) {
        if (strcmp(key, "ext") == 0 && json_is_array(value)) {
            size_t index; 
            json_t* func_node;
            json_array_foreach(value, index, func_node) {
                if (strcmp(json_string_value(json_object_get(func_node, "_nodetype")), "FuncDef") == 0) {
                    const char* func_name = json_string_value(json_object_get(json_object_get(func_node, "decl"), "name"));
                    json_t* body = json_object_get(func_node, "body");
                    int num_ifs = count_ifs(body);
                    printf("Number of 'if' statements in function '%s': %d\n", func_name, num_ifs);
                }
            }
        }
    }
}

void remove_a(char *str) {
    int i, j = 0;
    int len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] != ' ' && str[i] != '"' && str[i] != ',') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

void func_name_extract(FILE* fs) {
    char str[100] = {0};
    int i = 1;

    while (fgets(str, 100, fs) != NULL) {                                          
        char* name_ptr = strstr(str, "decl"); 
        if (name_ptr != NULL ) {
            fgets(str, 100, fs);
            name_ptr = strstr(str, "_nodetype");
            if (name_ptr != NULL) {
                for (int i = 0; i < 6; i++) fgets(str, 100, fs);
                name_ptr = strstr(str, "name");
                name_ptr = strchr(name_ptr, ' ');
                remove_a(name_ptr);
                printf("Function %d: %s\n", i, name_ptr);
                i += 1;      
            }
        }
    }
}

void func_type(FILE* fp) {
    char str[1000] = {0}; 
    const char str_func[9] = "\"decl\""; 
    const char str_args[5]  = "args"; 
    const char str_type[6] = "names"; 
	
    while(fgets(str, 1000, fp) != NULL) {   
        char* name_ptr = strstr(str, str_func); 
        if (name_ptr != NULL) {
			while(fgets(str,1000,fp) != NULL) {
	        	char* args = strstr(str, str_args); 
    	    	char* isinstance_null = strstr(str, "null");  
    	    	if(args != NULL) { 
					if(isinstance_null != NULL) { 
						while(fgets(str,1000,fp) != NULL) {
		        			char* type_ptr = strstr(str, str_type); 
    		    			if(type_ptr != NULL) {
    	    					fgets(name_ptr,1000,fp); 
    	    					remove_a(name_ptr); 
    	        				printf("Return type: %s\n", name_ptr);
    	        				break;
    	       				}
    	       			}
    	       			break;
					} else { 
						int cnt = 1;
						while(fgets(str,1000,fp) != NULL) {
							char* ptr1 = strstr(str,"{");
							char* ptr2 = strstr(str,"}");
							if(ptr1 != NULL) cnt++;
							if(ptr2 != NULL) cnt--;
							if(cnt == 0) break;
						}
						while(fgets(str,1000,fp) != NULL) {
		        			char* type_ptr = strstr(str,str_type);  
	    	    			if(type_ptr !=NULL){
	        					fgets(name_ptr,1000,fp); 
	        					remove_a(name_ptr); 
	            				printf("Return type: %s\n", name_ptr);
	            				break;
	           				}
	           			} 
					break;
					}	
				}
			}
		}
    }
}

int count_functions(json_t *root) {
    int count = 0;   
    json_t *stack[1000];  
    int top = 0; 
    stack[top++] = root;

    while (top > 0) {
        json_t *node = stack[--top]; 
        if (json_is_object(node)) {
            if (strcmp(json_string_value(json_object_get(node,"_nodetype")), "FuncDef") == 0)
                count++;

            const char *key;
            json_t *value;
            json_object_foreach(node,key,value)
                stack[top++] = value;
        } else if (json_is_array(node)) {
            size_t size=json_array_size(node);
            for(size_t i=0;i<size;i++)
                stack[top++]=json_array_get(node,i); 
        }
    }
    return count;
}

void parameter_parser(FILE* fs){
    fseek(fs , 0 , SEEK_END);
    int size=ftell(fs);
    fseek(fs , 0 , SEEK_SET);
    
	fread(buffer,size ,1 ,fs);

	cJSON *root=cJSON_Parse(buffer);

	cJSON_Delete(root);
}

int main(){
	json_t *root=json_load_file("binary_tree.json",NULL,NULL);

	printf("Function count: %d\n\n",count_functions(root));

	FILE *fp=fopen("binary_tree.json","r");

	func_type(fp);
	func_name_extract(fp);

	fclose(fp);

	find_if_inFunc(root);

	json_decref(root);

	return(1);
}
