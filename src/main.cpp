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

int main(int argc, char* argv [ ]) {
    QApplication app(argc, argv);

    // ---- 应用元信息 (QSettings 用于此) ----
    QApplication::setApplicationName("TimeplanCpp");
    QApplication::setApplicationDisplayName("时智");
    QApplication::setOrganizationName("TimeplanCpp");
    QApplication::setApplicationVersion("1.0.0");

    // ---- 字体（macOS 优先使用 PingFang SC，Windows 优先微软雅黑） ----
#ifdef Q_OS_MAC
    QFont f("PingFang SC", 13);
#elif defined(Q_OS_WIN)
    QFont f("Microsoft YaHei UI", 10);
#else
    QFont f("Noto Sans CJK SC", 10);
#endif
    app.setFont(f);

    // ---- 数据库 ----
    timeplan::Database db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, "时智",
            QString("数据库初始化失败。\n路径：%1").arg(db.filePath()));
        return 1;
    }

    // ---- 客户端 ----
    timeplan::DeepSeekClient ai;
    QSettings settings;
    QString savedKey = settings.value("deepseek_api_key").toString();
    if (!savedKey.isEmpty()) {
        ai.setApiKey(savedKey);
    }

    // ---- 主题（自动从 QSettings 恢复，构造时已处理） ----
    timeplan::Theme::instance();

    // ---- 主窗口 ----
    timeplan::MainWindow w(&db, &ai);
    w.show();

    return app.exec();
}
