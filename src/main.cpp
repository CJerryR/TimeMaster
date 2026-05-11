#include "ui/MainWindow.h"
#include "core/Database.h"
#include "core/DeepSeekClient.h"
#include "ui/Theme.h"
#include "ui/FontLoader.h"
#include "ui/IconRenderer.h"

#include <QApplication>
#include <QSettings>
#include <QFont>
#include <QFontDatabase>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QIcon>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // ---- 应用元信息（QSettings 持久化用） ----
    QApplication::setApplicationName("TimeMaster");
    QApplication::setApplicationDisplayName("时间管理大师");
    QApplication::setOrganizationName("TimeMaster");
    QApplication::setOrganizationDomain("timemaster.local");
    QApplication::setApplicationVersion("3.2.0");

    // ---- 字体加载（Inter + 思源黑体，回退到系统字体） ----
    timemaster::FontLoader::initialize();

    // 应用基础字体：用 FontLoader 解析出的主字体，并通过 setFamilies 串起中英文回退链
    QFont appFont;
    QStringList families;
    if (!timemaster::FontLoader::primaryFamily().isEmpty()) families << timemaster::FontLoader::primaryFamily();
    if (!timemaster::FontLoader::cjkFamily().isEmpty())     families << timemaster::FontLoader::cjkFamily();
#ifdef Q_OS_WIN
    families << "Segoe UI Variable" << "Segoe UI" << "Microsoft YaHei UI";
#elif defined(Q_OS_MAC)
    families << "SF Pro Text" << "PingFang SC";
#else
    families << "Noto Sans CJK SC";
#endif
    appFont.setFamilies(families);
    appFont.setPointSize(10);
    appFont.setHintingPreference(QFont::PreferFullHinting);
    appFont.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(appFont);

    // ---- 应用主图标（用作任务栏/exe 图标） ----
    QIcon appIcon;
    timemaster::IconRenderer::Icon iconId = timemaster::IconRenderer::defaultAppIcon();
    for (int sz : {16, 24, 32, 48, 64, 128, 256}) {
        appIcon.addPixmap(timemaster::IconRenderer::appIcon(iconId, sz));
    }
    QApplication::setWindowIcon(appIcon);

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
