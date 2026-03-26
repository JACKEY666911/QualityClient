#include "qualitycontrolshortcutmanager.h"

#include <QShortcut>
#include <QKeySequence>
#include <QAbstractButton>
#include <QWidget>
#include <spdlog/spdlog.h>

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
    , m_serverType(ServerType::Unknown)
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
    if (const auto logger = spdlog::get("qdebug")) {
        logger->info("[ShortcutManager] configure, typeButtons={}", m_typeButtons.size());
    }
    setupShortcuts();
}

void QualityControlShortcutManager::setServerType(ServerType serverType)
{
    if (m_serverType == serverType) {
        return;
    }
    m_serverType = serverType;
    if (const auto logger = spdlog::get("qdebug")) {
        logger->info("[ShortcutManager] setServerType={}", static_cast<int>(m_serverType));
    }
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
    if (const auto logger = spdlog::get("qdebug")) {
        logger->debug("[ShortcutManager] setEnabled={}", m_enabled);
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
    settings.ensureShortcutDefaults();
    settings.sync();

    auto createShortcut = [this](const QString &sequence) {
        QShortcut *shortcut = new QShortcut(QKeySequence(sequence), m_scope);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        return shortcut;
    };

    if (!m_escShortcut) {
        m_escShortcut = createShortcut(settings.loadShortcut(QStringLiteral("Shortcuts/ESC")));
        connect(m_escShortcut, &QShortcut::activated, this, [this]() {
            if (m_escButton) {
                m_escButton->click();
            }
        });
    } else {
        m_escShortcut->setKey(QKeySequence(settings.loadShortcut(QStringLiteral("Shortcuts/ESC"))));
    }

    if (!m_startShortcut) {
        m_startShortcut = createShortcut(settings.loadShortcut(QStringLiteral("Shortcuts/StartCheck")));
        connect(m_startShortcut, &QShortcut::activated, this, [this]() {
            if (m_startButton) {
                m_startButton->click();
            }
        });
    } else {
        m_startShortcut->setKey(QKeySequence(settings.loadShortcut(QStringLiteral("Shortcuts/StartCheck"))));
    }

    if (!m_passShortcut) {
        m_passShortcut = createShortcut(settings.loadShortcut(QStringLiteral("Shortcuts/Pass")));
        connect(m_passShortcut, &QShortcut::activated, this, [this]() {
            if (m_passButton) {
                m_passButton->click();
            }
        });
    } else {
        m_passShortcut->setKey(QKeySequence(settings.loadShortcut(QStringLiteral("Shortcuts/Pass"))));
    }

    if (!m_detailShortcut) {
        m_detailShortcut = createShortcut(settings.loadShortcut(QStringLiteral("Shortcuts/PersonDetail")));
        connect(m_detailShortcut, &QShortcut::activated, this, [this]() {
            if (m_detailButton) {
                m_detailButton->click();
            }
        });
    } else {
        m_detailShortcut->setKey(QKeySequence(settings.loadShortcut(QStringLiteral("Shortcuts/PersonDetail"))));
    }

    struct TypeShortcut {
        int type;
        QString key;
    };

    const QList<TypeShortcut> typeShortcuts = {
        {XImage_TF_ED, QStringLiteral("Shortcuts/TF_ED")},
        {XImage_TF_GEN, QStringLiteral("Shortcuts/TF_GEN")},
        {XImage_TF_HI, QStringLiteral("Shortcuts/TF_HI")},
        {XImage_TF_LOW, QStringLiteral("Shortcuts/TF_LOW")},
        {XImage_TF_OS, QStringLiteral("Shortcuts/TF_OS")},
        {XImage_TF_MS, QStringLiteral("Shortcuts/TF_MS")},
        {XImage_YS_E0, QStringLiteral("Shortcuts/YS_E0")},
        {XImage_YS_E1, QStringLiteral("Shortcuts/YS_E1")},
        {XImage_YS_E2, QStringLiteral("Shortcuts/YS_E2")},
        {XImage_YS_OS, QStringLiteral("Shortcuts/YS_OS")},
        {XImage_YS_HD, QStringLiteral("Shortcuts/YS_HD")},
        {XImage_YS_SC, QStringLiteral("Shortcuts/YS_SC")}
    };

    for (const TypeShortcut &entry : typeShortcuts) {
        if (m_typeShortcuts.contains(entry.type)) {
            m_typeShortcuts.value(entry.type)->setKey(QKeySequence(settings.loadShortcut(entry.key)));
            continue;
        }
        QShortcut *shortcut = createShortcut(settings.loadShortcut(entry.key));
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
    switch (m_serverType) {
    case ServerType::YiSuo:
        enabledTypes = {XImage_YS_E0, XImage_YS_E1, XImage_YS_E2, XImage_YS_SC, XImage_YS_OS, XImage_YS_HD};
        break;
    case ServerType::TongFang:
        enabledTypes = {XImage_TF_ED, XImage_TF_GEN, XImage_TF_HI, XImage_TF_LOW, XImage_TF_OS, XImage_TF_MS};
        break;
    case ServerType::HaiMan:
        enabledTypes.clear();
        if (const auto logger = spdlog::get("qdebug")) {
            logger->warn("[ShortcutManager] unsupported serverType={}, disable type shortcuts",
                         static_cast<int>(m_serverType));
        }
        break;
    case ServerType::Unknown:
        enabledTypes.clear();
        if (const auto logger = spdlog::get("qdebug")) {
            logger->debug("[ShortcutManager] serverType unknown, disable type shortcuts");
        }
        break;
    default:
        enabledTypes.clear();
        if (const auto logger = spdlog::get("qdebug")) {
            logger->warn("[ShortcutManager] illegal serverType={}, disable type shortcuts",
                         static_cast<int>(m_serverType));
        }
        break;
    }

    for (auto it = m_typeShortcuts.begin(); it != m_typeShortcuts.end(); ++it) {
        it.value()->setEnabled(m_enabled && enabledTypes.contains(it.key()));
    }
    if (const auto logger = spdlog::get("qdebug")) {
        logger->debug("[ShortcutManager] refresh shortcuts serverType={}, enabledCount={}",
                      static_cast<int>(m_serverType), enabledTypes.size());
    }
}
