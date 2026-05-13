//---------------------------Auther---------------------------
//Written by CJerryR
//https://github.com/CJerryR
//------------------------------------------------------------

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
QString g_slogen;     // V4.2: Smiley Sans
QString g_serif;      // V4.2: IBM Plex Serif (used for both primary and numeric)
bool    g_customLoaded = false;

// 注册单个字体文件并返回字体族名称
QString registerFont(const QString &path) {
    int id = QFontDatabase::addApplicationFont(path);
    if (id < 0) return {};
    QStringList fams = QFontDatabase::applicationFontFamilies(id);
    if (fams.isEmpty()) return {};
    return fams.first();
}

// 判断字体族名称是否属于中文字体
bool looksLikeCjk(const QString &fam) {
    return QFontDatabase::writingSystems(fam).contains(QFontDatabase::SimplifiedChinese)
        || fam.contains("Noto Sans") || fam.contains("Source Han") || fam.contains("YaHei")
        || fam.contains("PingFang") || fam.contains("HarmonyOS") || fam.contains("思源")
        || fam.contains("Smiley", Qt::CaseInsensitive)
        || fam.contains("得意黑");
}

// 判断字体族名称是否属于等宽字体
bool looksLikeMono(const QString &fam) {
    return fam.contains("Mono", Qt::CaseInsensitive) || fam.contains("Code", Qt::CaseInsensitive)
        || fam.contains("Consolas", Qt::CaseInsensitive);
}

// 判断是否属于 Slogan 字体（Smiley Sans / 得意黑）
bool looksLikeSlogen(const QString &fam, const QString &fname) {
    return fam.contains("Smiley", Qt::CaseInsensitive)
        || fname.contains("Smiley", Qt::CaseInsensitive)
        || fam.contains("得意黑");
}

// 判断是否属于 IBM Plex Serif 字体
bool looksLikeIbmPlexSerif(const QString &fam, const QString &fname) {
    return (fam.contains("IBM Plex", Qt::CaseInsensitive) && fam.contains("Serif", Qt::CaseInsensitive))
        || fname.contains("IBMPlexSerif", Qt::CaseInsensitive);
}
} // namespace

// 初始化字体加载器：搜索内置字体目录、分类注册并设置各字体族
void FontLoader::initialize() {
    QStringList searchRoots;
    searchRoots << ":/fonts";
    searchRoots << QCoreApplication::applicationDirPath() + "/fonts";
    searchRoots << QCoreApplication::applicationDirPath() + "/assets/fonts";

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
    QStringList serifCandidates;
    QStringList slogenCandidates;

    for (const QString &root : searchRoots) {
        QDirIterator it(root, {"*.ttf", "*.otf"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString p = it.next();
            QString fam = registerFont(p);
            if (fam.isEmpty()) continue;
            g_customLoaded = true;
            QString fname = QFileInfo(p).baseName();

            // V4.2: priority bucketing — slogan > serif > mono > cjk > primary
            if (looksLikeSlogen(fam, fname)) {
                slogenCandidates << fam;
                // Smiley Sans does contain CJK glyphs but we don't want it as the default
                // CJK family. So we DON'T add it to cjkCandidates.
            } else if (looksLikeIbmPlexSerif(fam, fname)) {
                serifCandidates << fam;
                primaryCandidates << fam;
            } else if (looksLikeMono(fname) || looksLikeMono(fam)) {
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

    // V4.2 §6.1 — IBM Plex Serif is now the preferred Latin family
    g_primary = pickFirstContaining(primaryCandidates,
                                    {"IBM Plex Serif", "Inter", "Source Sans", "SF Pro", "Manrope"});
    g_cjk     = pickFirstContaining(cjkCandidates, {"Noto Sans CJK SC", "Noto Sans SC",
                                                    "Source Han Sans SC", "Source Han Sans"});
    g_mono    = pickFirstContaining(monoCandidates, {"JetBrains Mono", "Source Code Pro", "Fira Code"});
    g_serif   = pickFirstContaining(serifCandidates, {"IBM Plex Serif"});
    g_slogen  = pickFirstContaining(slogenCandidates, {"Smiley Sans", "得意黑"});

    // ---- 兜底 ----
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
    if (g_serif.isEmpty()) {
        // Fallback to whatever the system has if IBM Plex Serif wasn't found
#ifdef Q_OS_WIN
        g_serif = "Georgia";
#elif defined(Q_OS_MAC)
        g_serif = "Times New Roman";
#else
        g_serif = "Serif";
#endif
    }
    // g_slogen may be empty if Smiley Sans wasn't found — callers handle that.

    qInfo() << "[FontLoader] primary=" << g_primary
            << "cjk=" << g_cjk
            << "mono=" << g_mono
            << "serif=" << g_serif
            << "slogen=" << g_slogen
            << "custom=" << g_customLoaded;
}

// 获取主显示字体族
QString FontLoader::primaryFamily() { return g_primary; }
// 获取中文显示字体族
QString FontLoader::cjkFamily()     { return g_cjk; }
// 获取等宽字体族
QString FontLoader::monoFamily()    { return g_mono; }
// 获取 Serif 字体族
QString FontLoader::serifFamily()   { return g_serif; }

// 获取 Slogan 字体族（Smiley Sans 优先，兜底用 CJK）
QString FontLoader::slogenFamily() {
    // If Smiley Sans wasn't loaded, fall back to a bold CJK font that still feels punchy
    if (!g_slogen.isEmpty()) return g_slogen;
    return g_cjk;
}

// 获取数字专用字体族（优先 Serif，兜底主字体）
QString FontLoader::numericFamily() {
    // V4.2 §6: numbers explicitly use IBM Plex Serif (literary/publication look)
    if (!g_serif.isEmpty()) return g_serif;
    return g_primary;
}

// 构建完整 CSS font-family 回退链（Serif → Primary → CJK → 系统兜底）
QString FontLoader::familyChain() {
    QStringList chain;
    auto add = [&](const QString &f) {
        if (!f.isEmpty() && !chain.contains(f)) chain << ("\"" + f + "\"");
    };
    // V4.2: Serif first (Latin), then CJK (for Chinese glyphs Qt falls back automatically)
    add(g_serif.isEmpty() ? g_primary : g_serif);
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
    add("serif");
    return chain.join(", ");
}

// 是否加载了自定义字体
bool FontLoader::customLoaded() { return g_customLoaded; }

} // namespace timemaster
