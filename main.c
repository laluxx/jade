// TODO a flag to generate the .c files for inspection (by default generate an ELF)
// TODO a flag to choose the C compiler
// TODO a flag to choose the indentation
// TODO defer capabilities
// TODO automatic return when there is no ";"
// TODO jade stdlib and macros like println!
// TODO The main function should automatically take arguments
// TODO Tooling "jade init name"
// TODO Variable to define the number of newlines after the includes
// TODO Ability to define include
// TODO Type inference
// TODO http use statement
// TODO LATER LLVM optimizations
// TODO LATER comment unused includes linting the genrated c code
// TODO LATER treat arrays as lists using HEAD and TAIL or car and cdr
// TODO SYNTAX pattern matching
// TODO STNTAX double x = x*2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256 // TODO Dynamic memory allocation (If needed)

size_t indentation = 4; // TODO "-i"
size_t function_spacing = 1; // TODO "-fs"

void print_indentation(FILE *output_file, size_t level) {
    for (size_t i = 0; i < level; i++) {
        fprintf(output_file, " ");
    }
}

void gather_prototypes(FILE *input_file, char prototypes[][MAX_LINE_LENGTH], int *prototype_count) {
    char line[MAX_LINE_LENGTH];
    fseek(input_file, 0, SEEK_SET);
    while (fgets(line, sizeof(line), input_file)) {
        // Check for function definition
        if (strstr(line, "fn ") && strstr(line, "{")) {
            char function_name[MAX_LINE_LENGTH];
            char parameters[MAX_LINE_LENGTH];
            char return_type[MAX_LINE_LENGTH] = "int";
            
            // Parse function signature
            sscanf(line, "fn %s", function_name);
            char *start_params = strchr(line, '(') + 1;
            char *end_params = strchr(line, ')');
            strncpy(parameters, start_params, end_params - start_params);
            parameters[end_params - start_params] = '\0';

            if (strstr(line, "-> i32")) {
                strcpy(return_type, "int");
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
                if (strstr(param_type, "i32")) {
                    strcat(translated_params, "int ");
                }
                strcat(translated_params, param_name);
                param = strtok(NULL, ",");
                if (param != NULL) {
                    strcat(translated_params, ", ");
                }
            }

            // Exclude main function from prototypes
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
    
    char prototypes[100][MAX_LINE_LENGTH]; // Assuming a maximum of 100 prototypes
    int prototype_count = 0;
    int uses_stdio = 0; // Flag to track usage of printf

    // Gather includes
    char includes[100][MAX_LINE_LENGTH]; // Assuming a maximum of 100 includes
    int include_count = 0;
    gather_includes(input_file, includes, &include_count, &uses_stdio);

    // Gather prototypes
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
    int in_function = 0;
    int in_main_function = 0;
    int is_first_function = 1;

    while (fgets(line, sizeof(line), input_file)) {
        // Skip the commented "use" lines
        if (strstr(line, "// use ")) {
            continue;
        }

        // Check for function definition
        if (strstr(line, "fn ") && strstr(line, "{")) {
            char function_name[MAX_LINE_LENGTH];
            char parameters[MAX_LINE_LENGTH];
            char return_type[MAX_LINE_LENGTH] = "int";
            
            // Parse function signature
            sscanf(line, "fn %s", function_name);
            char *start_params = strchr(line, '(') + 1;
            char *end_params = strchr(line, ')');
            strncpy(parameters, start_params, end_params - start_params);
            parameters[end_params - start_params] = '\0';

            if (strstr(line, "-> i32")) {
                strcpy(return_type, "int");
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
                if (strstr(param_type, "i32")) {
                    strcat(translated_params, "int ");
                }
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

            // Check if this is the main function
            if (strcmp(function_name, "main") == 0) {
                in_main_function = 1;
            }
            continue;
        }
        
        // Check for end of function
        if (in_function && strstr(line, "}")) {
            if (in_main_function) {
                print_indentation(output_file, indentation);
                fprintf(output_file, "return 0;\n");
                in_main_function = 0;
            }
            fprintf(output_file, "}\n");
            in_function = 0;
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
                print_indentation(output_file, indentation);
                if (is_commented) {
                    fprintf(output_file, "// ");
                }
                fprintf(output_file, "printf(\"%s\\n\");\n", content);
            }
            continue;
        }

        // Copy comments directly
        if (strstr(line, "//")) {
            // Remove leading whitespace from the line before adding our indentation
            char* trimmed_line = line;
            while (*trimmed_line == ' ' || *trimmed_line == '\t') {
                trimmed_line++;
            }
            print_indentation(output_file, indentation);
            fprintf(output_file, "%s", trimmed_line);
            continue;
        }

        // Copy function body lines if inside any function
        if (in_function) {
            // Remove leading whitespace from the line before adding our indentation
            char* trimmed_line = line;
            while (*trimmed_line == ' ' || *trimmed_line == '\t') {
                trimmed_line++;
            }

            print_indentation(output_file, indentation);
            fprintf(output_file, "%s", trimmed_line);
        }
    }

    fclose(input_file);
    fclose(output_file);
}

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file.jade>\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char *output_filename = replace_extension(input_filename, ".c");

    transpile(input_filename, output_filename);

    printf("Transpiled %s to %s\n", input_filename, output_filename);

    free(output_filename);

    return 0;
}

