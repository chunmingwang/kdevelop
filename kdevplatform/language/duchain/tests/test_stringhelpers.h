/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_STRINGHELPERS_H
#define KDEVPLATFORM_TEST_STRINGHELPERS_H

#include <QObject>

class TestDUChain
    : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testFormatComment_data();
    void testFormatComment();

    void benchFormatComment();
};

#endif // KDEVPLATFORM_TEST_STRINGHELPERS_H
