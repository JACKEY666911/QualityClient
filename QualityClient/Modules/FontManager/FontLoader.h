#ifndef FONTLOADER_H
#define FONTLOADER_H

#include "FontConfig.h"

#include <QFont>



class FontLoader
{
public:

    //加载字体文件
    static bool loadFont(const FontConfig &config);
    //卸载指定字体
    static bool unloadFont(const QString &fontFamily);
    //获取加载后的字体文件
    static QFont getLoadedFont(const FontConfig &config, int fontSize = -1);
    //检查字体文件是否存在
    static bool checkFontFileExists(const QString &filePath);
private:
        FontLoader();
};

#endif // FONTLOADER_H
