/*
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef QTPRINTERSTEST_H
#define QTPRINTERSTEST_H

#include <QObject>

class QtPrintersTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testQString();
    void testQByteArray();
    void testQListContainer_data();
    void testQListContainer();
    void testQMapInt();
    void testQMapString();
    void testQMapStringBool();
    void testQDate();
    void testQTime();
    void testQDateTime();
    void testQUrl();
    void testQHashInt();
    void testQHashString();
    void testQMultiHashInt();
    void testQMultiHashString();
    void testQSetInt();
    void testQSetString();
    void testQChar();
    void testQListPOD();
    void testQUuid();
    void testQVariant();
    void testKTextEditorTypes();
    void testKDevelopTypes();
};

#endif
