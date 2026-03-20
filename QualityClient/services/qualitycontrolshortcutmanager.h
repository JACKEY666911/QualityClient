#pragma once

#include <QObject>
#include <QHash>
#include <QSet>

class QWidget;
class QShortcut;
class QAbstractButton;
class SettingsService;

class QualityControlShortcutManager : public QObject
{
    Q_OBJECT
public:
    explicit QualityControlShortcutManager(QWidget *scope, QObject *parent = nullptr);

    void configure(const QHash<int, QAbstractButton*> &typeButtons,
                   QAbstractButton *escButton,
                   QAbstractButton *startButton,
                   QAbstractButton *passButton,
                   QAbstractButton *detailButton);

    void setBrandIsYisuo(bool isYisuo);
    void setEnabled(bool enabled);
    void reload();

private:
    void setupShortcuts();
    void refreshBrandShortcuts();
    void ensureShortcutDefaults(SettingsService &settings) const;
    QString readShortcut(SettingsService &settings, const QString &key, const QString &fallback) const;

    QWidget *m_scope;
    QHash<int, QAbstractButton*> m_typeButtons;
    QAbstractButton *m_escButton;
    QAbstractButton *m_startButton;
    QAbstractButton *m_passButton;
    QAbstractButton *m_detailButton;

    QShortcut *m_escShortcut;
    QShortcut *m_startShortcut;
    QShortcut *m_passShortcut;
    QShortcut *m_detailShortcut;
    QHash<int, QShortcut*> m_typeShortcuts;

    bool m_isYisuoBrand;
    bool m_enabled;
};
