#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "FontConfig.h"

#include <QFont>
#include <QMap>
#include <QReadWriteLock>



class FontManager
{
public:
    static FontManager& instance();

    // 初始化：注册并加载所有字体
    void init();

    // 注册字体配置（支持动态添加）
    void registerFontConfig(const FontConfig& config);

    // 获取指定类型的字体（带字号）
    QFont getFont(FontType type, int pointSize = -1);

    // 卸载所有字体（析构时自动调用）
    void unloadAllFonts();

private:
    FontManager() = default;
    FontManager(const FontManager &) =delete;
    FontManager& operator=(const FontManager &) = delete;

    // 线程安全锁（读写分离）
    QReadWriteLock m_lock;
    // 存储所有字体配置
    QMap<FontType, FontConfig> m_fontConfigs;
};

#endif // FONTMANAGER_H
