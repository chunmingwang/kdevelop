/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonkwargsinfo.h"
#include "debug.h"

MesonKWARGSInfo::MesonKWARGSInfo(MesonKWARGSInfo::Function fn, QString id)
    : m_func(fn)
    , m_id(id)
{
}

QJsonObject MesonKWARGSInfo::command()
{
    QJsonObject res;

    auto func2str = [](Function fn) -> QString {
        switch (fn) {
        case PROJECT:
            return QStringLiteral("project");
        case TARGET:
            return QStringLiteral("target");
        case DEPENDENCY:
            return QStringLiteral("dependency");
        }

        return QStringLiteral("ERROR");
    };

    res[QStringLiteral("type")] = QStringLiteral("kwargs");
    res[QStringLiteral("function")] = func2str(m_func);
    res[QStringLiteral("id")] = m_id;
    res[QStringLiteral("operation")] = QStringLiteral("info");

    m_infoID = func2str(m_func) + QStringLiteral("#") + m_id;

    return res;
}

void MesonKWARGSInfo::parseResult(QJsonObject data)
{
    if (!data[QStringLiteral("kwargs")].isObject()) {
        qCWarning(KDEV_Meson) << "REWRITR: Failed to parse rewriter result";
        return;
    }

    QJsonObject kwargs = data[QStringLiteral("kwargs")].toObject();

    if (!kwargs[m_infoID].isObject()) {
        qCWarning(KDEV_Meson) << "REWRITR: Failed to extract info data from object";
        return;
    }

    m_result = kwargs[m_infoID].toObject();
}

MesonKWARGSInfo::Function MesonKWARGSInfo::function() const
{
    return m_func;
}

QString MesonKWARGSInfo::id() const
{
    return m_id;
}

QJsonObject MesonKWARGSInfo::result() const
{
    return m_result;
}

QString MesonKWARGSInfo::getString(QString kwarg) const
{
    if (!m_result.contains(kwarg)) {
        return QString();
    }
    return m_result[kwarg].toString();
}

// Getters for Project

QString MesonKWARGSProjectInfo::mesonVersion() const
{
    return getString(QStringLiteral("meson_version"));
}

QString MesonKWARGSProjectInfo::license() const
{
    return getString(QStringLiteral("license"));
}

QString MesonKWARGSProjectInfo::subprojectDir() const
{
    return getString(QStringLiteral("subproject_dir"));
}

QString MesonKWARGSProjectInfo::version() const
{
    return getString(QStringLiteral("version"));
}

// Constructors

MesonKWARGSProjectInfo::MesonKWARGSProjectInfo()
    : MesonKWARGSInfo(PROJECT, QStringLiteral("/"))
{
}
MesonKWARGSTargetInfo::MesonKWARGSTargetInfo(QString id)
    : MesonKWARGSInfo(TARGET, id)
{
}
MesonKWARGSDependencyInfo::MesonKWARGSDependencyInfo(QString id)
    : MesonKWARGSInfo(DEPENDENCY, id)
{
}

// Destructors

MesonKWARGSInfo::~MesonKWARGSInfo() {}
MesonKWARGSProjectInfo::~MesonKWARGSProjectInfo() {}
MesonKWARGSTargetInfo::~MesonKWARGSTargetInfo() {}
MesonKWARGSDependencyInfo::~MesonKWARGSDependencyInfo() {}