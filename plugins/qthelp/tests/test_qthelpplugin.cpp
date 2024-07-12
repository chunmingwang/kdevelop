/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_qthelpplugin.h"
#include "../qthelpplugin.h"
#include "../qthelpprovider.h"
#include "../qthelp_config_shared.h"
#include "../qthelpqtdoc.h"

#include <QCoreApplication>
#include <QHelpLink>
#include <QRegularExpression>
#include <QTest>

#include <interfaces/idocumentation.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/pointertype.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testhelpers.h>

#include "testqthelpconfig.h"

#include <functional>
#include <memory>

namespace {
const QString VALID1 = QTHELP_FILES "/valid1.qch";
const QString VALID2 = QTHELP_FILES "/valid2.qch";
const QString INVALID = QTHELP_FILES "/invalid.qch";

template<typename WriteConfigCallback>
std::unique_ptr<QtHelpPlugin> makePlugin(KDevelop::TestCore* testCore, const WriteConfigCallback& writeConfigCallback)
{
    auto plugin = std::make_unique<QtHelpPlugin>(testCore, QVariantList());

    // write default config and read it
    writeConfigCallback();
    plugin->readConfig();

    // IILE to allow QTRY_VERIFY to return void on failure
    [&]() {
        // ensure the qmake process is finished before we continue the test
        QTRY_VERIFY(plugin->isInitialized());
    }();

    return plugin;
}

std::unique_ptr<QtHelpPlugin> makePlugin(KDevelop::TestCore* testCore)
{
    return makePlugin(testCore, []() {
        qtHelpWriteConfig(QStringList(), QStringList(), QStringList(), QStringList(), QString(), true);
    });
}

using TestDeclarationLookupProvider = std::shared_ptr<const QtHelpQtDoc>;
using TestDeclarationLookupCallback =
    std::function<void(const KDevelop::TopDUContext* ctx, const QtHelpProviderAbstract* provider)>;
}

Q_DECLARE_METATYPE(TestDeclarationLookupProvider)
Q_DECLARE_METATYPE(TestDeclarationLookupCallback)

QTEST_MAIN(TestQtHelpPlugin)
using namespace KDevelop;

void TestQtHelpPlugin::initTestCase()
{
    AutoTestShell::init({"no plugins"});

    // Prevent SegFault, then "ICE default IO error handler doing an exit(), pid = <PID>, errno = 32"
    // crash when the test runs for at least 60 seconds. This is a workaround for QTBUG-58709.
    QCoreApplication::processEvents();

    m_testCore = new TestCore();
    m_testCore->initialize();
}

void TestQtHelpPlugin::init()
{
    // cleanup cached files from previous test runs
    const auto appDataDir =
        QDir(QtHelpProviderAbstract::collectionFileLocation(), "*.qhc", QDir::NoSort, QDir::Files | QDir::Hidden);
    const auto entries = appDataDir.entryInfoList();
    for (const auto& file : entries)
        QVERIFY(QFile::remove(file.absoluteFilePath()));
}

void TestQtHelpPlugin::cleanupTestCase()
{
    m_testCore->cleanup();
    delete m_testCore;
}

void TestQtHelpPlugin::testDefaultValue()
{
    auto plugin = makePlugin(m_testCore);
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->isQtHelpQtDocLoaded(), true);
    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 0);
    QCOMPARE(plugin->providers().size(), 1);

    const auto provider = dynamic_cast<QtHelpProviderAbstract*>(plugin->providers().at(0));
    if (!provider->isValid()) {
        QSKIP("Qt help not available");
    }

    QTRY_VERIFY(provider->indexModel()->rowCount() > 0);
}

void TestQtHelpPlugin::testUnsetQtHelpDoc()
{
    auto plugin = makePlugin(m_testCore, []() {
        qtHelpWriteConfig(QStringList(), QStringList(), QStringList(), QStringList(), QString(), false);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->providers().size(), 0);
}

void TestQtHelpPlugin::testAddOneValidProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1;
    name << QStringLiteral("file1");
    icon << QStringLiteral("myIcon");
    ghns << QStringLiteral("0");

    auto plugin = makePlugin(m_testCore, [&]() {
        qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 1);
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->fileName(), path.at(0));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->name(), name.at(0));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->iconName(), icon.at(0));
}

void TestQtHelpPlugin::testAddTwoDifferentValidProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID2;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");

    auto plugin = makePlugin(m_testCore, [&]() {
        qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 2);
    // first provider
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->fileName(), path.at(0));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->name(), name.at(0));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0)->iconName(), icon.at(0));
    // second provider
    QCOMPARE(plugin->qtHelpProviderLoaded().at(1)->fileName(), path.at(1));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(1)->name(), name.at(1));
    QCOMPARE(plugin->qtHelpProviderLoaded().at(1)->iconName(), icon.at(1));
}

void TestQtHelpPlugin::testAddInvalidProvider()
{
    QStringList path, name, icon, ghns;
    path << INVALID;
    name << QStringLiteral("file1");
    icon << QStringLiteral("myIcon");
    ghns << QStringLiteral("0");

    auto plugin = makePlugin(m_testCore, [&]() {
        qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 0);
}

void TestQtHelpPlugin::testAddTwiceSameProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID1;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");

    auto plugin = makePlugin(m_testCore, [&]() {
        qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 1);
}

void TestQtHelpPlugin::testRemoveOneProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID2;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");

    auto plugin = makePlugin(m_testCore, [&]() {
        qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    });
    RETURN_IF_TEST_FAILED();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 2);
    // we remove the second provider
    QtHelpProvider* provider = plugin->qtHelpProviderLoaded().at(0);
    path.removeAt(1);
    name.removeAt(1);
    icon.removeAt(1);
    ghns.removeAt(1);
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    plugin->readConfig();

    QCOMPARE(plugin->qtHelpProviderLoaded().size(), 1);
    QCOMPARE(plugin->qtHelpProviderLoaded().at(0), provider);
}

void TestQtHelpPlugin::testDeclarationLookup_data()
{
    QTest::addColumn<TestDeclarationLookupProvider>("provider");
    QTest::addColumn<QString>("fileContents");
    QTest::addColumn<TestDeclarationLookupCallback>("callback");

    bool anyAvailable = false;
    const auto qmakeCandidates = QtHelpQtDoc::qmakeCandidates();
    for (const auto& qmake : qmakeCandidates) {
        const auto mutableProvider =
            std::make_shared<QtHelpQtDoc>(nullptr, qmake, qmake + "_testDeclarationLookup.qhc");
        QTRY_VERIFY(mutableProvider->isInitialized());

        mutableProvider->loadDocumentation();

        if (!mutableProvider->isValid() || mutableProvider->engine()->documentsForIdentifier("QObject").isEmpty()) {
            qWarning() << "Qt help not available for" << qmake;
            continue;
        }

        const auto provider = TestDeclarationLookupProvider(mutableProvider);

        anyAvailable = true;

        QTest::addRow("QObject-%s", qPrintable(qmake))
            << provider << "class QObject; QObject* o;"
            << TestDeclarationLookupCallback{[](const TopDUContext* ctx, const QtHelpProviderAbstract* provider) {
                   auto decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("o"))).first();
                   QVERIFY(decl);
                   auto typeDecl = dynamic_cast<const IdentifiedType*>(decl->type<PointerType>()->baseType().data())
                                       ->declaration(nullptr);
                   QVERIFY(typeDecl);

                   auto doc = provider->documentationForDeclaration(typeDecl);
                   QVERIFY(doc);
                   QCOMPARE(doc->name(), QStringLiteral("QObject"));
                   const auto description = doc->description();
                   // we should have the brief section extracted
                   QVERIFY(description.contains("The QObject class"));
                   // the unhelpful More... link should have been removed
                   QVERIFY(!description.contains("More..."));
                   // we also don't want the unhelpful link description fallback
                   QVERIFY(!description.contains("|"));
               }};

        QTest::addRow("QString::fromLatin1-%s", qPrintable(qmake))
            << provider << "class QString { static QString fromLatin1(const QByteArray&); };"
            << TestDeclarationLookupCallback{[](const TopDUContext* ctx, const QtHelpProviderAbstract* provider) {
                   auto decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("QString"))).first();
                   QVERIFY(decl);
                   auto declFromLatin1 =
                       decl->internalContext()->findDeclarations(QualifiedIdentifier("fromLatin1")).first();

                   auto doc = provider->documentationForDeclaration(declFromLatin1);
                   QVERIFY(doc);
                   QCOMPARE(doc->name(), QStringLiteral("QString::fromLatin1"));
                   const auto description = doc->description();
                   QVERIFY(description.contains(QRegularExpression{"See also.*toLatin1"}));
                   QVERIFY(!description.contains("href"));
                   QVERIFY(!description.contains("<p"));
                   QVERIFY(!description.contains("<h3"));
               }};

        QTest::addRow("operator-%s", qPrintable(qmake))
            << provider << "class C {}; bool operator<(const C& a, const C& b) { return true; }"
            << TestDeclarationLookupCallback{[](const TopDUContext* ctx, const QtHelpProviderAbstract* provider) {
                   auto decl = ctx->findDeclarations(QualifiedIdentifier("operator<")).first();
                   QVERIFY(decl);

                   auto doc = provider->documentationForDeclaration(decl);
                   // TODO: We should never find a documentation entry for this (but instead, the operator< for QChar is found here)
                   QEXPECT_FAIL("", "doc should be null here", Continue);
                   QVERIFY(!doc);
               }};
    }

    if (!anyAvailable) {
        QSKIP("Qt help not available");
    }
}

void TestQtHelpPlugin::testDeclarationLookup()
{
    QFETCH(const TestDeclarationLookupProvider, provider);
    QFETCH(const QString, fileContents);
    QFETCH(const TestDeclarationLookupCallback, callback);

    QVERIFY(provider);
    QVERIFY(callback);

    TestFile file(fileContents, "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    QVERIFY(ctx);

    callback(ctx, provider.get());
}

#include "moc_test_qthelpplugin.cpp"
