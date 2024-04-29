#ifndef SHELL_H
#define SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

extern char* shell_execute(const char* input, int input_size);
extern char* shell_execute_mut(char* input_mut, int input_size);

#ifdef __cplusplus
}
#endif

#endif /* SHELL_H */
