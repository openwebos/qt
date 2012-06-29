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

#ifndef QWEBOSFONTDATABASE_H
#define QWEBOSFONTDATABASE_H

#include <QPlatformFontDatabase>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QObject>

struct FontFile
{
    QString fileName;
    int indexValue;
    QString familyName;
};


class QWebOSFontDatabase : public QObject, public QPlatformFontDatabase
{
Q_OBJECT
public:

    enum FontConfig {
        FontConfigSystem,
        FontConfigFallback
    };

    QWebOSFontDatabase();
    void populateFontDatabase();
    QFontEngine *fontEngine(const QFontDef &fontDef, QUnicodeTables::Script script, void *handle);
    QStringList fallbacksForFamily(const QString family, const QFont::Style &style, const QFont::StyleHint &styleHint, const QUnicodeTables::Script &script) const;
    QStringList addApplicationFont(const QByteArray &fontData, const QString &fileName);
    void releaseHandle(void *handle);

    /*!
        Sets the path for the given font configuration.

        If the path starts with an '/' it will be treated as an abolute path, otherwise
        it will be treated as relative to either the 'QT_QPA_FONTDIR' environment variable
        or under the WEBOS and PALM_DEVICE config relative to /usr/share/fonts.
    */
    void setFontConfig(FontConfig config, const QString& location);
    static QStringList addTTFile(QWebOSFontDatabase* qwfdb, const QByteArray &fontData, const QByteArray &file, const QStringList &additionalFamilies);

public Q_SLOTS:
    void doFontDatabaseChanged();

private:
    QString appFontDir();
    void populateFontDatabaseFromAppFonts();
    void removeAppFontFiles();

    /*
       Resolves the path for the given for configuration file.

       \sa setFontConfig
    */
    QString resolveFontPathFor(FontConfig config);

    /*! Internal utility function that actuall ydoes the resolving. */
    QString resolvePath(const QString& fontFile);

    QStringList addFontFile(const QByteArray &fontData, const QString &fileName, const QStringList &additionalFamilies);
    bool createFileWithFontData(QString& fileName, const QByteArray &fontData);
    bool m_initialized;
    bool m_debug;
    QStringList m_fallbackFonts;
    QApplication* m_qApp;
    QStringList m_fontFileList;

    QMap<FontConfig, QString> m_fontConfig;
};

#endif // QWEBOSFONTDATABASE_H
