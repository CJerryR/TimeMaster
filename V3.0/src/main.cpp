#include "ui/MainWindow.h"
#include "core/Database.h"
#include "core/DeepSeekClient.h"
#include "ui/Theme.h"

#include <QApplication>
#include <QSettings>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // ---- 应用元信息（QSettings 持久化用） ----
    QApplication::setApplicationName("TimeMaster");
    QApplication::setApplicationDisplayName("时间管理大师");
    QApplication::setOrganizationName("TimeMaster");
    QApplication::setOrganizationDomain("timemaster.local");
    QApplication::setApplicationVersion("2.0.0");

    // ---- 字体 ----
#ifdef Q_OS_MAC
    QFont f("PingFang SC", 13);
#elif defined(Q_OS_WIN)
    QFont f("Microsoft YaHei UI", 10);
#else
    QFont f("Noto Sans CJK SC", 10);
#endif
    f.setHintingPreference(QFont::PreferFullHinting);
    f.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(f);

    // ---- 数据库 ----
    timemaster::Database db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, "时间管理大师",
            QString("数据库初始化失败。\n路径：%1").arg(db.filePath()));
        return 1;
    }

    // ---- AI 客户端 ----
    timemaster::DeepSeekClient ai;
    QSettings settings;
    QString savedKey = settings.value("deepseek_api_key").toString();
    if (!savedKey.isEmpty()) {
        ai.setApiKey(savedKey);
    }
    QString savedBase = settings.value("api_base_url").toString();
    if (!savedBase.isEmpty()) ai.setBaseUrl(savedBase);
    QString savedModel = settings.value("api_model").toString();
    if (!savedModel.isEmpty()) ai.setModel(savedModel);

    timemaster::Theme::instance();

    timemaster::MainWindow w(&db, &ai);
    w.show();

    return app.exec();
}
