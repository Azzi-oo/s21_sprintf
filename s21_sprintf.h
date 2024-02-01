#define s21_size_t unsigned long long

#define s21_NULL (void*)0

#include "s21_string.h"

typedef struct 
{
    int minus;
    int plus;
    int space;
    int hash;
    int zero;
    int width;
    int accuracy;
    char length;
    int number_system;
    int flag_to_size;
    int dot;
    int upper_case;
    int g;
    int e;
} Spec;

const char *get_specs(const char *format, Spec *specs);
const char *get_width(const char *format, int *width, va_list *arguments);
const char *set_specs(Spec *specs, const char *format, va_list *arguments);
int add_sym_to_str(char *str, int *i, char symbol);
long add_space_plus_minus(Spec specs, long double num, int *i, char *str_to_num,
                          int *flag_to_size, long int *size);
int add_buffer_to_string(int width, int *i, char *buffer, chat *str);
int invert_num(long int *num_int, long double *num_double);
Spec set_number_system(Spec specs, char format);
char get_num_char(int num, int upper_case);
Spec set_num_sys_double(Spec specs, char format);
int add_zero_to_str(Spec specs, char *str_to_num, s21_size_t size, int *i,
                    int flag);
