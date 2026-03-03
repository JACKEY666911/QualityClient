#ifndef FONTCONFIG_H
#define FONTCONFIG_H

#include <QString>



enum class FontType {
    IconFont,        // 图标字体
    BusinessFont,    // 业务字体
    TitleFont        // 标题字体
};

struct FontConfig
{

    FontType type;    //字体类型
    QString fontFilePath;//字体资源路径
    QString fontFamily;//字体家族名
    int defaultPointSize = 16;
    bool isValid = false;
    FontConfig() = default;// 配置是否有效
    FontConfig(FontType t, QString &path, QString &family, int size = 16):
    type(t), fontFilePath(path), fontFamily(family), defaultPointSize(size), isValid(true){}
};

#endif // FONTCONFIG_H
