/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 Hewlett-Packard Development Company, L.P.
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebosfontdatabase.h"
#include <QtGui/QPlatformNativeInterface>
#include <QtGui/private/qapplication_p.h>
#include <QtGui/QPlatformScreen>

#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>
#include <QtDebug>

#include <QCryptographicHash>
#include <QDomDocument>

#undef QT_NO_FREETYPE
#include <QtGui/private/qfontengine_ft_p.h>
#include <QtGui/private/qfontengine_p.h>

#include <ft2build.h>
#include FT_TRUETYPE_TABLES_H

#define SimplifiedChineseCsbBit 18
#define TraditionalChineseCsbBit 20
#define JapaneseCsbBit 17
#define KoreanCsbBit 21

static int requiredUnicodeBits[QFontDatabase::WritingSystemsCount][2] = {
        // Any,
    { 127, 127 },
        // Latin,
    { 0, 127 },
        // Greek,
    { 7, 127 },
        // Cyrillic,
    { 9, 127 },
        // Armenian,
    { 10, 127 },
        // Hebrew,
    { 11, 127 },
        // Arabic,
    { 13, 127 },
        // Syriac,
    { 71, 127 },
    //Thaana,
    { 72, 127 },
    //Devanagari,
    { 15, 127 },
    //Bengali,
    { 16, 127 },
    //Gurmukhi,
    { 17, 127 },
    //Gujarati,
    { 18, 127 },
    //Oriya,
    { 19, 127 },
    //Tamil,
    { 20, 127 },
    //Telugu,
    { 21, 127 },
    //Kannada,
    { 22, 127 },
    //Malayalam,
    { 23, 127 },
    //Sinhala,
    { 73, 127 },
    //Thai,
    { 24, 127 },
    //Lao,
    { 25, 127 },
    //Tibetan,
    { 70, 127 },
    //Myanmar,
    { 74, 127 },
        // Georgian,
    { 26, 127 },
        // Khmer,
    { 80, 127 },
        // SimplifiedChinese,
    { 126, 127 },
        // TraditionalChinese,
    { 126, 127 },
        // Japanese,
    { 126, 127 },
        // Korean,
    { 56, 127 },
        // Vietnamese,
    { 0, 127 }, // same as latin1
        // Other,
    { 126, 127 },
        // Ogham,
    { 78, 127 },
        // Runic,
    { 79, 127 },
        // Nko,
    { 14, 127 },
};

static const char *s_writingSystemStrings[QFontDatabase::WritingSystemsCount] = {
    "Any",
    "Latin",
    "Greek",
    "Cyrillic",
    "Armenian",
    "Hebrew",
    "Arabic",
    "Syriac",
    "Thaana",
    "Devanagari",
    "Bengali",
    "Gurmukhi",
    "Gujarati",
    "Oriya",
    "Tamil",
    "Telugu",
    "Kannada",
    "Malayalam",
    "Sinhala",
    "Thai",
    "Lao",
    "Tibetan",
    "Myanmar",
    "Georgian",
    "Khmer",
    "SimplifiedChinese",
    "TraditionalChinese",
    "Japanese",
    "Korean",
    "Vietnamese",
    "Symbol",
    "Ogham",
    "Runic",
    "Nko"
};

static QString qSupportedWritingSystemsToQString(const QSupportedWritingSystems &writingSystems)
{
    QStringList writingSystemsStringList;

    int i;
    for (i = 0; i < QFontDatabase::WritingSystemsCount; i++) {
        if (writingSystems.supported(QFontDatabase::WritingSystem(i)))
            writingSystemsStringList << s_writingSystemStrings[i];
    }
    return writingSystemsStringList.join(",");
}

static QSupportedWritingSystems determineWritingSystemsFromTrueTypeBits(quint32 unicodeRange[4], quint32 codePageRange[2])
{
    QSupportedWritingSystems writingSystems;
    bool hasScript = false;

    int i;
    for(i = 0; i < QFontDatabase::WritingSystemsCount; i++) {
        int bit = requiredUnicodeBits[i][0];
        int index = bit/32;
        int flag =  1 << (bit&31);
        if (bit != 126 && unicodeRange[index] & flag) {
            bit = requiredUnicodeBits[i][1];
            index = bit/32;

            flag =  1 << (bit&31);
            if (bit == 127 || unicodeRange[index] & flag) {
                writingSystems.setSupported(QFontDatabase::WritingSystem(i));
                hasScript = true;
            }
        }
    }
    if(codePageRange[0] & (1 << SimplifiedChineseCsbBit)) {
        writingSystems.setSupported(QFontDatabase::SimplifiedChinese);
        hasScript = true;
    }
    if(codePageRange[0] & (1 << TraditionalChineseCsbBit)) {
        writingSystems.setSupported(QFontDatabase::TraditionalChinese);
        hasScript = true;
    }
    if(codePageRange[0] & (1 << JapaneseCsbBit)) {
        writingSystems.setSupported(QFontDatabase::Japanese);
        hasScript = true;
    }
    if(codePageRange[0] & (1 << KoreanCsbBit)) {
        writingSystems.setSupported(QFontDatabase::Korean);
        hasScript = true;
    }
    if (!hasScript)
        writingSystems.setSupported(QFontDatabase::Symbol);

    return writingSystems;
}

static inline bool scriptRequiresOpenType(int script)
{
    return ((script >= QUnicodeTables::Syriac && script <= QUnicodeTables::Sinhala)
            || script == QUnicodeTables::Khmer || script == QUnicodeTables::Nko);
}

static const char *s_hintingPreferenceStrings[QFont::PreferFullHinting+1] = {
    "PreferDefaultHinting",
    "PreferNoHinting",
    "PreferVerticalHinting",
    "PreferFullHinting"
};

static QString qHintingPreferenceToQString(const QFont::HintingPreference pref) {
    return QString(s_hintingPreferenceStrings[pref]);
}

static const char *s_styleStrings[QFont::StyleOblique+1] = {
    "StyleNormal",
    "StyleItalic",
    "StyleOblique"
};

static QString qStyleToQString(const QFont::Style style) {
    return QString(s_styleStrings[style]);
}

static QString qWeightToQString(const QFont::Weight weight) {
    int weightInt = (int) weight;
    switch (weightInt) {
    case QFont::Light:
        return "Light";
    case QFont::Normal:
        return "Normal";
    case QFont::DemiBold:
        return "DemiBold";
    case QFont::Bold:
        return "Bold";
    case QFont::Black:
        return "Black";
    default:
        return QString(weightInt);
    }
}

static QString qStretchToQString(const QFont::Stretch stretch) {
    int stretchInt = (int) stretch;
    switch (stretchInt) {
    case QFont::UltraCondensed:
        return "UltraCondensed";
    case QFont::ExtraCondensed:
        return "ExtraCondensed";
    case QFont::Condensed:
        return "Condensed";
    case QFont::SemiCondensed:
        return "SemiCondensed";
    case QFont::Unstretched:
        return "Unstretched";
    case QFont::SemiExpanded:
        return "SemiExpanded";
    case QFont::Expanded:
        return "Expanded";
    case QFont::ExtraExpanded:
        return "ExtraExpanded";
    case QFont::UltraExpanded:
        return "UltraExpanded";
    default:
        return QString(stretchInt);
    }
}

static const char *s_styleHintStrings[QFont::Fantasy+1] = {
    "Helvetica",
    "Times",
    "Courier",
    "OldEnglish",
    "System",
    "AnyStyle",
    "Cursive",
    "Monospace",
    "Fantasy"
};

static QString qStyleHintToQString(const QFont::StyleHint styleHint) {
    return QString(s_styleHintStrings[styleHint]);
}

static QString qStyleStrategyToQString(const QFont::StyleStrategy styleStrategy) {
    QStringList strategies;

    if (styleStrategy & QFont::PreferDefault)
        strategies << "PreferDefault";
    if (styleStrategy & QFont::PreferBitmap)
        strategies << "PreferBitmap";
    if (styleStrategy & QFont::PreferDevice)
        strategies << "PreferDevice";
    if (styleStrategy & QFont::PreferOutline)
        strategies << "PreferOutline";
    if (styleStrategy & QFont::ForceOutline)
        strategies << "ForceOutline";
    if (styleStrategy & QFont::PreferMatch)
        strategies << "PreferMatch";
    if (styleStrategy & QFont::PreferQuality)
        strategies << "PreferQuality";
    if (styleStrategy & QFont::PreferAntialias)
        strategies << "PreferAntialias";
    if (styleStrategy & QFont::NoAntialias)
        strategies << "NoAntialias";
    if (styleStrategy & QFont::OpenGLCompatible)
        strategies << "OpenGLCompatible";
    if (styleStrategy & QFont::ForceIntegerMetrics)
        strategies << "ForceIntegerMetrics";
    if (styleStrategy & QFont::NoFontMerging)
        strategies << "NoFontMerging";

    return strategies.join("|");
}

static const char *s_scriptStrings[QUnicodeTables::ScriptCount] = {
    "Common",
    "Greek",
    "Cyrillic",
    "Armenian",
    "Hebrew",
    "Arabic",
    "Syriac",
    "Thaana",
    "Devanagari",
    "Bengali",
    "Gurmukhi",
    "Gujarati",
    "Oriya",
    "Tamil",
    "Telugu",
    "Kannada",
    "Malayalam",
    "Sinhala",
    "Thai",
    "Lao",
    "Tibetan",
    "Myanmar",
    "Georgian",
    "Hangul",
    "Ogham",
    "Runic",
    "Khmer",
    "Nko"
};

static QString qScriptToQString(const QUnicodeTables::Script script) {
    return QString(s_scriptStrings[script]);
};

static QString qFontDefToQString(const QFontDef &fontDef) {
    return QString("{family = '%1', styleName = '%2', pointSize = %3, pixelSize = %4, styleStrategy = %5, styleHint = %6, weight = %7, fixedPitch = %8, style = %9, stretch = %10, ignorePitch = %11, hintingPreference = %12, ...}").arg(fontDef.family).arg(fontDef.styleName).arg(fontDef.pointSize).arg(fontDef.pixelSize).arg(qStyleStrategyToQString(QFont::StyleStrategy(fontDef.styleStrategy))).arg(qStyleHintToQString(QFont::StyleHint(fontDef.styleHint))).arg(qWeightToQString(QFont::Weight(fontDef.weight))).arg(fontDef.fixedPitch ? "true" : "false").arg(qStyleToQString(QFont::Style(fontDef.style))).arg(qStretchToQString(QFont::Stretch(fontDef.stretch))).arg(fontDef.ignorePitch ? "true" : "false").arg(qHintingPreferenceToQString(QFont::HintingPreference(fontDef.hintingPreference)));
}

// convert 0 ~ 1000 integer to QFont::Weight
static QFont::Weight weightFromInteger(int weight)
{
    if (weight < 400)
        return QFont::Light;
    else if (weight < 600)
        return QFont::Normal;
    else if (weight < 700)
        return QFont::DemiBold;
    else if (weight < 800)
        return QFont::Bold;
    else
        return QFont::Black;
}

// Defines for usWidthClass according to Microsoft
// See http://www.microsoft.com/typography/otspec/os2.htm#wdc
#ifndef FWIDTH_ULTRA_CONDENSED
#define FWIDTH_ULTRA_CONDENSED 1
#define FWIDTH_EXTRA_CONDENSED 2
#define FWIDTH_CONDENSED 3
#define FWIDTH_SEMI_CONDENSED 4
#define FWIDTH_NORMAL 5
#define FWIDTH_SEMI_EXPANDED 6
#define FWIDTH_EXPANDED 7
#define FWIDTH_EXTRA_EXPANDED 8
#define FWIDTH_ULTRA_EXPANDED 9
#endif

static QFont::Stretch determineStretchFromTrueTypeWidthClass(unsigned short usWidthClass)
{
    switch (usWidthClass) {
    case FWIDTH_ULTRA_CONDENSED:
        return QFont::UltraCondensed;
    case FWIDTH_EXTRA_CONDENSED:
        return QFont::ExtraCondensed;
    case FWIDTH_CONDENSED:
        return QFont::Condensed;
    case FWIDTH_SEMI_CONDENSED:
        return QFont::SemiCondensed;
    case FWIDTH_NORMAL:
        return QFont::Unstretched;
    case FWIDTH_SEMI_EXPANDED:
        return QFont::SemiExpanded;
    case FWIDTH_EXPANDED:
        return QFont::Expanded;
    case FWIDTH_EXTRA_EXPANDED:
        return QFont::ExtraExpanded;
    case FWIDTH_ULTRA_EXPANDED:
        return QFont::UltraExpanded;
    default:
        return QFont::Unstretched;
    }
}

// Defines for fsSelection according to Microsoft
// See http://www.microsoft.com/typography/otspec/os2.htm#fss
#ifndef SELECTION_ITALIC_BIT
#define SELECTION_ITALIC_BIT 1 << 0
#define SELECTION_UNDERSCORE_BIT 1 << 1
#define SELECTION_NEGATIVE_BIT 1 << 2
#define SELECTION_OUTLINED_BIT 1 << 3
#define SELECTION_STRIKEOUT_BIT 1 << 4
#define SELECTION_BOLD_BIT 1 << 5
#define SELECTION_REGULAR_BIT 1 << 6
#define SELECTION_USE_TYPO_METRICS_BIT 1 << 7
#define SELECTION_WWS_BIT 1 << 8
#define SELECTION_OBLIQUE_BIT 1 << 9
#endif

static QFont::Style determineStyleFromTrueTypeSelection(unsigned short fsSelection)
{
    if (fsSelection & SELECTION_OBLIQUE_BIT)
        return QFont::StyleOblique;
    else if (fsSelection & SELECTION_ITALIC_BIT)
        return QFont::StyleItalic;
    else
        return QFont::StyleNormal;
}

QWebOSFontDatabase::QWebOSFontDatabase() : m_initialized(false), m_debug(false), m_qApp(NULL)
{
    qDebug() << __PRETTY_FUNCTION__;
    // These are the defaults that will be used
    setFontConfig(FontConfigSystem, "webos-system-fonts.xml");
    setFontConfig(FontConfigFallback, "webos-fallback-fonts.xml");
}

void QWebOSFontDatabase::setFontConfig(FontConfig config, const QString& location)
{
    qDebug() << __PRETTY_FUNCTION__ << config << location;
    m_fontConfig.insert(config, location);
}

QString QWebOSFontDatabase::resolveFontPathFor(FontConfig config)
{
    QString value = m_fontConfig[config];
    QString path = resolvePath(value);
    if(!QFile::exists(path)) {
        qDebug() << __PRETTY_FUNCTION__ << "Cannot find font directory" << path << "is Qt installed correctly?";
    }
    return path; 
}

QString QWebOSFontDatabase::resolvePath(const QString& fontFile)
{
    QString path;
    if (fontFile.indexOf('/')== 0) {
        // This is an absolute path, return as it is
        path = fontFile;
    } else {
        // We will treat it in relation to what is defined by 'fontDir()'
        // see gui/text/qplatformfontdatabase_qpa.cpp for that
        path = fontDir() + "/" + fontFile;
    }
    return path;
}

void QWebOSFontDatabase::populateFontDatabase()
{
    if (m_debug)
        qDebug("populateFontDatabase : m_initialized = %s", m_initialized ? "true" : "false");

    if (!m_initialized) {
        removeAppFontFiles();
        m_initialized = true;
    }

    QString systemFontsFileName = resolveFontPathFor(FontConfigSystem);
    qDebug() << __PRETTY_FUNCTION__ << systemFontsFileName;
    QDomDocument systemDoc("systemFonts");
    QFile systemFontsFile(systemFontsFileName);

    if (systemFontsFile.exists()) {
        if (!systemFontsFile.open(QIODevice::ReadOnly)) {
            qDebug("Can't find / open %s", qPrintable(systemFontsFileName));
            return;
        }
        if (!systemDoc.setContent(&systemFontsFile)) {
            systemFontsFile.close();
            qDebug("Could not get content from %s", qPrintable(systemFontsFileName));
            return;
        }
        systemFontsFile.close();

        QDomNodeList familySetNodes = systemDoc.elementsByTagName("familyset");
        for (int s = 0; s < familySetNodes.size(); ++s) {
            QDomElement familySet = familySetNodes.at(s).toElement();
            QDomNodeList familyNodes = familySet.elementsByTagName("family");
            for (int f = 0; f < familyNodes.size(); ++f) {
                QDomElement family = familyNodes.at(f).toElement();
                QStringList additionalFamilies;
                QDomNodeList nameNodes = family.elementsByTagName("name");
                QDomNodeList fileNodes = family.elementsByTagName("file");
                for (int n = 0; n < nameNodes.size(); ++n) {
                    QDomElement nameElem = nameNodes.at(n).toElement();
                    additionalFamilies << nameElem.text();
                }
                for (int i = 0; i < fileNodes.size(); ++i) {
                    QDomElement fileElem = fileNodes.at(i).toElement();
                    QString fileName = resolvePath(fileElem.text());
                    if (QFile::exists(fileName)) {
                        (void) addFontFile(QByteArray(), fileName, additionalFamilies);
                    } else {
                        qWarning() << "The font file does not exist" << fileName;
                    }
                }
            }
        }
    } else {
        qWarning() << "Could NOT find" << systemFontsFileName;
    }

    QString fallbackFontsFileName = resolveFontPathFor(FontConfigFallback);
    QFile fallbackFontsFile(fallbackFontsFileName);
    if (fallbackFontsFile.exists()) {
        qDebug() << __PRETTY_FUNCTION__ << fallbackFontsFileName;
        QDomDocument fallbackDoc("fallbackFonts");

        if (!fallbackFontsFile.open(QIODevice::ReadOnly)) {
            qDebug("Can't find / open %s", qPrintable(fallbackFontsFileName));
            return;
        }
        if (!fallbackDoc.setContent(&fallbackFontsFile)) {
            fallbackFontsFile.close();
            qDebug("Could not get content from %s", qPrintable(fallbackFontsFileName));
            return;
        }
        fallbackFontsFile.close();

        QDomNodeList fileNodes = fallbackDoc.elementsByTagName("file");
        for (int i = 0; i < fileNodes.size(); ++i) {
            QDomElement fileElem = fileNodes.at(i).toElement();
            QString fileName = resolvePath(fileElem.text());
            if (QFile::exists(fileName)) {
                QStringList families = addFontFile(QByteArray(), fileName, QStringList());
                m_fallbackFonts << families;
            } else {
                qWarning() << "The font file does not exist" << fileName;
            }
        }
        m_fallbackFonts.removeDuplicates();
    } else {
        qWarning() << "Could NOT find" << fallbackFontsFileName;
    }
}

void QWebOSFontDatabase::populateFontDatabaseFromAppFonts()
{
    QString fontpath = appFontDir();

    if(!QFile::exists(fontpath)) {
        qDebug("QWebOSFontDatabase: Cannot find app font directory %s", qPrintable(fontpath));
        return;
    }

    QDir dir(fontpath);
    dir.setNameFilters(QStringList() << QLatin1String("*.fnt"));
    dir.refresh();
    for (int i = 0; i < int(dir.count()); ++i) {
        const QString file = dir.absoluteFilePath(dir[i]);
        if (m_debug)
            qDebug() << "looking at" << file;
        addFontFile(QByteArray(), file, QStringList());
    }
}

void QWebOSFontDatabase::removeAppFontFiles()
{
    QString fontpath = appFontDir();

    if(!QFile::exists(fontpath)) {
        if (m_debug)
            qDebug("QWebOSFontDatabase: Cannot find app font directory %s", qPrintable(fontpath));
        return;
    }

    QDir dir(fontpath);
    dir.setNameFilters(QStringList() << QLatin1String("*.fnt"));
    dir.refresh();
    for (int i = 0; i < int(dir.count()); ++i) {
        (void) dir.remove(dir[i]);
    }
}

QFontEngine *QWebOSFontDatabase::fontEngine(const QFontDef &fontDef, QUnicodeTables::Script script, void *usrPtr)
{
    FontFile *fontfile = static_cast<FontFile *> (usrPtr);
    if (m_debug)
        qDebug("fontEngine(fontDef = %s, script = %s, fontFile = {fileName = %s, indexValue = %d, familyName = '%s'})",
               qPrintable(qFontDefToQString(fontDef)),
               qPrintable(qScriptToQString(script)),
               qPrintable(fontfile->fileName),
               fontfile->indexValue,
               qPrintable(fontfile->familyName));
    QFontEngineFT *engine;
    QFontEngine::FaceId fid;
    fid.filename = fontfile->fileName.toLocal8Bit();
    fid.index = fontfile->indexValue;
    engine = new QFontEngineFT(fontDef);

    bool antialias = !(fontDef.styleStrategy & QFont::NoAntialias);
    QFontEngineFT::GlyphFormat format = antialias? QFontEngineFT::Format_A8 : QFontEngineFT::Format_Mono;
    if (!engine->init(fid,antialias,format)) {
        delete engine;
        engine = 0;
        return engine;
    }
    if (engine->invalid()) {
        delete engine;
        engine = 0;
    } else if (scriptRequiresOpenType(script)) {
        HB_Face hbFace = engine->harfbuzzFace();
        if (!hbFace || !hbFace->supported_scripts[script]) {
            delete engine;
            engine = 0;
        }
    }

    return engine;
}

QStringList QWebOSFontDatabase::fallbacksForFamily(const QString family, const QFont::Style &style, const QFont::StyleHint &styleHint, const QUnicodeTables::Script &script) const
{
    if (m_debug)
        qDebug("fallbacksForFamily(family = '%s', style = %s, styleHint = %s, script = %s)",
               qPrintable(family),
               qPrintable(qStyleToQString(style)),
               qPrintable(qStyleHintToQString(styleHint)),
               qPrintable(qScriptToQString(script)));

    QString familyName = qStyleHintToQString(styleHint).toLower();

    if (familyName == "anystyle")
        familyName = family;

    if (script == QUnicodeTables::Common) {
        if (familyName == "system")
            familyName = "helvetica";
        if (familyName == "oldenglish")
            familyName = "serif";
    }

    QStringList fallbacks = QStringList(familyName) + m_fallbackFonts;

    if (m_debug)
        qDebug("returning '%s'", qPrintable(fallbacks.join(",")));
    return fallbacks;
}

QStringList QWebOSFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
    if (m_debug)
        qDebug("addApplicationFont(fontData.size() = %d, fileName = \"%s\")", fontData.size(), qPrintable(fileName.toLocal8Bit()));

    if (!m_qApp) {
        m_qApp = (QApplication *) QApplication::instance();
        QWebOSFontDatabase* self = (QWebOSFontDatabase *) this;
        connect(m_qApp, SIGNAL(fontDatabaseChanged()), self, SLOT(doFontDatabaseChanged()));
    }

    return addFontFile(fontData, fileName, QStringList());
}

void QWebOSFontDatabase::doFontDatabaseChanged()
{
    if (m_debug)
        qDebug("doFontDatabaseChanged");
    populateFontDatabase();
    populateFontDatabaseFromAppFonts();
}

void QWebOSFontDatabase::releaseHandle(void *handle)
{
    FontFile *file = static_cast<FontFile *>(handle);
    if (m_debug)
        qDebug("releaseHandle(%s)", file ? qPrintable(file->fileName) : "null");

    m_fontFileList.removeAll(file->fileName);

    delete file;
}

QString QWebOSFontDatabase::appFontDir()
{
    return QDir::tempPath() + "/webos-app-fonts";
}

bool QWebOSFontDatabase::createFileWithFontData(QString& fileName, const QByteArray &fontData)
{
    QDir fontDir(appFontDir());
    if (!fontDir.mkpath(fontDir.path())) {
        qDebug("Couldn't make directory (%s) to hold application fonts", qPrintable(fontDir.path()));
        return false;
    }
    QByteArray fontDataHash = QCryptographicHash::hash(fontData, QCryptographicHash::Sha1);
    fileName = appFontDir() + "/" + QString(fontDataHash.toHex()) + ".fnt";

    QFile fontFile(fileName);
    if (!fontFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug("Couldn't open font file (%s) to store the font data", qPrintable(fileName));
        return false;
    }

    qint64 bytesWritten = fontFile.write(fontData.constData(), fontData.size());
    fontFile.close();
    if (bytesWritten < 0) {
        qDebug("Error writing font data to %s", qPrintable(fileName));
        return false;
    } else if (bytesWritten < fontData.size()) {
        qDebug("Wrote %d bytes to %s.  Expected %d.", bytesWritten, qPrintable(fileName), fontData.size());
        return false;
    }

    return true;
}

QStringList QWebOSFontDatabase::addFontFile(const QByteArray &fontData, const QString &fileName, const QStringList &additionalFamilies)
{
    QStringList families;
    QString fontFileName = fileName;
    QByteArray data = fontData;

    if (!fontData.isEmpty() && fileName.startsWith(":qmemoryfonts/")) {
        if (!createFileWithFontData(fontFileName, fontData))
            return families;
        data = QByteArray();
    }

    if (!m_fontFileList.contains(fontFileName)) {
        families = addTTFile(this, data, fontFileName.toLocal8Bit(), additionalFamilies);
        m_fontFileList << fontFileName;
    }
    return families;
}

QStringList QWebOSFontDatabase::addTTFile(QWebOSFontDatabase* qwfdb, const QByteArray &fontData, const QByteArray &file, const QStringList &additionalFamilies)
{
    if (qwfdb && qwfdb->m_debug)
        qDebug("addTTFile(fontData.size() = %d, file = '%s', additionalFamilies = '%s')",
               fontData.size(),
               qPrintable(file),
               qPrintable(additionalFamilies.join(",")));
    extern FT_Library qt_getFreetype();
    FT_Library library = qt_getFreetype();

    int numFaces = 0;
    int index = 0;
    QStringList families;

    FT_Face face;
    FT_Error error;
    if (!fontData.isEmpty()) {
        error = FT_New_Memory_Face(library, (const FT_Byte *)fontData.constData(), fontData.size(), index, &face);
    } else {
        error = FT_New_Face(library, file.constData(), index, &face);
    }
    if (error != FT_Err_Ok) {
        qDebug() << "FT_New_Face for " << qPrintable(file) << " failed with index" << index << ":" << hex << error;
        return families;
    }

    numFaces = face->num_faces;
    if (numFaces > 1) {
        qWarning() << "numFaces for " << qPrintable(file) << " is " << numFaces << ", expected just 1";
    }

    QFont::Weight weight = QFont::Normal;
    QFont::Stretch stretch = QFont::Unstretched;
    QFont::Style style = QFont::StyleNormal;

    if (face->style_flags & FT_STYLE_FLAG_ITALIC)
        style = QFont::StyleItalic;

    if (face->style_flags & FT_STYLE_FLAG_BOLD)
        weight = QFont::Bold;

    QSupportedWritingSystems writingSystems;
    // detect symbol fonts
    for (int i = 0; i < face->num_charmaps; ++i) {
        FT_CharMap cm = face->charmaps[i];
        if (cm->encoding == ft_encoding_adobe_custom
            || cm->encoding == ft_encoding_symbol) {
            writingSystems.setSupported(QFontDatabase::Symbol);
            break;
        }
    }

    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2) {
        quint32 unicodeRange[4] = {
            os2->ulUnicodeRange1, os2->ulUnicodeRange2, os2->ulUnicodeRange3, os2->ulUnicodeRange4
        };
        quint32 codePageRange[2] = {
            os2->ulCodePageRange1, os2->ulCodePageRange2
        };

        writingSystems = determineWritingSystemsFromTrueTypeBits(unicodeRange, codePageRange);
        // If the OS2 struct is availabel read the weight from there and convert it
        // to a QFont::Weight. This fixes OWEBOS-1866
        weight = weightFromInteger(os2->usWeightClass);
        stretch = determineStretchFromTrueTypeWidthClass(os2->usWidthClass);
        style = determineStyleFromTrueTypeSelection(os2->fsSelection);
    }

    QString family = QString::fromAscii(face->family_name);

    QStringList allFamilies(family);
    allFamilies << additionalFamilies;

    for (int i = 0; i < allFamilies.size(); ++i) {
        FontFile *fontFile = new FontFile;
        fontFile->fileName = file;
        fontFile->indexValue = index;
        fontFile->familyName = allFamilies.at(i);
        qDebug("registerFont(\"%s\",\"\",%s,%s,%s,true,true,0,\"%s\",fontFile = {fileName = \"%s\", indexValue = %d, familyName = %s})",
               qPrintable(allFamilies.at(i)),
               qPrintable(qWeightToQString(weight)),
               qPrintable(qStyleToQString(style)),
               qPrintable(qStretchToQString(stretch)),
               qPrintable(qSupportedWritingSystemsToQString(writingSystems)),
               qPrintable(fontFile->fileName),
               fontFile->indexValue,
               qPrintable(fontFile->familyName));

        registerFont(allFamilies.at(i), "", weight, style, stretch, true, true, 0, writingSystems, fontFile);
        families.append(family);
    }

    FT_Done_Face(face);

    return families;
}
