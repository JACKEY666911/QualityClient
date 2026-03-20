#ifndef XIMAGETYPE_H
#define XIMAGETYPE_H

enum XImageType : int
{
    XImage_Invalid = -1,
    XImage_Origin = 499,

    XImage_TopView = 10,
    XImage_SideView = 11,

    XImage_HM_Origin = 300,
    XImage_HM_NEG = 301,
    XImage_HM_BW = 302,
    XImage_HM_O2 = 303,
    XImage_HM_SEN = 304,
    XImage_HM_OS = 305,
    XImage_HM_HI = 306,

    XImage_HM_CONTRABAND = 8000,

    XImage_YS_Origin = 200,
    XImage_YS_E0 = 201,
    XImage_YS_E1 = 202,
    XImage_YS_E2 = 203,
    XImage_YS_HD = 204,
    XImage_YS_SC = 205,
    XImage_YS_VAR = 206,
    XImage_YS_SCANNER = 207,
    XImage_YS_OS = 208,
    XImage_YS_Zoom = 209,
    XImage_YS_SC2 = 210,
    XImage_YS_CHART = 211,

    XImage_YS_CONTRABAND = 5000,

    XImage_TF_Origin = 100,
    XImage_TF_ED = 101,
    XImage_TF_HI = 102,
    XImage_TF_ADD = 103,
    XImage_TF_ESC = 104,
    XImage_TF_COLOR = 105,
    XImage_TF_OS = 106,
    XImage_TF_MS = 107,
    XImage_TF_OR = 108,
    XImage_TF_REV = 109,
    XImage_TF_S = 110,
    XImage_TF_GEN = 111,
    XImage_TF_LOW = 112,
    XImage_TF_SUB = 113,
    XImage_TF_SMO = 114,
    XImage_TF_GRAY = 115,
    XImage_TF_OS_PLUS = 116,
    XImage_TF_MS_PLUS = 117,
    XImage_TF_PC = 118,
    XImage_TF_789 = 119,
    XImage_TF_GRAD = 120,

    XImage_TF_CONTRABAND = 3000
};

#endif // XIMAGETYPE_H
