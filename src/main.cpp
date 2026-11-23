// slate — a settings app for the VEIKK A15 drawing tablet.

#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>

#include "backend.h"
#include "daemon.h"
#include "penmonitor.h"
#include "theme.h"
#include "translator.h"

int main(int argc, char *argv[])
{
    bool daemonMode = false;
    int initialPage = 0;
    bool initialCapture = false;
    QString screenshotPath;
    for (int i = 1; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        if (argument == QStringLiteral("--daemon")) {
            daemonMode = true;
        } else if (argument == QStringLiteral("--page") && i + 1 < argc) {
            initialPage = qBound(0, QString::fromLocal8Bit(argv[++i]).toInt(), 3);
        } else if (argument == QStringLiteral("--capture")) {
            initialCapture = true;
        } else if (argument == QStringLiteral("--screenshot") && i + 1 < argc) {
            screenshotPath = QString::fromLocal8Bit(argv[++i]);
        }
    }

    if (daemonMode) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(QStringLiteral("slate"));
        QCoreApplication::setApplicationVersion(QStringLiteral("0.2.0"));

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
    app.setApplicationVersion(QStringLiteral("0.2.0"));

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
    engine.rootContext()->setContextProperty(QStringLiteral("initialPage"), initialPage);
    engine.rootContext()->setContextProperty(QStringLiteral("initialCapture"), initialCapture);
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    // Development/documentation helper: render one page to a PNG and exit.
    // Useful for regenerating the screenshots in docs/ without a compositor.
    if (!screenshotPath.isEmpty()) {
        QTimer::singleShot(900, &app, [&engine, screenshotPath] {
            auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
            if (!window || !window->grabWindow().save(screenshotPath))
                qWarning().noquote() << "slate: could not save screenshot to" << screenshotPath;
            QCoreApplication::quit();
        });
    }

    return app.exec();
}
