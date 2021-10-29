/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "jobstatus.h"

#include <kcoreaddons_version.h>
#include <KJob>
#include <KLocalizedString>

using namespace KDevelop;

class KDevelop::JobStatusPrivate
{
public:
    QString m_statusName;
};

JobStatus::JobStatus(KJob* job, const QString& statusName, QObject* parent)
    : QObject(parent)
    , d_ptr(new JobStatusPrivate{statusName})
{
    connect(job, &KJob::infoMessage, this, [this](KJob*, const QString& plain, const QString&) {
        emit showMessage(this, plain);
    });
    connect(job, &KJob::finished, this, [this, job]() {
        if (job->error() == KJob::KilledJobError) {
            emit showErrorMessage(i18n("Task aborted"));
        }
        emit hideProgress(this);
        deleteLater();
    });
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 80, 0)
    connect(job, QOverload<KJob*, unsigned long>::of(&KJob::percent), this, &JobStatus::slotPercent);
#else
    connect(job, &KJob::percentChanged, this, &JobStatus::slotPercent);
#endif
}

JobStatus::~JobStatus()
{
}

QString JobStatus::statusName() const
{
    Q_D(const JobStatus);

    return d->m_statusName;
}

void JobStatus::slotPercent(KJob* job, unsigned long percent)
{
    Q_UNUSED(job)
    emit showProgress(this, 0, 100, percent);
}

#include "moc_jobstatus.cpp"
