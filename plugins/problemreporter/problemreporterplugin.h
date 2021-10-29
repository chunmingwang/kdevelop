/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H
#define KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariant>

#include <serialization/indexedstring.h>
#include <language/duchain/topducontext.h>

namespace KTextEditor
{
class Document;
}
namespace KDevelop
{
class IDocument;
}

class ProblemHighlighter;
class ProblemInlineNoteProvider;
class ProblemReporterModel;

class ProblemReporterPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit ProblemReporterPlugin(QObject* parent, const QVariantList& = QVariantList());
    ~ProblemReporterPlugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    // KDevelop::Plugin methods
    void unload() override;

    ProblemReporterModel* model() const;

private Q_SLOTS:
    void updateReady(const KDevelop::IndexedString& url, const KDevelop::ReferencedTopDUContext&);
    void updateHighlight(const KDevelop::IndexedString& url);
    void textDocumentCreated(KDevelop::IDocument* document);
    void documentActivated(KDevelop::IDocument* document);
    void showModel(const QString& id);

private:
    void updateOpenedDocumentsHighlight();
    class ProblemReporterFactory* m_factory;
    ProblemReporterModel* m_model;

    QHash<KDevelop::IndexedString, ProblemHighlighter*> m_highlighters;
    QHash<KDevelop::IndexedString, ProblemInlineNoteProvider*> m_inlineNoteProviders;
    QSet<KDevelop::IndexedString> m_reHighlightNeeded;
public Q_SLOTS:
    void documentClosed(KDevelop::IDocument*);
};

#endif // KDEVPLATFORM_PLUGIN_PROBLEMREPORTERPLUGIN_H
