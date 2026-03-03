#include "FontLoader.h"
#include <QFontDatabase>
#include <QDebug>
#include <QResource>
#include <QFile>
bool FontLoader::loadFont(const FontConfig &config)
{
    // 校验配置有效性
    if(!config.isValid && config.fontFilePath.isEmpty())
    {
        return false;
    }
    //检查字体文件是否存在
    if(!checkFontFileExists(config.fontFilePath))
    {
        return false;
    }

    //加载字体到数据库
    int fontId = QFontDatabase::addApplicationFont(config.fontFilePath);
    if(fontId == -1)
    {
        qWarning() << "[FontLoader] Font file not exists:" << config.fontFilePath;
        return false;
    }

    QStringList familyList = QFontDatabase::applicationFontFamilies(fontId);
    if(familyList.isEmpty() || !familyList.contains(config.fontFamily))
    {
        qWarning() << "[FontLoader] Font family not match:" << config.fontFamily;
        QFontDatabase::removeApplicationFont(fontId);
        return false;
    }
    return true;
}

bool FontLoader::unloadFont(const QString &fontFamily)
{
    QFontDatabase db;
    return db.hasFamily(fontFamily);
}

QFont FontLoader::getLoadedFont(const FontConfig &config, int fontSize)
{
    if (!config.isValid) {
        qWarning() << "[FontLoader] Get font failed: invalid config";
        return QFont(); // 返回系统默认字体
    }

    QFont font(config.fontFamily);
    // 优先使用传入的字号，无则用配置默认值
    font.setPointSize(fontSize > 0 ? fontSize : config.defaultPointSize);
    return font;
}

bool FontLoader::checkFontFileExists(const QString &filePath)
{
    QFile file(filePath);
    return file.exists();
}

FontLoader::FontLoader()
{

}
