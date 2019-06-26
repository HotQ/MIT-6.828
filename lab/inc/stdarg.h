#ifndef JOS_INC_STDARG_H
#define JOS_INC_STDARG_H

typedef char *va_list;

#define __va_size(x) ((sizeof(x) + 3) & 0xFFFFFFFC)
#define __get_val(type) *(type *)

#define va_start(ap, last) (ap = (va_list)&last + __va_size(last))
#define va_arg(ap, type)   (ap+= __va_size(type), __get_val(type)(ap-__va_size(type)))
#define va_end(ap)         (ap = NULL)

#endif /* !JOS_INC_STDARG_H */
