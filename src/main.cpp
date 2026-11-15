// slate — a settings app for the VEIKK A15 drawing tablet.

#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "backend.h"
#include "daemon.h"
#include "penmonitor.h"
#include "theme.h"
#include "translator.h"

int main(int argc, char *argv[])
{
    bool daemonMode = false;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == QStringLiteral("--daemon"))
            daemonMode = true;
    }

    if (daemonMode) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(QStringLiteral("slate"));
        QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

        Daemon daemon;
        QString error;
        if (!daemon.start(&error)) {
            qCritical().noquote() << "slate:" << error;
            return 1;
        }
        return app.exec();
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("slate"));
    app.setApplicationDisplayName(QStringLiteral("Slate"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // Matches slate.desktop, so the compositor (Wayland app_id) and the
    // launcher pick up the installed icon.
    app.setDesktopFileName(QStringLiteral("slate"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("slate")));

    QQuickStyle::setStyle(QStringLiteral("Material"));

    // Installed before anything is translated, and every qsTr() binding is
    // re-evaluated when the Omarchy language changes.
    Translator translator;
    app.installTranslator(&translator);

    Theme theme;
    Backend backend;
    PenMonitor pen;

    QQmlApplicationEngine engine;
    QObject::connect(&translator, &Translator::changed, &engine, &QQmlEngine::retranslate);
    engine.rootContext()->setContextProperty(QStringLiteral("translator"), &translator);
    engine.rootContext()->setContextProperty(QStringLiteral("theme"), &theme);
    engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
    engine.rootContext()->setContextProperty(QStringLiteral("pen"), &pen);
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
