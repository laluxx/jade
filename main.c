// TODO automatic linking and compiling
// TODO FLAG to generate the .c files for inspection (by default generate an ELF)
// TODO FLAG to choose the C compiler
// TODO When automatically including libraries NOTE it
// TODO MAYBE automatic return when there is no ";" 
// TODO The main function should automatically take arguments
// TODO Variable to define the number of newlines after the includes
// TODO http use statement
// TODO Work with arrays as lists in fp using HEAD and TAIL or car and cdr
// TODO IMPORTANT automatic .h file creation
// TODO LATER STNTAX double x = x*2
// TODO SYNTAX pattern matching
// FIXME Why does let add one newline between functions?
// TODO IMPORTANT FIX if
// TODO IMPORTANT Emacs jade-mode
// TODO support inline asm
// TODO jade jit REPL && client for emacs
// TODO IMPORTANT Automatic lib linking based on the use statements
// TODO NEXT fix closing curly brace of the while
// TODO IMPORTANT NEXT Get rid of MAX_LINE_LENGTH and work with dyamic lines
// TODO if the main() function is not present treat everything as inside the main function (but use statement)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 8192 // TODO Dynamic memory allocation

size_t indentation = 4; // TODO "-i" or "--indentation"
size_t function_spacing = 1; // TODO "-fs" or "--function-spacing"

int in_function = 0;
int in_main_function = 0;
int is_first_function = 1;
int scope_level = 0;

void print_indentation(FILE *output_file, size_t level) {
    for (size_t i = 0; i < level; i++) {
        fprintf(output_file, " ");
    }
}

#define MAX_DEFERS 100

typedef struct {
    char statements[MAX_DEFERS][MAX_LINE_LENGTH];
    int count;
} DeferStack;

DeferStack deferStacks[MAX_DEFERS];

void push_defer_statement(const char *line) {
    const char *statement_start = line;
    while (*statement_start == ' ' || *statement_start == '\t') {
        statement_start++;
    }

    // Now find the start of the action part after "defer "
    const char *action_start = strstr(statement_start, "defer ");
    if (action_start) {action_start += 6; // Skip past the length of "defer "

        if (deferStacks[scope_level].count < MAX_DEFERS) {
            strncpy(
                    deferStacks[scope_level].statements[deferStacks[scope_level].count],
                    action_start, MAX_LINE_LENGTH - 1);
            deferStacks[scope_level]
                .statements[deferStacks[scope_level].count][MAX_LINE_LENGTH - 1] =
                '\0';
            deferStacks[scope_level].count++;
        }
    }
}

void pop_and_execute_defers(FILE *output_file, int level) {
    int i;
    // NOTE When there are multiple defers in a single block, they are executed in  reverse order.
    for (i = deferStacks[level].count - 1; i >= 0; i--) {
        print_indentation(output_file, level * indentation);
        char *statement = deferStacks[level].statements[i];
        int len = strlen(statement);
        fprintf(output_file, "%s", statement);
    }
    deferStacks[level].count = 0; // Clear the stack for the current scope level
}

const char* map_type(const char* input_type);

const char* map_array_type(const char* element_type, const char* size_spec) {
    static char result[MAX_LINE_LENGTH];
    const char* c_type = map_type(element_type);
    
    if (strcmp(size_spec, "_") == 0) {
        // Size will be inferred from initializer
        snprintf(result, MAX_LINE_LENGTH, "%s", c_type);
        return result;
    } else if (strlen(size_spec) == 0) {
        // Dynamic array - use pointer
        snprintf(result, MAX_LINE_LENGTH, "%s*", c_type);
        return result;
    } else {
        // Fixed size array
        snprintf(result, MAX_LINE_LENGTH, "%s", c_type);
        return result;
    }
}

const char *infer_type_from_value(const char *value);
int is_range_expression(const char* expr);
int expand_range_expression(const char* range_expr, char* expanded, size_t expanded_size, const char** element_type);

const char* infer_array_type(const char* initializer) {
    // Skip opening bracket and whitespace
    const char* ptr = initializer;
    while (*ptr && (*ptr == '[' || *ptr == ' ' || *ptr == '\t')) ptr++;
    
    // Extract content between brackets
    char content[MAX_LINE_LENGTH];
    const char* bracket_end = strrchr(initializer, ']');
    if (bracket_end) {
        int content_len = bracket_end - ptr;
        strncpy(content, ptr, content_len);
        content[content_len] = '\0';
        
        // Check if it's a range expression
        if (is_range_expression(content)) {
            const char* element_type;
            char dummy[MAX_LINE_LENGTH];
            expand_range_expression(content, dummy, MAX_LINE_LENGTH, &element_type);
            return element_type;
        }
    }
    
    // Find first element for regular arrays
    char first_element[MAX_LINE_LENGTH] = {0};
    int i = 0;
    while (*ptr && *ptr != ',' && *ptr != ']' && i < MAX_LINE_LENGTH - 1) {
        if (*ptr != ' ' && *ptr != '\t') {
            first_element[i++] = *ptr;
        }
        ptr++;
    }
    first_element[i] = '\0';
    
    return infer_type_from_value(first_element);
}

int count_array_elements(const char* initializer) {
    const char* ptr = initializer;
    int in_string = 0;
    int in_char = 0;
    
    // Skip opening bracket
    while (*ptr && *ptr != '[') ptr++;
    if (*ptr) ptr++;
    
    // Extract content between brackets
    char content[MAX_LINE_LENGTH];
    const char* bracket_end = strrchr(initializer, ']');
    if (bracket_end) {
        int content_len = bracket_end - ptr;
        strncpy(content, ptr, content_len);
        content[content_len] = '\0';
        
        // Check if it's a range expression
        if (is_range_expression(content)) {
            const char* element_type;
            char dummy[MAX_LINE_LENGTH];
            return expand_range_expression(content, dummy, MAX_LINE_LENGTH, &element_type);
        }
    }
    
    // Count commas + 1 for regular arrays, but be careful about strings and chars
    int count = 0;
    ptr = initializer;
    while (*ptr && *ptr != '[') ptr++; // Reset to start of content
    if (*ptr) ptr++;
    
    while (*ptr && *ptr != ']') {
        if (*ptr == '"' && !in_char) {
            in_string = !in_string;
        } else if (*ptr == '\'' && !in_string) {
            in_char = !in_char;
        } else if (*ptr == ',' && !in_string && !in_char) {
            count++;
        }
        ptr++;
    }
    
    // If we found any content, add 1 for the last element
    ptr = initializer;
    while (*ptr && (*ptr == '[' || *ptr == ' ' || *ptr == '\t')) ptr++;
    if (*ptr && *ptr != ']') {
        count++;
    }
    
    return count;
}

int is_range_expression(const char* expr) {
    return strstr(expr, "..") != NULL;
}

int expand_range_expression(const char* range_expr, char* expanded, size_t expanded_size, const char** element_type) {
    char start_str[MAX_LINE_LENGTH], end_str[MAX_LINE_LENGTH];
    char *range_op = strstr(range_expr, "..");
    
    if (!range_op) return 0;
    
    // Extract start and end values
    int start_len = range_op - range_expr;
    strncpy(start_str, range_expr, start_len);
    start_str[start_len] = '\0';
    strcpy(end_str, range_op + 2);
    
    // Trim whitespace
    char *start_trimmed = start_str;
    while (*start_trimmed == ' ' || *start_trimmed == '\t') start_trimmed++;
    char *end_trimmed = end_str;
    while (*end_trimmed == ' ' || *end_trimmed == '\t') end_trimmed++;
    
    // Remove trailing whitespace from end
    char *end_ptr = end_trimmed + strlen(end_trimmed) - 1;
    while (end_ptr > end_trimmed && (*end_ptr == ' ' || *end_ptr == '\t')) {
        *end_ptr = '\0';
        end_ptr--;
    }
    
    expanded[0] = '\0';
    
    // Handle character ranges like 'a'..'z'
    if (start_trimmed[0] == '\'' && end_trimmed[0] == '\'') {
        char start_char = start_trimmed[1];
        char end_char = end_trimmed[1];
        *element_type = "char";
        
        strcat(expanded, "{");
        int count = 0;
        for (char c = start_char; c <= end_char; c++) {
            if (count > 0) strcat(expanded, ", ");
            char temp[10];
            sprintf(temp, "'%c'", c);
            strcat(expanded, temp);
            count++;
        }
        strcat(expanded, "}");
        return count;
    }
    
    // Handle integer ranges like 1..100
    else if (strchr(start_trimmed, '.') == NULL && strchr(end_trimmed, '.') == NULL) {
        int start_val = atoi(start_trimmed);
        int end_val = atoi(end_trimmed);
        *element_type = "int";
        
        strcat(expanded, "{");
        int count = 0;
        for (int i = start_val; i <= end_val; i++) {
            if (count > 0) strcat(expanded, ", ");
            char temp[20];
            sprintf(temp, "%d", i);
            strcat(expanded, temp);
            count++;
        }
        strcat(expanded, "}");
        return count;
    }
    
    // Handle float ranges like 1.0..100.0
    else {
        float start_val = atof(start_trimmed);
        float end_val = atof(end_trimmed);
        *element_type = "float";
        
        strcat(expanded, "{");
        int count = 0;
        for (float f = start_val; f <= end_val; f += 1.0f) {
            if (count > 0) strcat(expanded, ", ");
            char temp[30];
            sprintf(temp, "%.1f", f);
            strcat(expanded, temp);
            count++;
        }
        strcat(expanded, "}");
        return count;
    }
}

void convert_array_initializer(const char* jade_init, char* c_init, size_t c_init_size) {
    const char* src = jade_init;
    char* dst = c_init;
    size_t remaining = c_init_size - 1; // Leave space for null terminator
    
    // Skip leading whitespace
    while (*src && (*src == ' ' || *src == '\t')) src++;
    
    if (*src == '[') {
        src++; // Skip the opening bracket
        
        // Extract the content between brackets
        char content[MAX_LINE_LENGTH];
        const char* content_start = src;
        const char* bracket_end = strrchr(jade_init, ']');
        if (bracket_end) {
            int content_len = bracket_end - content_start;
            strncpy(content, content_start, content_len);
            content[content_len] = '\0';
            
            // Check if it's a range expression
            if (is_range_expression(content)) {
                const char* element_type;
                expand_range_expression(content, c_init, c_init_size, &element_type);
                return;
            }
        }
        
        // Regular array - convert brackets to braces
        *dst++ = '{';
        remaining--;
        
        // Copy the content, replacing the closing bracket with closing brace
        while (*src && remaining > 1) {
            if (*src == ']') {
                *dst++ = '}';
                remaining--;
                src++;
            } else {
                *dst++ = *src++;
                remaining--;
            }
        }
    } else {
        // If it doesn't start with '[', copy as-is (fallback)
        strncpy(c_init, jade_init, c_init_size - 1);
    }
    
    *dst = '\0';
}

// Convert Jade array initializer [1,2,3] to C array initializer {1,2,3}
void parse_variable_declaration(const char* line, FILE* output_file, int indentation_level) {
    char var_name[MAX_LINE_LENGTH];
    char var_type[MAX_LINE_LENGTH] = "";
    char var_value[MAX_LINE_LENGTH] = "";
    char array_size[MAX_LINE_LENGTH] = "";
    int is_array = 0;
    
    char *trimmed_line = (char*)line;
    while (*trimmed_line == ' ' || *trimmed_line == '\t') {
        trimmed_line++;
    }
    
    // Check if it's an array declaration with [size] syntax
    if (strstr(trimmed_line, "[") && strchr(trimmed_line, ']')) {
        char *bracket_start = strchr(trimmed_line, '[');
        char *bracket_end = strchr(bracket_start, ']');
        
        // Make sure this is part of the variable declaration, not the initializer
        char *equals_pos = strchr(trimmed_line, '=');
        if (!equals_pos || bracket_start < equals_pos) {
            is_array = 1;
            
            // Extract array size specification
            strncpy(array_size, bracket_start + 1, bracket_end - bracket_start - 1);
            array_size[bracket_end - bracket_start - 1] = '\0';
            
            // Parse variable name (everything between "let " and "[")
            sscanf(trimmed_line, "let %[^[]", var_name);
            int len = strlen(var_name);
            if (len > 0 && var_name[len - 1] == ':') {
                var_name[len - 1] = '\0';
            }
            
            // Check what comes after the bracket
            char *after_bracket = bracket_end + 1;
            
            // Skip whitespace
            while (*after_bracket == ' ' || *after_bracket == '\t') {
                after_bracket++;
            }
            
            // Handle different syntax patterns
            if (*after_bracket == ':') {
                // Pattern: let arr:[10]:type or let arr:[10]:type = [...]
                after_bracket++; // Skip the ':'
                
                // Skip more whitespace
                while (*after_bracket == ' ' || *after_bracket == '\t') {
                    after_bracket++;
                }
                
                // Extract type and optional value
                char temp_line[MAX_LINE_LENGTH];
                strcpy(temp_line, after_bracket);
                
                char *equals_in_temp = strchr(temp_line, '=');
                if (equals_in_temp) {
                    // Has assignment: let arr:[10]:type = [...]
                    *equals_in_temp = '\0'; // Split at equals
                    
                    // Get type (before equals)
                    sscanf(temp_line, "%s", var_type);
                    
                    // Get value (after equals)
                    sscanf(equals_in_temp + 1, " %[^\n]", var_value);
                } else {
                    // No assignment: let arr:[10]:type;
                    // Remove semicolon and get type
                    char *semicolon = strchr(temp_line, ';');
                    if (semicolon) *semicolon = '\0';
                    sscanf(temp_line, "%s", var_type);
                }
                
            } else if (*after_bracket && !(*after_bracket == ';' || *after_bracket == '\n')) {
                // Pattern: let arr:[10]type or let arr:[10]type = [...]
                char temp_line[MAX_LINE_LENGTH];
                strcpy(temp_line, after_bracket);
                
                char *equals_in_temp = strchr(temp_line, '=');
                if (equals_in_temp) {
                    // Has assignment
                    *equals_in_temp = '\0';
                    
                    // Get type (before equals, trim whitespace)
                    char *type_start = temp_line;
                    while (*type_start == ' ' || *type_start == '\t') type_start++;
                    char *type_end = type_start + strlen(type_start) - 1;
                    while (type_end > type_start && (*type_end == ' ' || *type_end == '\t')) type_end--;
                    *(type_end + 1) = '\0';
                    strcpy(var_type, type_start);
                    
                    // Get value (after equals)
                    sscanf(equals_in_temp + 1, " %[^\n]", var_value);
                } else {
                    // No assignment: let arr:[10]type;
                    char *semicolon = strchr(temp_line, ';');
                    if (semicolon) *semicolon = '\0';
                    
                    // Get type, trim whitespace
                    char *type_start = temp_line;
                    while (*type_start == ' ' || *type_start == '\t') type_start++;
                    char *type_end = type_start + strlen(type_start) - 1;
                    while (type_end > type_start && (*type_end == ' ' || *type_end == '\t')) type_end--;
                    *(type_end + 1) = '\0';
                    strcpy(var_type, type_start);
                }
                
            } else {
                // Pattern: let arr:[10]; or let arr:[10] = [...]
                char *rest_of_line = after_bracket;
                while (*rest_of_line == ' ' || *rest_of_line == '\t') rest_of_line++;
                
                if (*rest_of_line == '=') {
                    // Has assignment, infer type
                    sscanf(rest_of_line + 1, " %[^\n]", var_value);
                    // Type will be inferred later
                }
                // If no assignment and no type, var_type remains empty (will cause error, but that's expected)
            }
        }
    }
    
    // Check for simple array assignment without [size] syntax: let harr = [1, 2, 3];
    if (!is_array && strchr(trimmed_line, '=')) {
        char temp_value[MAX_LINE_LENGTH];
        if (strstr(trimmed_line, ":")) {
            sscanf(trimmed_line, "let %[^:]:%s = %[^\n]", var_name, var_type, temp_value);
        } else {
            sscanf(trimmed_line, "let %s = %[^\n]", var_name, temp_value);
        }
        
        // Check if the value starts with '[' (array initializer)
        char *trimmed_value = temp_value;
        while (*trimmed_value == ' ' || *trimmed_value == '\t') trimmed_value++;
        
        if (*trimmed_value == '[') {
            is_array = 1;
            strcpy(var_value, temp_value);
            strcpy(array_size, "_"); // Inferred size
            
            // If no type was specified, infer it from the array elements
            if (strlen(var_type) == 0) {
                strcpy(var_type, infer_array_type(var_value));
            }
        }
    }
    
    // If not an array, handle as regular variable
    if (!is_array) {
        if (strstr(trimmed_line, ":")) {
            sscanf(trimmed_line, "let %[^:]:%s = %[^\n]", var_name, var_type, var_value);
        } else {
            sscanf(trimmed_line, "let %s = %[^\n]", var_name, var_value);
            strcpy(var_type, infer_type_from_value(var_value));
        }
    }
    
    // Remove any trailing semicolon
    char *end_ptr = var_value + strlen(var_value) - 1;
    if (*end_ptr == ';') {
        *end_ptr = '\0';
    }
    
    if (is_array) {
        const char *element_type;
        int array_count = 0;
        
        // Determine element type
        if (strlen(var_type) > 0) {
            element_type = map_type(var_type);
        } else {
            element_type = infer_array_type(var_value);
        }
        
        // Convert Jade array initializer to C array initializer
        char c_initializer[MAX_LINE_LENGTH];
        if (strlen(var_value) > 0) {
            convert_array_initializer(var_value, c_initializer, MAX_LINE_LENGTH);
        }
        
        // Print indentation
        print_indentation(output_file, indentation_level);
        
        if (strcmp(array_size, "_") == 0) {
            // Inferred size array: int arr[] = {1, 2, 3};
            if (strlen(var_value) > 0) {
                fprintf(output_file, "%s %s[] = %s;\n", element_type, var_name, c_initializer);
            } else {
                fprintf(output_file, "%s %s[];\n", element_type, var_name);
            }
        } else if (strlen(array_size) == 0) {
            // Dynamic array - count elements and create fixed size
            array_count = count_array_elements(var_value);
            if (strlen(var_value) > 0) {
                fprintf(output_file, "%s %s[%d] = %s;\n", element_type, var_name, array_count, c_initializer);
            } else {
                fprintf(output_file, "%s %s[];\n", element_type, var_name);
            }
        } else {
            // Fixed size array: int arr[10] = {1, 2, 3};
            if (strlen(var_value) > 0) {
                fprintf(output_file, "%s %s[%s] = %s;\n", element_type, var_name, array_size, c_initializer);
            } else {
                fprintf(output_file, "%s %s[%s] = {0};\n", element_type, var_name, array_size);
            }
        }
    } else {
        // Regular variable
        const char *c_type = strlen(var_type) > 0
            ? map_type(var_type)
            : infer_type_from_value(var_value);
        
        print_indentation(output_file, indentation_level);
        if (strlen(var_value) > 0) {
            fprintf(output_file, "%s %s = %s;\n", c_type, var_name, var_value);
        } else {
            fprintf(output_file, "%s %s;\n", c_type, var_name);
        }
    }
}

//TODO IMPORTANT Automatically include stdint.h if needed 
const char* map_type(const char* input_type) {
    if (strcmp(input_type, "i8"    ) == 0) return "int8_t";
    if (strcmp(input_type, "u8"    ) == 0) return "uint8_t";
    if (strcmp(input_type, "i16"   ) == 0) return "int16_t";
    if (strcmp(input_type, "u16"   ) == 0) return "uint16_t";
    if (strcmp(input_type, "i32"   ) == 0) return "int";
    if (strcmp(input_type, "u32"   ) == 0) return "uint32_t";
    if (strcmp(input_type, "i64"   ) == 0) return "int64_t";
    if (strcmp(input_type, "u64"   ) == 0) return "uint64_t";
    if (strcmp(input_type, "i128"  ) == 0) return "int128_t";
    if (strcmp(input_type, "u128"  ) == 0) return "uint128_t";
    if (strcmp(input_type, "f32"   ) == 0) return "float";
    if (strcmp(input_type, "f64"   ) == 0) return "double";
    if (strcmp(input_type, "int"   ) == 0) return "int";
    if (strcmp(input_type, "float" ) == 0) return "float";
    if (strcmp(input_type, "char"  ) == 0) return "char";
    if (strcmp(input_type, "str"   ) == 0) return "char*";
    if (strcmp(input_type, "bool"  ) == 0) return "bool";
    return input_type; // _->
}


void gather_prototypes(FILE *input_file, char prototypes[][MAX_LINE_LENGTH], int *prototype_count) {
    char line[MAX_LINE_LENGTH];
    fseek(input_file, 0, SEEK_SET);
    while (fgets(line, sizeof(line), input_file)) {
        // Check for function definition
        if (strstr(line, "fn ") && strstr(line, "{")) {
            char function_name[MAX_LINE_LENGTH];
            char parameters[MAX_LINE_LENGTH];
            char return_type[MAX_LINE_LENGTH] = "void";

            // Parse function signature
            sscanf(line, "fn %s", function_name);
            char *start_params = strchr(line, '(') + 1;
            char *end_params = strchr(line, ')');
            strncpy(parameters, start_params, end_params - start_params);
            parameters[end_params - start_params] = '\0';

            // Check for return type
            char *return_type_pos = strstr(line, "->");
            if (return_type_pos != NULL) {
                sscanf(return_type_pos, "-> %s", return_type);
                strcpy(return_type, map_type(return_type));
            }

            // Remove the "fn " prefix and the return type part
            strtok(function_name, "(");

            // Translate parameters
            char translated_params[MAX_LINE_LENGTH] = "";
            char *param = strtok(parameters, ",");
            while (param != NULL) {
                char param_name[MAX_LINE_LENGTH];
                char param_type[MAX_LINE_LENGTH];
                sscanf(param, "%[^:]:%s", param_name, param_type);
                strcat(translated_params, map_type(param_type));
                strcat(translated_params, " ");
                strcat(translated_params, param_name);
                param = strtok(NULL, ",");
                if (param != NULL) {
                    strcat(translated_params, ", ");
                }
            }

            // NOTE Exclude main function from prototypes
            if (strcmp(function_name, "main") != 0) {
                // Store the prototype
                snprintf(prototypes[*prototype_count], MAX_LINE_LENGTH, "%s %s(%s);", return_type, function_name, translated_params);
                (*prototype_count)++;
            }
        }
    }
}

void gather_includes(FILE *input_file, char includes[][MAX_LINE_LENGTH], int *include_count, int *uses_stdio) {
    char line[MAX_LINE_LENGTH];
    fseek(input_file, 0, SEEK_SET);
    while (fgets(line, sizeof(line), input_file)) {
        if (strstr(line, "use ")) {
            char library[MAX_LINE_LENGTH];
            int is_commented = (strstr(line, "//") == line);

            sscanf(line, is_commented ? "// use %s" : "use %s", library);

            if (strcmp(library, "stdio") == 0) {
                snprintf(includes[*include_count], MAX_LINE_LENGTH, "%s#include <stdio.h>", is_commented ? "/* " : "");
                if (!is_commented) {
                    (*uses_stdio) = 1;
                }
            } else if (strcmp(library, "stdlib") == 0) {
                snprintf(includes[*include_count], MAX_LINE_LENGTH, "%s#include <stdlib.h>", is_commented ? "/* " : "");
            } else {
                // Replace dots with slashes for libraries like GLFW.glfw3
                for (char *p = library; *p; p++) {
                    if (*p == '.') {
                        *p = '/';
                    }
                }
                snprintf(includes[*include_count], MAX_LINE_LENGTH, "%s#include <%s.h>", is_commented ? "/* " : "", library);
            }
            if (is_commented) {
                strcat(includes[*include_count], " */");
            }
            (*include_count)++;
        }
    }
}

void handle_loop(FILE *output_file, int indentation_level) {
    for (int i = 0; i < indentation_level; i++) {
        fprintf(output_file, " ");
    }

    fprintf(output_file, "while (1) {\n");
}


void handleType(FILE *input_file, FILE *output_file, const char *line) {
    char type_name[MAX_LINE_LENGTH];
    char fields[MAX_LINE_LENGTH * 10]; // Assuming a maximum of 10 lines of fields for simplicity
    int is_recursive = 0;

    // Check if it's a recursive type
    if (strstr(line, "rec type ")) {
        is_recursive = 1;
        sscanf(line, "rec type %s {", type_name);
    } else {
        sscanf(line, "type %s {", type_name);
    }

    // Initialize fields
    fields[0] = '\0';

    // Read the lines containing the fields
    while (fgets(line, MAX_LINE_LENGTH, input_file)) {
        // Check for the closing brace
        if (strstr(line, "}")) {
            break;
        }

        // Append the line to the fields string
        strcat(fields, line);
    }

    // Print the typedef struct
    if (is_recursive) {
        fprintf(output_file, "typedef struct %s {\n", type_name);
    } else {
        fprintf(output_file, "typedef struct {\n");
    }

    // Print fields with indentation
    char field_name[MAX_LINE_LENGTH];
    char field_type[MAX_LINE_LENGTH];
    char *field_line = strtok(fields, "\n");
    while (field_line != NULL) {
        if (sscanf(field_line, " %[^:]:%s", field_name, field_type) == 2) {
            // Remove any trailing characters from field_type
            field_type[strcspn(field_type, " \t\n\r;")] = '\0';
            const char *mapped_type = map_type(field_type);
            fprintf(output_file, "    %s %s;\n", mapped_type, field_name);
        }
        field_line = strtok(NULL, "\n");
    }

    // Close the struct
    fprintf(output_file, "} %s;\n\n", type_name);
}

const char *infer_type_from_value(const char *value) {
    // STRING
    if (strchr(value, '\"')) {
        return "char*";
    }
    // CHAR
    if (strchr(value, '\'')) {
        return "char";
    }
    // FLOAT
    if (strchr(value, '.')) {
        return "float";
    }
    // INT
    char *endptr;
    strtol(value, &endptr, 10);
    while (*endptr == ' ')
        endptr++; // Skip trailing whitespace

    if (*endptr == '\0' || *endptr == ';') {
        return "int";
    }

    // BOOL NOTE we check bool last so we can write false or true in a comment
    if (strstr(value, "true") || strstr(value, "false")) {
        return "bool";
    }

    // _->
    return "ERROR";
}


void transpile(const char* input_filename, const char* output_filename) {
    FILE *input_file = fopen(input_filename, "r");
    FILE *output_file = fopen(output_filename, "w");

    if (!input_file) {
        fprintf(stderr, "Could not open input file %s\n", input_filename);
        exit(1);
    }

    if (!output_file) {
        fprintf(stderr, "Could not open output file %s\n", output_filename);
        exit(1);
    }


    // Gather includes
    char includes[100][MAX_LINE_LENGTH]; // Assuming a maximum of 100 includes
    int include_count = 0;
    int uses_stdio = 0; // Flag to track usage of printf
    gather_includes(input_file, includes, &include_count, &uses_stdio);

    // Gather prototypes
    char prototypes[100] [MAX_LINE_LENGTH]; // Assuming a maximum of 100 prototypes
    int prototype_count = 0;

    gather_prototypes(input_file, prototypes, &prototype_count);

    // Check for usage of printf (println!) that is not commented out
    fseek(input_file, 0, SEEK_SET);
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), input_file)) {
        char* trimmed_line = line;
        while (*trimmed_line == ' ' || *trimmed_line == '\t') {
            trimmed_line++;
        }
        if (strstr(trimmed_line, "println!(") && !strstr(trimmed_line, "//")) {
            uses_stdio = 1;
            break;
        }
    }

    // Write the necessary includes at the top of the file
    for (int i = 0; i < include_count; i++) {
        fprintf(output_file, "%s\n", includes[i]);
    }
    if (uses_stdio) {
        int already_included = 0;
        for (int i = 0; i < include_count; i++) {
            if (strstr(includes[i], "#include <stdio.h>") && !strstr(includes[i], "/*")) {
                already_included = 1;
                break;
            }
        }
        if (!already_included) {
            fprintf(output_file, "#include <stdio.h>\n");
        }
    }
    if (prototype_count > 0 || uses_stdio) {
        fprintf(output_file, "\n");
    }

    // Write the prototypes at the top of the file
    for (int i = 0; i < prototype_count; i++) {
        fprintf(output_file, "%s\n", prototypes[i]);
    }
    if (prototype_count > 0) {
        for (size_t i = 0; i < function_spacing; i++) {
            fprintf(output_file, "\n");
        }
    }

    // Transpile the actual code
    fseek(input_file, 0, SEEK_SET);
    /* int in_function = 0; */
    /* int in_main_function = 0; */
    /* int is_first_function = 1; */
    /* int scope_level = 0; */

    while (fgets(line, sizeof(line), input_file)) {
        // Skip the commented "use" lines
        if (strstr(line, "use ")) {
            continue;
        }

        if (strstr(line, "defer ")) {
            // Capture the action following the 'defer' keyword
            char *action = strchr(line, ' ') + 1;
            push_defer_statement(action);
            continue; // Skip writing the defer line to the output
        }

        // Handle type definitions
        if (strstr(line, "type ")) {
            handleType(input_file, output_file, line);
            continue;
        }

        // Check for function definition
        if (strstr(line, "fn ") && strstr(line, "{")) {
            char function_name[MAX_LINE_LENGTH];
            char parameters[MAX_LINE_LENGTH];
            char return_type[MAX_LINE_LENGTH] = "void"; // NOTE for functions

            // Parse function signature
            sscanf(line, "fn %s", function_name);
            char *start_params = strchr(line, '(') + 1;
            char *end_params = strchr(line, ')');
            strncpy(parameters, start_params, end_params - start_params);
            parameters[end_params - start_params] = '\0';

            // Check for return type
            char *return_type_pos = strstr(line, "->");
            if (return_type_pos != NULL) {
                sscanf(return_type_pos, "-> %s", return_type);
                strcpy(return_type, map_type(return_type));
            }

            // Remove the "fn " prefix and the return type part
            strtok(function_name, "(");

            // Translate parameters
            char translated_params[MAX_LINE_LENGTH] = "";
            char *param = strtok(parameters, ",");
            while (param != NULL) {
                char param_name[MAX_LINE_LENGTH];
                char param_type[MAX_LINE_LENGTH];
                sscanf(param, "%[^:]:%s", param_name, param_type);
                strcat(translated_params, map_type(param_type));
                strcat(translated_params, " ");
                strcat(translated_params, param_name);
                param = strtok(NULL, ",");
                if (param != NULL) {
                    strcat(translated_params, ", ");
                }
            }

            if (!is_first_function) {
                for (size_t i = 0; i < function_spacing; i++) {
                    fprintf(output_file, "\n");
                }
            }
            is_first_function = 0;

            fprintf(output_file, "%s %s(%s) {\n", return_type, function_name, translated_params);
            in_function = 1;
            scope_level = 1; // Reset scope level for a new function

            // Check if this is the main function
            if (strcmp(function_name, "main") == 0) {
                in_main_function = 1;
            }
            continue;
        }

        /* Check for end of function or scope */
        if (in_function && strstr(line, "}")) {
            pop_and_execute_defers(output_file, scope_level);
            scope_level--;
            if (scope_level == 0) { // End of function
                fprintf(output_file, "}\n");
                in_function = 0;
                if (in_main_function) {
                    in_main_function = 0;
                }
            } else {
                print_indentation(output_file, scope_level * indentation);
                fprintf(output_file, "}\n");
            }
            continue;
        }

        // Replace macros with their C equivalents
        if (strstr(line, "println!(")) {
            // Check if the line is commented
            char* trimmed_line = line;
            while (*trimmed_line == ' ' || *trimmed_line == '\t') {
                trimmed_line++;
            }
            int is_commented = strstr(trimmed_line, "//") && (strstr(trimmed_line, "//") < strstr(trimmed_line, "println!("));

            // Find the start and end of the macro content
            char* start_content = strchr(line, '(') + 1;
            char* end_content = strrchr(line, ')');
            if (start_content && end_content && start_content < end_content) {
                char content[MAX_LINE_LENGTH];
                strncpy(content, start_content, end_content - start_content);
                content[end_content - start_content] = '\0';

                // Remove surrounding double quotes from content if present
                if (content[0] == '"' && content[strlen(content) - 1] == '"') {
                    content[strlen(content) - 1] = '\0';
                    memmove(content, content + 1, strlen(content));
                }

                // Write the translated printf statement, commented if necessary
                print_indentation(output_file, scope_level * indentation);
                if (is_commented) {
                    fprintf(output_file, "// ");
                }
                fprintf(output_file, "printf(\"%s\\n\");\n", content);
            }
            continue;
        }


        // Transpile variable declarations
        char *trimmed_line = line;
        while (*trimmed_line == ' ' || *trimmed_line == '\t') {
            trimmed_line++;
        }

        if (strstr(trimmed_line, "let ")) {
            parse_variable_declaration(line, output_file, in_function ? (trimmed_line - line) : 0);
            continue;
        }

       
        // Transpile if statements
        if (strstr(trimmed_line, "if ")) {
            int is_commented = strstr(line, "//") != NULL;
            char condition[MAX_LINE_LENGTH];
            sscanf(trimmed_line, "if %[^\n]", condition);

            // Remove any trailing brace from the condition
            char* brace_ptr = strrchr(condition, '{');
            if (brace_ptr) {
                *brace_ptr = '\0';
            }

            // Add parentheses around the condition if not present
            if (condition[0] != '(') {
                char temp[MAX_LINE_LENGTH];
                snprintf(temp, MAX_LINE_LENGTH, "(%s)", condition);
                strcpy(condition, temp);
            }

            // Remove trailing space before the closing parenthesis
            char* space_ptr = strrchr(condition, ' ');
            if (space_ptr && *(space_ptr + 1) == ')') {
                *space_ptr = ')';
                *(space_ptr + 1) = '\0';
            }

            // Translate the if statement
            if (in_function) {
                print_indentation(output_file, trimmed_line - line);
            }
            if (is_commented) {
                fprintf(output_file, "// if %s {\n", condition);
                scope_level++;
                while (fgets(line, sizeof(line), input_file)) {
                    trimmed_line = line;
                    while (*trimmed_line == ' ' || *trimmed_line == '\t') {
                        trimmed_line++;
                    }
                    if (strstr(trimmed_line, "//")) {
                        print_indentation(output_file, trimmed_line - line);
                        fprintf(output_file, "%s", trimmed_line);
                        if (strstr(trimmed_line, "}")) {
                            scope_level--;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } else {
                fprintf(output_file, "if %s {\n", condition);
                scope_level++;
            }
            continue;
        }

        // Transpile loop keyword
        if (strstr(trimmed_line, "loop {")) {
            handle_loop(output_file, scope_level * indentation);
            scope_level++;
            continue;
        }

        // Copy comments directly
        if (strstr(trimmed_line, "//")) {
            if (in_function) {
                print_indentation(output_file, trimmed_line - line);
            }
            fprintf(output_file, "%s", trimmed_line);
            continue;
        }

        // Copy function body lines if inside any function
        if (in_function) {
            print_indentation(output_file, trimmed_line - line);
            fprintf(output_file, "%s", trimmed_line);
        } else {
            // Copy lines outside of any function without indentation
            fprintf(output_file, "%s", line);
        }
    }

    fclose(input_file);
    fclose(output_file);
}

#include <stdbool.h>


char* replace_extension(const char* filename, const char* new_extension) {
    char *new_filename = malloc(strlen(filename) + strlen(new_extension) + 1);
    strcpy(new_filename, filename);
    char *dot = strrchr(new_filename, '.');
    if (dot) {
        *dot = '\0';
    }
    strcat(new_filename, new_extension);
    return new_filename;
}

#include <sys/stat.h>

void jadeInit(const char *project_name) {
    // Use dynamic allocation for path to avoid fixed buffer size
    size_t path_len = strlen(project_name) + 20; // Extra space for "/main.jade" etc.
    char *path = malloc(path_len);
    
    mkdir(project_name, 0777); // Create directory with read/write/execute for all

    // Create and write main.jade
    snprintf(path, path_len, "%s/main.jade", project_name);
    FILE *file = fopen(path, "w");
    if (file != NULL) {
        fprintf(file, "#include <stdio.h>\n");
        fprintf(file, "fn main() -> i32 {\n    printf(\"Hello world\\n\");\n    return 0;\n}\n");
        fclose(file);
    }

    // Create and write Makefile with proper tab indentation
    snprintf(path, path_len, "%s/Makefile", project_name);
    file = fopen(path, "w");
    if (file != NULL) {
        fprintf(file, "# Makefile for %s project\n", project_name);
        fprintf(file, "# Generated by JADE compiler\n\n");
        fprintf(file, "CC = cc\n");
        fprintf(file, "CFLAGS = -Wall -Wextra -std=c99\n");
        fprintf(file, "TARGET = %s\n", project_name);
        fprintf(file, "SOURCE = main.c\n\n");
        
        // Rules with proper tab indentation
        fprintf(file, "all: $(TARGET)\n\n");
        fprintf(file, "$(TARGET): $(SOURCE)\n");
        fprintf(file, "\t$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)\n\n");
        
        fprintf(file, "$(SOURCE): main.jade\n");
        fprintf(file, "\tjade main.jade\n\n");
        
        fprintf(file, "clean:\n");
        fprintf(file, "\trm -f $(TARGET) $(SOURCE)\n\n");
        
        fprintf(file, "run: $(TARGET)\n");
        fprintf(file, "\t./$(TARGET)\n\n");
        
        fprintf(file, ".PHONY: all clean run\n");
        
        fclose(file);
    }
    
    free(path);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [<args>]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "init") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s init <project_name>\n", argv[0]);
            return 1;
        }
        jadeInit(argv[2]);
        printf("Project %s initialized.\n", argv[2]);
    } else {
        char *input_filename = argv[1];
        char *output_filename = replace_extension(input_filename, ".c");
        transpile(input_filename, output_filename);
        printf("Transpiled %s to %s\n", input_filename, output_filename);
        free(output_filename);
    }

    return 0;
}
