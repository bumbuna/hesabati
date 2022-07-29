//Jacob Bumbuna <developer@devbumbuna.com>
//2022

#if !defined(HESABATI_H)
#define HESABATI_H

#define HESABATI_VERSION_MAJOR 1
#define HESABATI_VERSION_MINOR 0
#define HESABATI_VERSION_PATCH 0
#define HESABATI_VERSION_TWEAK 0
#define HESABATI_HOMEPAGE_URL  "https://hesabati.devbumbuna.com"

enum RETURN_TYPE {
    TYPE_NUM,
    TYPE_DOUBLE,
};
typedef signed long hnum_t;
typedef  double hdouble_t;
typedef float hfloat_t;
int
hesabati(const char *, int *, void **);
#endif //HESABATI_H
