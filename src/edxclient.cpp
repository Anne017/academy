/*
 * Copyright (C) 2016 Roman Shchekin aka mrqtros.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <edxclient.h>
#include <localization.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>

Q_LOGGING_CATEGORY(Edx, "Edx")

namespace http = core::net::http;
namespace net = core::net;

using namespace std;

EdxClient::EdxClient(Config::Ptr config) :
    CachedClient(config)
{
    //qCDebug(Edx) << "Cache dir:" << QString::fromStdString(config->cache_dir);
}

QList<Course> EdxClient::courses(const QString &query)
{
    QList<Course> list;

    static QByteArray data;
    net::Uri::Path path;
    net::Uri::QueryParameters params;

    qCDebug(Edx) << "Download started...";
    if (data.isNull())
        get( path, params, data);
    qCDebug(Edx) << "Data received:" << data.length() << "bytes";

    QDomDocument doc;
    if (!doc.setContent(data))
    {
        qCWarning(Edx) << "Parse error, data was incorrect:" << data;
        return list;
    }

    QDomNodeList itemList = doc.elementsByTagName("item");
    qCDebug(Edx) << "Element count:" << itemList.length();

    SearchEngine se(query);

    for (int i = 0; i < itemList.length(); i++)
    {
        QDomElement courseElem = itemList.at(i).toElement();

        if (courseElem.isNull())
            continue;

        Course course;

        QDomElement id = courseElem.firstChildElement("guid");
        if (!id.isNull())
            course.id = id.text();

        QDomElement title = courseElem.firstChildElement("title");
        if (!title.isNull())
            course.title = title.text();

        QDomElement description = courseElem.firstChildElement("description");
        if (!description.isNull())
            course.description = description.text();

        QDomElement subtitle = courseElem.firstChildElement("course:subtitle");
        if (!subtitle.isNull())
            course.headline = subtitle.text();

        QDomElement art = courseElem.firstChildElement("course:image-thumbnail");
        if (!art.isNull())
            course.art = art.text();

        QDomElement link = courseElem.firstChildElement("link");
        if (!link.isNull())
            course.link = link.text();

        QDomElement video = courseElem.firstChildElement("course:video-youtube");
        if (!video.isNull())
            course.video = video.text();

        course.extra = grabExtra(courseElem);

        // Instructors.
        QDomNodeList instrList = courseElem.elementsByTagName("course:staff");
        for (int j = 0; j < instrList.count(); ++j)
        {
            QDomElement instrElem = instrList.at(j).toElement();

            if (instrElem.isNull())
                continue;

            Instructor instr;

            QDomElement name = instrElem.firstChildElement("staff:name");
            if (!name.isNull())
                instr.name = name.text();

            QDomElement bio = instrElem.firstChildElement("staff:bio");
            if (!bio.isNull())
                instr.bio = bio.text();

            QDomElement image = instrElem.firstChildElement("staff:image");
            if (!image.isNull())
                instr.image = image.text();

            //qCDebug(Edx) << "Instr details:" << instr.name << instr.image;

            course.instructors.append(instr);
        }

        // Departments.
        QDomNodeList depList = courseElem.elementsByTagName("course:subject");
        for (int k = 0; k < depList.count(); ++k)
        {
            QDomElement depElem = depList.at(k).toElement();

            if (depElem.isNull())
                continue;

            course.departments.append(depElem.text());
        }

        //qCDebug(Edx) << "Subject count: " << course.departments;
        //qCDebug(Edx) << "Instr count: " << course.instructors.size();
        if (query.isEmpty() || se.isMatch(course))
            list.append(course);
    }

    return list;
}

const QString EdxClient::baseApiUrl() const
{
    return QStringLiteral("https://www.edx.org/api/v2/report/course-feed/rss");
}

const QString EdxClient::name() const
{
    return QStringLiteral("edX");
}

QString EdxClient::grabExtra(const QDomElement &courseElem)
{
    QStringList extra;

    QDomElement effort = courseElem.firstChildElement("course:effort");
    if (!effort.isNull())
        extra << _("workload - ") + effort.text();

    QDomElement length = courseElem.firstChildElement("course:length");
    if (!length.isNull())
        extra << _("duration - ") + length.text();

    return extra.join(", ");
}
