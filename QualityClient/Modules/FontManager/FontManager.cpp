#include "FontManager.h"

FontManager &FontManager::instance()
{
    static FontManager instance;
    return instance;
}

void FontManager::init()
{

}

void FontManager::registerFontConfig(const FontConfig &config)
{

}

QFont FontManager::getFont(FontType type, int pointSize)
{
    return QFont();
}

void FontManager::unloadAllFonts()
{

}

