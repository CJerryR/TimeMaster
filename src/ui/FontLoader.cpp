#include "FontLoader.h"

#include <QFontDatabase>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDirIterator>
#include <QDebug>

namespace timemaster {

namespace {
QString g_primary;
QString g_cjk;
QString g_mono;
bool    g_customLoaded = false;

// 从一个 .ttf 文件注册到 QFontDatabase，返回主 family 名
QString registerFont(const QString &path) {
    int id = QFontDatabase::addApplicationFont(path);
    if (id < 0) return {};
    QStringList fams = QFontDatabase::applicationFontFamilies(id);
    if (fams.isEmpty()) return {};
    return fams.first();
}

// 判定一个 family 看起来是否中文字体（含 CJK 字符）
bool looksLikeCjk(const QString &fam) {
    QFont test(fam);
    return QFontDatabase::writingSystems(fam).contains(QFontDatabase::SimplifiedChinese)
        || fam.contains("Noto Sans") || fam.contains("Source Han") || fam.contains("YaHei")
        || fam.contains("PingFang") || fam.contains("HarmonyOS") || fam.contains("思源");
}

bool looksLikeMono(const QString &fam) {
    return fam.contains("Mono", Qt::CaseInsensitive) || fam.contains("Code", Qt::CaseInsensitive)
        || fam.contains("Consolas", Qt::CaseInsensitive);
}
} // namespace

void FontLoader::initialize() {
    QStringList searchRoots;
    // 1) Qt 资源
    searchRoots << ":/fonts";
    // 2) exe 同级 fonts/
    searchRoots << QCoreApplication::applicationDirPath() + "/fonts";
    // 3) exe 同级 assets/fonts/（项目开发态）
    searchRoots << QCoreApplication::applicationDirPath() + "/assets/fonts";
    // 4) 向上寻找项目根的 assets/fonts/
    QDir up(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 5; ++i) {
        if (up.exists("assets/fonts")) {
            searchRoots << up.absoluteFilePath("assets/fonts");
            break;
        }
        if (!up.cdUp()) break;
    }

    QStringList primaryCandidates;
    QStringList cjkCandidates;
    QStringList monoCandidates;

    for (const QString &root : searchRoots) {
        QDirIterator it(root, {"*.ttf", "*.otf"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString p = it.next();
            QString fam = registerFont(p);
            if (fam.isEmpty()) continue;
            g_customLoaded = true;
            QString fname = QFileInfo(p).baseName();
            if (looksLikeMono(fname) || looksLikeMono(fam)) {
                monoCandidates << fam;
            } else if (looksLikeCjk(fam) || looksLikeCjk(fname)) {
                cjkCandidates << fam;
            } else {
                primaryCandidates << fam;
            }
        }
    }

    auto pickFirstContaining = [](const QStringList &haystack, const QStringList &keys) -> QString {
        for (const QString &k : keys) {
            for (const QString &h : haystack) {
                if (h.contains(k, Qt::CaseInsensitive)) return h;
            }
        }
        return haystack.isEmpty() ? QString() : haystack.first();
    };

    // 主显示字体：偏好 Inter > Source Sans > 任意已加载
    g_primary = pickFirstContaining(primaryCandidates, {"Inter", "Source Sans", "SF Pro", "Manrope"});
    // 中文字体：偏好 Noto Sans CJK > Source Han > 已加载
    g_cjk     = pickFirstContaining(cjkCandidates, {"Noto Sans CJK SC", "Noto Sans SC",
                                                    "Source Han Sans SC", "Source Han Sans"});
    // 等宽：偏好 JetBrains Mono > Source Code Pro
    g_mono    = pickFirstContaining(monoCandidates, {"JetBrains Mono", "Source Code Pro", "Fira Code"});

    // ---- 兜底到系统字体 ----
    if (g_primary.isEmpty()) {
#ifdef Q_OS_WIN
        g_primary = "Segoe UI Variable";
        if (!QFontDatabase::families().contains(g_primary)) g_primary = "Segoe UI";
#elif defined(Q_OS_MAC)
        g_primary = "SF Pro Text";
#else
        g_primary = "Sans Serif";
#endif
    }
    if (g_cjk.isEmpty()) {
#ifdef Q_OS_WIN
        g_cjk = "Microsoft YaHei UI";
        if (!QFontDatabase::families().contains(g_cjk)) g_cjk = "Microsoft YaHei";
#elif defined(Q_OS_MAC)
        g_cjk = "PingFang SC";
#else
        g_cjk = "Noto Sans CJK SC";
#endif
    }
    if (g_mono.isEmpty()) {
#ifdef Q_OS_WIN
        g_mono = "Consolas";
#else
        g_mono = "Monospace";
#endif
    }

    qInfo() << "[FontLoader] primary=" << g_primary
            << "cjk=" << g_cjk
            << "mono=" << g_mono
            << "custom=" << g_customLoaded;
}

QString FontLoader::primaryFamily() { return g_primary; }
QString FontLoader::cjkFamily()     { return g_cjk; }
QString FontLoader::monoFamily()    { return g_mono; }

QString FontLoader::familyChain() {
    QStringList chain;
    auto add = [&](const QString &f) {
        if (!f.isEmpty() && !chain.contains(f)) chain << ("\"" + f + "\"");
    };
    add(g_primary);
    add(g_cjk);
#ifdef Q_OS_WIN
    add("Segoe UI Variable");
    add("Segoe UI");
    add("Microsoft YaHei UI");
#elif defined(Q_OS_MAC)
    add("SF Pro Text");
    add("PingFang SC");
#else
    add("Noto Sans CJK SC");
#endif
    add("sans-serif");
    return chain.join(", ");
}

bool FontLoader::customLoaded() { return g_customLoaded; }

} // namespace timemaster
