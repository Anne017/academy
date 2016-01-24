#ifndef SEARCHENGINE_H_
#define SEARCHENGINE_H_

#include <QRegExp>

#include "course.h"

class SearchEngine
{
public:
    explicit SearchEngine(const QString& query)
    {
        QStringList parts = query.split(' ');
        for (int i = 0; i < parts.size(); ++i)
            parts[i] = "(" + parts[i].toLower() + ")";
        QString regex = parts.join('|');
        m_regex = QRegExp(regex, Qt::CaseInsensitive);
    }

    bool isMatch(const Course& course)
    {
        return m_regex.indexIn(course.title) != -1 ||
                m_regex.indexIn(course.subTitle) != -1 ||
                m_regex.indexIn(course.description) != -1;
    }

private:
    QRegExp m_regex;
};

#endif // SEARCHENGINE_H_
