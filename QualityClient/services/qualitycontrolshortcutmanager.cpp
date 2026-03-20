#include "qualitycontrolshortcutmanager.h"

#include <QShortcut>
#include <QKeySequence>
#include <QAbstractButton>
#include <QWidget>

#include "services/settingsservice.h"
#include "Models/XImageType.h"

QualityControlShortcutManager::QualityControlShortcutManager(QWidget *scope, QObject *parent)
    : QObject(parent)
    , m_scope(scope)
    , m_escButton(nullptr)
    , m_startButton(nullptr)
    , m_passButton(nullptr)
    , m_detailButton(nullptr)
    , m_escShortcut(nullptr)
    , m_startShortcut(nullptr)
    , m_passShortcut(nullptr)
    , m_detailShortcut(nullptr)
    , m_isYisuoBrand(false)
    , m_enabled(true)
{
}

void QualityControlShortcutManager::configure(const QHash<int, QAbstractButton*> &typeButtons,
                                              QAbstractButton *escButton,
                                              QAbstractButton *startButton,
                                              QAbstractButton *passButton,
                                              QAbstractButton *detailButton)
{
    m_typeButtons = typeButtons;
    m_escButton = escButton;
    m_startButton = startButton;
    m_passButton = passButton;
    m_detailButton = detailButton;
    setupShortcuts();
}

void QualityControlShortcutManager::setBrandIsYisuo(bool isYisuo)
{
    if (m_isYisuoBrand == isYisuo) {
        return;
    }
    m_isYisuoBrand = isYisuo;
    refreshBrandShortcuts();
}

void QualityControlShortcutManager::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (m_escShortcut) {
        m_escShortcut->setEnabled(enabled);
    }
    if (m_startShortcut) {
        m_startShortcut->setEnabled(enabled);
    }
    if (m_passShortcut) {
        m_passShortcut->setEnabled(enabled);
    }
    if (m_detailShortcut) {
        m_detailShortcut->setEnabled(enabled);
    }
    refreshBrandShortcuts();
}

void QualityControlShortcutManager::reload()
{
    setupShortcuts();
}

void QualityControlShortcutManager::setupShortcuts()
{
    SettingsService &settings = SettingsService::instance();
    ensureShortcutDefaults(settings);
    settings.sync();

    auto createShortcut = [this](const QString &sequence) {
        QShortcut *shortcut = new QShortcut(QKeySequence(sequence), m_scope);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        return shortcut;
    };

    if (!m_escShortcut) {
        m_escShortcut = createShortcut(readShortcut(settings, QStringLiteral("Shortcuts/ESC"), QStringLiteral("Esc")));
        connect(m_escShortcut, &QShortcut::activated, this, [this]() {
            if (m_escButton) {
                m_escButton->click();
            }
        });
    }

    if (!m_startShortcut) {
        m_startShortcut = createShortcut(readShortcut(settings, QStringLiteral("Shortcuts/StartCheck"), QStringLiteral("F")));
        connect(m_startShortcut, &QShortcut::activated, this, [this]() {
            if (m_startButton) {
                m_startButton->click();
            }
        });
    }

    if (!m_passShortcut) {
        m_passShortcut = createShortcut(readShortcut(settings, QStringLiteral("Shortcuts/Pass"), QStringLiteral("Space")));
        connect(m_passShortcut, &QShortcut::activated, this, [this]() {
            if (m_passButton) {
                m_passButton->click();
            }
        });
    }

    if (!m_detailShortcut) {
        m_detailShortcut = createShortcut(readShortcut(settings, QStringLiteral("Shortcuts/PersonDetail"), QStringLiteral("`")));
        connect(m_detailShortcut, &QShortcut::activated, this, [this]() {
            if (m_detailButton) {
                m_detailButton->click();
            }
        });
    }

    struct TypeShortcut {
        int type;
        QString key;
        QString fallback;
    };

    const QList<TypeShortcut> typeShortcuts = {
        {XImage_TF_ED, QStringLiteral("Shortcuts/TF_ED"), QStringLiteral("W")},
        {XImage_TF_GEN, QStringLiteral("Shortcuts/TF_GEN"), QStringLiteral("Q")},
        {XImage_TF_HI, QStringLiteral("Shortcuts/TF_HI"), QStringLiteral("E")},
        {XImage_TF_LOW, QStringLiteral("Shortcuts/TF_LOW"), QStringLiteral("R")},
        {XImage_TF_OS, QStringLiteral("Shortcuts/TF_OS"), QStringLiteral("3")},
        {XImage_TF_MS, QStringLiteral("Shortcuts/TF_MS"), QStringLiteral("4")},
        {XImage_YS_E0, QStringLiteral("Shortcuts/YS_E0"), QStringLiteral("1")},
        {XImage_YS_E1, QStringLiteral("Shortcuts/YS_E1"), QStringLiteral("2")},
        {XImage_YS_E2, QStringLiteral("Shortcuts/YS_E2"), QStringLiteral("3")},
        {XImage_YS_OS, QStringLiteral("Shortcuts/YS_OS"), QStringLiteral("Q")},
        {XImage_YS_HD, QStringLiteral("Shortcuts/YS_HD"), QStringLiteral("W")},
        {XImage_YS_SC, QStringLiteral("Shortcuts/YS_SC"), QStringLiteral("E")}
    };

    for (const TypeShortcut &entry : typeShortcuts) {
        if (m_typeShortcuts.contains(entry.type)) {
            continue;
        }
        QShortcut *shortcut = createShortcut(readShortcut(settings, entry.key, entry.fallback));
        m_typeShortcuts.insert(entry.type, shortcut);
        connect(shortcut, &QShortcut::activated, this, [this, entry]() {
            QAbstractButton *button = m_typeButtons.value(entry.type, nullptr);
            if (button) {
                button->click();
            }
        });
    }

    refreshBrandShortcuts();
    setEnabled(m_enabled);
}

void QualityControlShortcutManager::refreshBrandShortcuts()
{
    QSet<int> enabledTypes;
    if (m_isYisuoBrand) {
        enabledTypes = {XImage_YS_E0, XImage_YS_E1, XImage_YS_E2, XImage_YS_SC, XImage_YS_OS, XImage_YS_HD};
    } else {
        enabledTypes = {XImage_TF_ED, XImage_TF_GEN, XImage_TF_HI, XImage_TF_LOW, XImage_TF_OS, XImage_TF_MS};
    }

    for (auto it = m_typeShortcuts.begin(); it != m_typeShortcuts.end(); ++it) {
        it.value()->setEnabled(m_enabled && enabledTypes.contains(it.key()));
    }
}

void QualityControlShortcutManager::ensureShortcutDefaults(SettingsService &settings) const
{
    if (!settings.contains(QStringLiteral("Shortcuts/TF_ED"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_ED"), QStringLiteral("W"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/TF_GEN"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_GEN"), QStringLiteral("Q"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/TF_HI"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_HI"), QStringLiteral("E"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/TF_LOW"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_LOW"), QStringLiteral("R"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/TF_OS"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_OS"), QStringLiteral("3"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/TF_MS"))) {
        settings.setValue(QStringLiteral("Shortcuts/TF_MS"), QStringLiteral("4"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_E0"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_E0"), QStringLiteral("1"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_E1"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_E1"), QStringLiteral("2"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_E2"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_E2"), QStringLiteral("3"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_OS"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_OS"), QStringLiteral("Q"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_HD"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_HD"), QStringLiteral("W"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/YS_SC"))) {
        settings.setValue(QStringLiteral("Shortcuts/YS_SC"), QStringLiteral("E"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/ESC"))) {
        settings.setValue(QStringLiteral("Shortcuts/ESC"), QStringLiteral("Esc"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/StartCheck"))) {
        settings.setValue(QStringLiteral("Shortcuts/StartCheck"), QStringLiteral("F"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/Pass"))) {
        settings.setValue(QStringLiteral("Shortcuts/Pass"), QStringLiteral("Space"));
    }
    if (!settings.contains(QStringLiteral("Shortcuts/PersonDetail"))) {
        settings.setValue(QStringLiteral("Shortcuts/PersonDetail"), QStringLiteral("`"));
    }
}

QString QualityControlShortcutManager::readShortcut(SettingsService &settings, const QString &key, const QString &fallback) const
{
    return settings.value(key, fallback).toString();
}
