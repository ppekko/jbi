#ifndef JBI_CONFIG_H
#define JBI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// All of these are self explanatory 
const char* jbi_get_variable(const char* var_name);
void jbi_handle_variable(const char* var_name, const char* var_value);

#ifdef __cplusplus
}
#endif

#endif // JBI_CONFIG_H
