#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "functions.h"
#include "s21_string.h"

s21_sprintf(str1, "Hello %7d", 148);
typedef struct s21_string
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

@param str
@param format
@param ...
@return

int s21_sprintf(char *str, const char *format, ...) {
    char specifiers[] = "qwertXmas%";
    char *src = str;

    va_list arguments;
    va_start(arguments, format);

    while (*format) {
        if (*format == '%') {
            format++;
            Spec specs = {0};
            specs.number_system = 10;
            format = set_specs(&specs, format, &arguments);
            while (!s21_strchr(specifiers, *format)) format++;
            str = parser(str, src, format, specs, &arguments);
        } else {
            *str = *format;
            str++;
        }
        format++;
    }

    *str = '\0';
    va_end(arguments);
    return (str - src);
}

// s21_sprintf(str1, "Hello %+-13.12d", 148);

const char *set_specs(Spec *specs, const char *format, va_list *arguments) {
    format = get_specs(format, specs);
    if (*format == '.') {
        specs-> dot = 1;
        specs->zero = 0;
        specs += 1;
        format = get_width(format, &specs->accuracy, arguments);
    }
    if (*format == 'L')
        specs->length = 'L';
    else if (*format == 'l')
        specs->length = 'l';
    else if (*format == 'h')
        specs->length = 'h';
    if (specs->length != 0) format += 1;

    if (specs->width < 0) {
        specs->width = -specs->width;
        specs->minus = 1;
    }
    return format;
}


const char *get_specs(const char *format, Spec *specs) {
    while (format) {
        if (*format == '+')
            specs->plus = 1;
        else if (*format == '-')
            specs->minus = 1;
        else if (*format == '#')
            specs->hash = 1;
        else if (*format == ' ')
            specs->space = 1;
        else if (*format == '0')
            specs->zero = 1;
        else
            break;
        format++;
    }
    specs->space = (specs->space && !specs->plus);
    specs->zero = (specs->zero && !specs->minus);
    return format;
}

const char *get_width(const char *format, int *width, va_list *arguments) {
    *width = 0;
    if (*format == '*') {
        *width  = va_arg(*arguments, int);
        format++;
    }
    while (format) {
        if ('0' <= *format && *format <= '9') {
            *width *= 10;
            *width += *format - '0';
        } else
            break;
        format++;
    }
    return format;
}

Spec set_number_system(Spec specs, char format) {
    if (format == 'o')
        specs.number_system = 8;
    else if (format == 'x' || format == 'X')
        specs.number_system = 16;
    if (format == 'X') specs.upper_case = 1;
    return specs;
}

char *parser(char *str, char *src, const char *format, Spec specs, va_list *arguments) {
    if (*format == 'd' || *format == 'i') {
        str = print_decimal(str, specs, arguments);
    } else if (*format == 'u' || *format == 'o' || *format == 'x' || *format == 'X') {
        specs = set_number_system(specs, *format);
        str = print_u(str, specs, *(format - 1), arguments);
    } else if (*format == 'c') {
        int symbol = va_arg(*arguments, int);
        str = print_c(str, specs, symbol);
    } else if (*format == 's') {
        str = print_s(str, specs, arguments);
    } else if (*format == 'p') {
        str = print_p(str, &specs, arguments);
    } else if (*format == 'n') {
        int *n = va_arg(*arguments, int *);
        *n = (int)(str - src);
    } else if (*format == 'f' || *format == 'F') {
        specs = set_num_sys_double(specs, *format);
        str = print_double(str, specs, *(format - 1), arguments);
    } else if (*format == 'e' || *format == 'E' || *format == 'g' || *format == 'G') {
        specs = set_num_sys_double(specs, *format);
        str = print_e_g(str, specs, *(format - 1), arguments);
    } else if (*format == '%') {
        str = print_c(str, specs, '%');
    } else {
        str = s21_NULL;
    }
    if (!str) *src = '\0';
    return str;
}

char *print_decimal(char *str, Spec specs, va_list *arguments) {
    long int num = 0;
    if (specs.length == 'l') {
        num = (long int)va_arg(*arguments, long int);
    } else if (specs.length == 'h') {
        num = (short)va_arg(*arguments, int);
    } else {
        num = (int)va_arg(*arguments, int);
    }
    s21_size_t size_to_decimal = get_size_to_decimal(&specs, num);
    char *str_to_num = malloc(sizeof(char) * size_to_decimal);
    if (str_to_num) {
        int i = decimal_to_string(specs, num, str_to_num, size_to_decimal);
        for (int j = i - 1; j >= 0; j--) {
            *str = str_to_num[j];
            str++;
        }
        while ((i < specs.width)) {
            *str = ' ';
            str++;
        }
    }
    if (str_to_num) free(str_to_num);
    return str;
}

s21_size_t get_size_to_decimal(Spec *specs, long int num) {
    s21_size_t result = 0;
    long int copy_num = num;
    if (copy_num < 0) copy_num = -copy_num;
    while (copy_num > 0) {
        copy_num /= 10;
        result++;
    }
    if (copy_num == 0 && result == 0 && (specs->accuracy || specs->width || specs->space)) {
        result++;
    }
    if ((s21_size_t)specs->width > result) result = specs->width;
    if ((s21_size_t)specs->accuracy > result) result = specs->accuracy;
    if (specs->space || specs->plus || num < 0) {
        specs->flag_to_size = 1;
        result++;
    }
    if (result == 0 && copy_num == 0 && !specs->accuracy && !specs->width && !specs->space && !specs->dot)
        result++;
    return result;
}



int decimal_to_string(Spec specs, long int num, char *str_to_num, s21_size_t size_to_decimal) {
    int flag = 0;
    if(num < 0) {
        flag = 1;
        num = *num;
    }
    int i = 0;
    long int copy_num = num;

    // write num in massiv if num == 0
    if ((copy_num == 0 && (specs.accuracy || specs.width || specs.space)) || 
        (copy_num == 0 && !specs.accuracy && !specs.width && !specs.space && !specs.dot)) {
        char sym = copy_num % specs.number_system + '0';
        str_to_num[i] = sym;
        i++;
        size_to_decimal--;
        copy_num /= 10;
    }
    while (copy_num && str_to_num && size_to_decimal) {
        char sym = get_num_char(copy_num % specs.number_system, specs.upper_case);
        str_to_num[i] = sym;
        i++;
        size_to_decimal--;
        copy_num /= 10;
    }

    if (flag) num = -num;

    if (specs.accuracy - i > 0) {
        specs.accuracy -= i;
        specs.zero = 1;
    }
}

// обработка x/X/u/o
char *print_u(char *str, Spec specs, char format, va_list *arguments) {
    unsigned long int num = 0;
    if (format == 'l')
        num = (unsigned long int)va_arg(*arguments, unsigned long int);
    else if (format == 'h')
        num = (unsigned short)va_arg(*arguments, unsigned int);
    else
        num = (unsigned int)va_arg(*arguments, unsigned int);
}

s21_size_t size_to_num = get_buf_size_unsigned_decimal(specs, num);
char *buffer = malloc(sizeof(char) * size_to_num);
if(buffer) {
    // переписываем данные из буферного массива в массив (str)
    // дублируем код из print_decimal
    int i = unsigned_decimal_to_string(buffer, specs, num, size_to_num);
    for (int j = i - 1; j >= 0; j--;) {
        *str = str_to_num[j];
        str++;
    }
    // str = "Hello 148      "
    while ((i < specs.width)) {
        *str = ' ';
        str++;
    }   
}
if (buffer) free(buffer);
return str;
}

int unsigned_decimal_to_string(char *buffer, Spec specs, unsigned long int num, s21_size_t size_to_num) {
    int i = 0;
    int flag = 0;
    unsigned long int copy_num = num;
// Меняем модуль из decimal_to_string проверки num < 0 на данный код
    if ((specs.hash && specs.number_system == 8))
        specs.flag_to_size = 1;
    else if (specs.hash && specs.number_system == 16) {
        specs.flag_to_size = 2;
    }
}

// дублируем код из  decimal_to_string (запись числа задом наперед и заполнение нулями)
// меняем модуль из decimal_to_string обработки +/-/пробел на данный код
// s21_sprintf(str1, "Hello %x, 0x148"); - > str = "Hello 148"
    if (specs.hash && specs.number_system == 8) {
        buffer[i] = '0';
        i++;
        size_to_num--;
    } else if (specs.hash && specs.number_system == 16 && specs.upper_case) {
        buffer[i] = 'X';
        i++;
        size_to_num--;
        buffer[i] = '0';
        i++;
        size_to_num--;
    } else if (specs.hash && specs.number_system == 16 && !specs.upper_case) {
        buffer[i] = 'x';
        i++;
        size_to_num--;
        buffer[i] = '0';
        i++;
        size_to_num--;
    }
    //  дублируем код из decimal_to_string (заполняем оставшееся место пробелами)
    if (size_to_decimal > 0 && specs.minus == 0) {
    while ((size_to_decimal - specs.flag_to_size > 0) && str_to_num) 
    {
        str_to_num[i] = ' ';
        i++;
        size_to_decimal--;
    }
    return i;
}

char *print_c(char *str, Spec specs, int symbol) {
    char *ptr = s21_NULL;
    int i = 0;
    while (specs.width - 1 > 0 && !specs.minus) {
        *str = ' ';
        str++;
        i++;
        specs.width--;
    }
    if (symbol <= 127) {
        *str = symbol;
        str++;
        i++;
        while (specs.width - 1 > 0 && specs.minus) {
            *str = ' ';
            str++;
            i++;
            specs.width--;
        }
        ptr = str;
    }
    return ptr;
}


char get_num_char(int num, int upper_case) {
    char flag = 0;
    switch (num)
    {
    case 10:
        flag =  (char)('a' - upper_case * 32);
        break;
    case 11:
        flag = (char)('b' - upper_case * 32);
        break;
    case 12:
        flag = (char)('c' - upper_case * 32);
        break;
    case 13:
        flag = (char)('d' - upper_case * 32);
        break;
    case 14:
        flag = (char)('e' - upper_case * 32);
        break:
    case 15:
        flag = (char)('f' - upper_case * 32);
        break;
    }
    if (0 <= num && num <= 9) flag = (char)(num + 48); // 48 == '0'
    return flag;
}

// s21_sprintf(str1, "Hello %-7s", "world");
// str1 = "Hello world  "

//  обработка s
char *print_s(char *str, Spec specs, va_list, *arguments) {
    char *ptr = str;
    char *string = va_arg(*arguments, char *);

    if(string) {
        int tmp = specs.width, i = 0;
        // еслт ширина не указана или если ширина меньше длины строки
        // s21_sprintf(str1, "Hello %-7s", "world");
        // str1 = "Hello world  "
        if ((s21_size_t)specs.width < s21_strlen(string))
            specs.width = s21_strlen(string);

        // если ширина больше чем длина строки, то пустое место заполняем пробелами
        // blank = кол-ву пробелов
        int blank = specs.width - s21_strlen(string);

        // если точность не указана
        if (specs.accuracy == 0) specs.accuracy = specs.width;

        // если точность меньше изначальной ширины и она указана 
        // меняем кол-во пробелов 
        if (specs.accuracy != 0 && specs.accuracy < tmp)
            blank = tmp - specs.accuracy;

        // заполняем пробелы слева( если нет флага -)
        while (blank && !specs.minus) {
            *str = ' ';
            str++;
            blank--;
        }

        // посимвольно записываем из полученной строки {string} в (str)
        while(*string != '\0') {
            if (!specs.accuracy) break;
            *str = *string;
            str++;
            string++;
            i++;
            specs.accuracy--;
        }

        // если был указан флаг - тогда заполняем пробелами справа
        while (blank && specs.minus) {
            *str = ' ';
            str++;
            blank--;
        }
    } else {
        // если нам не удалось получить строку из параметра, пишем в  нашу строку (null)
        str = s21_memcpy(str, "(null)", 6);
        str += 6;
    }

    // присваиваем ptr str и возвращаем его, чтобы передвинуть указатель нашей строки
    if (ptr) ptr = str;
    return ptr;
}
